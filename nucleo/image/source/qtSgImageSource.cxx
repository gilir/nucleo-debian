/*
 *
 * nucleo/image/source/qtSgImageSource.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>
#include <nucleo/core/cocoa/cPoolTrick.H>

#include <nucleo/image/source/qtSgImageSource.H>
#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/network/udp/UdpSender.H>
#include <nucleo/network/NetworkUtils.H>

#include <sstream>

// Window server private stuff
extern "C" OSErr CPSEnableForegroundOperation(ProcessSerialNumber *psn) ;

namespace nucleo {

  static const char *nSignalGroup = "225.0.0.252" ;
  static const int nSignalPort = 5556 ;

  // --------------------------------------------------------------------------

  static inline OSStatus
  SetNumberValue(CFMutableDictionaryRef inDict, CFStringRef inKey, SInt32 inValue) {
    CFNumberRef number = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &inValue);
    if (NULL == number) return coreFoundationUnknownErr;
    CFDictionarySetValue(inDict, inKey, number);
    CFRelease(number);
    return noErr;
  }

  inline QTVisualContextRef
  CreatePixelBufferContext(SInt32 inPixelFormat, SInt32 width, SInt32 height) {
    CFMutableDictionaryRef pbOpts = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
												  &kCFTypeDictionaryKeyCallBacks,
												  &kCFTypeDictionaryValueCallBacks);
    if (NULL == pbOpts) return 0 ;
    SetNumberValue(pbOpts, kCVPixelBufferPixelFormatTypeKey, inPixelFormat);
    SetNumberValue(pbOpts, kCVPixelBufferWidthKey, width);
    SetNumberValue(pbOpts, kCVPixelBufferHeightKey, height);
    
    CFMutableDictionaryRef vcOpts = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
												  &kCFTypeDictionaryKeyCallBacks,
												  &kCFTypeDictionaryValueCallBacks);
    if (NULL == vcOpts) {
	 CFRelease(pbOpts) ;
	 return 0 ;
    }
    CFDictionarySetValue(vcOpts, kQTVisualContextPixelBufferAttributesKey, pbOpts);
    
    QTVisualContextRef theContext = NULL ;
    OSStatus err = QTPixelBufferContextCreate(kCFAllocatorDefault, vcOpts, &theContext);
    if (err!=noErr) {
	 std::cerr << "qtSgImageSource: QTPixelBufferContextCreate failed (" << err << ")" << std::endl ;
	 CFRelease(vcOpts) ;
	 CFRelease(pbOpts) ;
	 if (theContext) QTVisualContextRelease(theContext) ;
	 return 0 ;
    }
    
    return theContext ;
  }
  
  void
  qtSgImageSource::cb_3_ready(QTVisualContextRef visualContext, 
						const CVTimeStamp *timeStamp, void *refCon) {
    qtSgImageSource *src = (qtSgImageSource*)refCon ;

    if (src->cvimage) {
	 CVPixelBufferUnlockBaseAddress(src->cvimage, 0) ;
	 CVPixelBufferRelease(src->cvimage) ;
    }

    OSErr err = QTVisualContextCopyImageForTime(visualContext, kCFAllocatorDefault,
									   timeStamp, &src->cvimage) ;
    if (err!=noErr) return ;

    CVPixelBufferLockBaseAddress((CVPixelBufferRef)src->cvimage, 0) ;

    src->lastImage.setData((unsigned char*)CVPixelBufferGetBaseAddress(src->cvimage),
					  CVPixelBufferGetDataSize(src->cvimage),
					  Image::NONE) ;
    src->lastImage.setTimeStamp() ;
    src->lastImage.setDims(src->iWidth, src->iHeight) ;
    src->lastImage.setEncoding(src->qtEncoding) ;

    src->frameCount++ ; src->sampler.tick() ;
    if (!src->_pendingNotifications) src->notifyObservers() ;

    QTVisualContextTask(visualContext) ;
  }

  pascal OSErr 
  qtSgImageSource::cb_1_grab(SGChannel c,
					    Ptr data, long length, long *offset,
					    long chRefCon, 
					    TimeValue timeValue, short writeType, long refCon) {
    qtSgImageSource *src = (qtSgImageSource*)refCon ;

    ICMFrameTimeRecord now = {0};	
    now.recordSize = sizeof(ICMFrameTimeRecord);
    *(TimeValue64*)&now.value = timeValue;
    now.scale = src->timeScale;
    now.rate = fixed1;
    now.decodeTime = timeValue;
    now.frameNumber = ++(src->frameNumber) ;
    now.flags = icmFrameTimeIsNonScheduledDisplayTime;
  
    OSStatus err = ICMDecompressionSessionDecodeFrame(src->decompSession, 
										    (const UInt8*)data, length, NULL, 
										    &now, 0);
    if (err!=noErr) {
	 std::cerr << "qtSgImageSource: ICMDecompressionSessionDecodeFrame failed" << std::endl ;
	 return err ;
    }
	
    err = ICMDecompressionSessionSetNonScheduledDisplayTime(src->decompSession, 
												*(long long int*)&now.value,
												src->timeScale, 0);
    if (err!=noErr) {
	 std::cerr << "qtSgImageSource: ICMDecompressionSessionSetNonScheduledDisplayTime failed" << std::endl ;
	 return err ;
    }

    return noErr ;
  }

  // --------------------------------------------------------------------------

  double
  qtSgImageSource::getRate(void) {
    Fixed rate = 0 ; 
    OSStatus err = SGGetFrameRate(channel, &rate) ;
    if (err!=noErr)
	 std::cerr << "qtSgImageSource: SGGetFrameRate failed (" << err << ")" << std::endl ;
    return FixedToFloat(rate) ;
  }

  bool
  qtSgImageSource::setRate(double frameRate) {
    OSStatus err = SGSetFrameRate(channel, FloatToFixed(frameRate)) ;
    if (err!=noErr)
	 std::cerr << "qtSgImageSource: SGSetFrameRate failed (" << err << ")" << std::endl ;
    return err==noErr ;
  }

  // --------------------------------------------------------------------------

  qtSgImageSource::qtSgImageSource(const URI &u, Image::Encoding e) {
    doThePoolTrick() ;
    
    uri = u ;

    component = 0 ;
    channel = 0 ;
    visualContext = 0 ;
    imgDescription = 0 ;
    decompSession = 0 ;
    cvimage = 0 ;
    offscreen = 0 ;
    nSignal = 0 ;
    timer = 0 ;
    frameNumber = 0 ;

    qtqtEncoding = k24RGBPixelFormat ;
    switch (e) {
    case Image::PREFERRED:
	 target_encoding = qtEncoding = Image::ARGB ;
	 qtqtEncoding = k32ARGBPixelFormat ;
	 break ;
    case Image::YpCbCr422:
	 target_encoding = qtEncoding = Image::YpCbCr422 ;
	 qtqtEncoding = k2vuyPixelFormat ; // kYUVSPixelFormat
	 break ;
    case Image::ARGB:
	 target_encoding = qtEncoding = Image::ARGB ;
	 qtqtEncoding = k32ARGBPixelFormat ;
	 break ;
    case Image::RGB:
	 target_encoding = qtEncoding = Image::RGB ;
	 break ;
    default:
	 target_encoding = e ;
	 qtEncoding = Image::RGB ;
	 break ;
    }

    nSignature = "videoin://" + getHostName() + uri.path ;
    // std::cerr << "URI: " << uri.asString() << std::endl ;
    // std::cerr << "Signature: " << nSignature << std::endl ;
  }

  qtSgImageSource::~qtSgImageSource() {
    stop() ;
  }

  bool
  qtSgImageSource::start() {  
    if (timer) return false ;

    component = OpenDefaultComponent(SeqGrabComponentType, 0);

    OSErr err = SGInitialize(component) ;
    if (err!=noErr) {
	 std::cerr << "qtSgImageSource: SGInitialize failed (" << err << ")" << std::endl ;
	 stop() ;
	 return false ;
    }

    err = SGSetDataRef(component, 0, 0, seqGrabDontMakeMovie);
    if (err!=noErr) {
	 std::cerr << "qtSgImageSource: SGSetDataRef failed (" << err << ")" << std::endl ;
	 stop() ;
	 return false ;
    }

    UdpSender signal(nSignalGroup, nSignalPort) ;
    signal.setMulticastTTL(0) ;
    signal.send(nSignature.c_str(),nSignature.size()) ;

    int trials=10 ;
    while (trials) {
	 // std::cerr << __FILE__ << ", " << __LINE__ << std::endl ;
	 err = SGNewChannel(component, VideoMediaType, &channel) ;
	 // std::cerr << __FILE__ << ", " << __LINE__ << std::endl ;
	 if (err==noErr) break ;
	 std::cerr << "qtSgImageSource: SGNewChannel failed (" << err << "), trying again" << std::endl ;
	 trials-- ;
	 ReactiveEngine::step(100) ; // wait a few milliseconds
    }

    if (!trials) {
	 std::cerr << "qtSgImageSource: SGNewChannel failed (" << err << ")" << std::endl ;
	 stop() ;
	 return false ;
    }

    // ------------------------------------------

    std::string ppath = uri.path ;
    if (ppath!="/any" && ppath!="") {
	 ppath = URI::decode(ppath) ;
	 ppath[0] = ppath.size()-1 ;
	 const char *ptr = ppath.c_str() ;
	 std::string path(ppath,1) ;
	 err = SGSetChannelDevice(channel, (StringPtr)ptr) ;
	 if (err) {
	   bool deviceExists = false ;
	   std::stringstream debugmsg ;
	   SGDeviceList dlist ;
	   SGGetChannelDeviceList(channel,0/*sgDeviceListIncludeInputs*/,&dlist) ;
	   SGDeviceName *devices = (*dlist)->entry ;
	   for (int d=0; d<(*dlist)->count; ++d) {
		char *cstr = (char *)devices[d].name ;
		std::string name(cstr+1, (int)*cstr) ;
		if (name==path) deviceExists = true ;
		debugmsg << " " ;
		if (devices[d].flags&sgDeviceNameFlagDeviceUnavailable) debugmsg << "[-]" ;
		else if (d==(*dlist)->selectedIndex) debugmsg << "[+]" ;
		else debugmsg << "[ ]" ;
		debugmsg << " " << URI::encode(name) << " (" << name << ")" << std::endl ;
	   }
	   SGDisposeDeviceList(component, dlist) ;
	   if (deviceExists) {
		std::cerr << "qtSgImageSource: device unavailable (" << ptr+1 << ")" << std::endl ;
		stop() ;
		return false ;
	   }
	   std::cerr << "qtSgImageSource: unknown device (" << ptr+1 << "). "
			   << "Please try one of the followings:" << std::endl
			   << debugmsg.str() << std::endl ;
	 }
    }

    // ------------------------------------------

    err = SGSetChannelUsage(channel, seqGrabRecord|seqGrabLowLatencyCapture) ;
    if (err!=noErr) {
	 std::cerr << "qtSgImageSource: SGSetChannelUsage failed (" << err << ")" << std::endl ;
	 stop() ;
	 return false ;
    }

    if (URI::getQueryArg(uri.query, "dialog")) {
	 ProcessSerialNumber psn ;
	 if (GetCurrentProcess(&psn) == noErr) {
	   CPSEnableForegroundOperation(&psn) ;
	   SetFrontProcess(&psn) ;
	 }
	 err = SGSettingsDialog(component, channel, 0, nil, 0, nil, nil) ;
	 if (err!=noErr) {
	   std::cerr << "qtSgImageSource: SGSettingsDialog failed (" << err << ")" << std::endl ;
	   stop() ;
	   return false ;
	 }
    }

    double frameRate = 0.0 ;
    URI::getQueryArg(uri.query, "framerate", &frameRate) ;
    if (frameRate>=0) {
	 // std::cerr << "Original frame rate: " << getRate() << std::endl ;
	 setRate(frameRate) ;
	 // std::cerr << "Setting frame rate to " << frameRate ;
	 frameRate = getRate() ;
	 // std::cerr << ": " << frameRate << std::endl ;
    }

    std::string imageSize = "SIF" ;
    URI::getQueryArg(uri.query, "size", &imageSize) ;
    int iwin = 0 ;
    for (iwin=0; Image::StandardSizes[iwin].name!=0; ++iwin)
	 if (imageSize==Image::StandardSizes[iwin].name) break ;
    if (Image::StandardSizes[iwin].name==0) {
	 std::cerr << "qtSgImageSource> Image size '" << imageSize << "' unknown..." << std::endl ;
	 stop() ;
	 return false ;
    }

    Rect bounds ;
    bounds.top = bounds.left = 0 ;
    bounds.right = Image::StandardSizes[iwin].width ; 
    bounds.bottom = Image::StandardSizes[iwin].height ;

    err = SGSetChannelBounds(channel, &bounds);
    if (err!=noErr) {
	 std::cerr << "qtSgImageSource: SGSetChannelBounds failed (" << err << ")" << std::endl ;
	 stop() ;
	 return false ;
    }

    // ------------------------------------------

    err = SGGetChannelBounds(channel, &bounds);
    // std::cerr << "SGGetChannelBounds: " << bounds.left << "," << bounds.top << " - " << bounds.right << "," << bounds.bottom << std::endl ;
    if (err!=noErr) {
	 std::cerr << "qtSgImageSource: SGGetChannelBounds failed (" << err << ")" << std::endl ;
	 stop() ;
	 return false ;
    }
    iWidth = bounds.right-bounds.left ;
    iHeight = bounds.bottom-bounds.top ;

    // ------------------------------------------

    err = QTNewGWorld(&offscreen, qtqtEncoding, &bounds, nil, nil, 0);
    if (err!=noErr) {
	 std::cerr << "qtSgImageSource: QTNewGWorld failed" << std::endl ;
	 stop() ;
	 return false ;
    }

    err = SGSetGWorld(channel, offscreen, nil);
    if (err!=noErr) {
	 std::cerr << "qtSgImageSource: SGSetGWorld failed" << std::endl ;
	 stop() ;
	 return false ;
    }

    visualContext = CreatePixelBufferContext(qtqtEncoding, iWidth, iHeight) ;
    if (!visualContext) {
	 std::cerr << "qtSgImageSource: CreatePixelBufferContext failed" << std::endl ;
	 stop() ;
	 return false ;
    }
    QTVisualContextRetain(visualContext);

    // ------------------------------------------

    err = QTVisualContextSetImageAvailableCallback(visualContext, cb_3_ready, (void *)this) ;
    if (err!=noErr) {
	 std::cerr << "qtSgImageSource: QTVisualContextSetImageAvailableCallback failed" << std::endl ;
	 stop() ;
	 return false ;
    }

    err = SGSetDataProc(component, cb_1_grab, (long)this);
    if (err!=noErr) {
	 std::cerr << "qtSgImageSource: SGSetDataProc failed (" << err << ")" << std::endl ;
	 stop() ;
	 return false ;
    }

    if (SGStartRecord(component)!=noErr) return false ;

    if (SGGetChannelTimeScale(channel, &timeScale)!=noErr) {
	 std::cerr << "qtSgImageSource: SGGetChannelTimeScale failed" << std::endl ;
	 stop() ;
	 return false ;
    }

    imgDescription = (ImageDescriptionHandle)NewHandle(0) ;
    if (SGGetChannelSampleDescription(channel, Handle(imgDescription))!=noErr) {
	 std::cerr << "qtSgImageSource: SGGetChannelSampleDescription failed" << std::endl ;
	 stop() ;
	 return false ;
    }

    ICMDecompressionTrackingCallbackRecord trackingCallback = {&cb_2_decompress, 0} ;
    if (ICMDecompressionSessionCreateForVisualContext(NULL, imgDescription, NULL,
										    visualContext, &trackingCallback, 
										    &decompSession)!=noErr) {
	 std::cerr << "qtSgImageSource: ICMDecompressionSessionCreateForVisualContext failed" << std::endl ;
	 stop() ;
	 return false ;
    }

    if (! URI::getQueryArg(uri.query, "locked")) {
	 try {
	   nSignal = new UdpReceiver(nSignalGroup,nSignalPort) ;
	   subscribeTo(nSignal) ;
	 } catch (...) {
	   nSignal = 0 ;
	 }
    }

    frameCount = 0 ; previousImageTime = TimeStamp::undef ; sampler.start() ;
    if (frameRate>0)
	 timer = TimeKeeper::create(TimeKeeper::second/(1.3*frameRate), true) ;
    else
	 timer = TimeKeeper::create(TimeKeeper::second/60, true) ;
    subscribeTo(timer) ;

    return true ;
  }

  void
  qtSgImageSource::react(Observable* obs) {
    if (!timer) return ;

    if (nSignal && obs==nSignal) {
	 unsigned char *data ;
	 unsigned int size ;
	 if (nSignal->receive(&data, &size)
		&& !strncmp(nSignature.c_str(),(const char *)data,size)) {
	   std::cerr << "qtSgImageSource::react: stream preempted..." << std::endl ;
	   stop() ;
	 }
    }

    if (obs==timer) SGIdle(component) ;
  }

  bool
  qtSgImageSource::getNextImage(Image * img, TimeStamp::inttype reftime) {
    if (!timer || !frameCount || reftime>=lastImage.getTimeStamp()) return false ;
    previousImageTime = lastImage.getTimeStamp() ;
    bool ok = convertImage(&lastImage, target_encoding) ;
    if (ok) img->linkDataFrom(lastImage) ;
    return ok ;
  }

  bool
  qtSgImageSource::stop(void) {
    sampler.stop() ;

    if (nSignal) {
	 unsubscribeFrom(nSignal) ;
	 delete nSignal ;
	 nSignal = 0 ;
    }

    if (component) SGStop(component) ;
    if (cvimage) {
	 CVPixelBufferUnlockBaseAddress(cvimage, 0) ;
	 CVPixelBufferRelease(cvimage) ;
    }
    if (decompSession) ICMDecompressionSessionRelease(decompSession) ;
    if (imgDescription) DisposeHandle(Handle(imgDescription)) ;
    
    if (timer) {
	 unsubscribeFrom(timer) ;
	 delete timer ;
	 timer = 0 ;
    }

    if (channel) SGDisposeChannel(component, channel);
    if (component) CloseComponent(component);
    if (offscreen) DisposeGWorld(offscreen) ;
    if (visualContext) QTVisualContextRelease(visualContext) ;

    component = 0 ;
    channel = 0 ;
    visualContext = 0 ;
    imgDescription = 0 ;
    decompSession = 0 ;
    cvimage = 0 ;
    offscreen = 0 ;
    nSignal = 0 ;
    timer = 0 ;
    frameNumber = 0 ;

    notifyObservers() ;
    return true ;
  }

  // --------------------------------------------------------------------------

}
