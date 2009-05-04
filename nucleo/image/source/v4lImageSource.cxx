/*
 *
 * nucleo/plugins/v4l/v4lImageSource.cxx  --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/source/v4lImageSource.H>

#include <nucleo/utils/StringUtils.H>

#include <nucleo/image/processing/basic/Resize.H>

#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/network/udp/UdpSender.H>

#include <cstdio>

#include <sys/errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/mman.h>

#include <unistd.h>

#define DEBUG_LEVEL 0

// ------------------------------------------------------------------------------------------------------

namespace nucleo {

  static const char *vsSignalGroup = "225.0.0.252" ;
  static const int vsSignalPort = 5556 ;

  static void
  vsGetVideoHardware(std::string signature) {
    UdpSender signal(vsSignalGroup, vsSignalPort) ;
    signal.setMulticastTTL(0) ;
    signal.send(signature.c_str(),signature.size()) ;
    usleep(100000) ; // wait a little...
  }


  static void
  debugCaps(struct video_capability *vid_caps) {
#if DEBUG_LEVEL>=1
    std::cerr << "v4l> using '" << vid_caps->name << "'" << std::endl ;
#endif
#if DEBUG_LEVEL>=2
    std::cerr << "   type:" ;
    if (vid_caps->type&VID_TYPE_CAPTURE) std::cerr << " CAPTURE" ;
    if (vid_caps->type&VID_TYPE_TUNER) std::cerr << " TUNER" ;
    if (vid_caps->type&VID_TYPE_TELETEXT) std::cerr << " TELETEXT" ;
    if (vid_caps->type&VID_TYPE_OVERLAY) std::cerr << " OVERLAY" ;
    if (vid_caps->type&VID_TYPE_CHROMAKEY) std::cerr << " CHROMAKEY" ;
    if (vid_caps->type&VID_TYPE_CLIPPING) std::cerr << " CLIPPING" ;
    if (vid_caps->type&VID_TYPE_FRAMERAM) std::cerr << " FRAMERAM" ;
    if (vid_caps->type&VID_TYPE_SCALES) std::cerr << " SCALES" ;
    if (vid_caps->type&VID_TYPE_MONOCHROME) std::cerr << " MONOCHROME" ;
    if (vid_caps->type&VID_TYPE_SUBCAPTURE) std::cerr << " SUBCAPTURE" ;
    std::cerr << std::endl ;
    std::cerr << "   channels: " << vid_caps->channels << std::endl ;
    std::cerr << "   audios: " << vid_caps->audios << std::endl ;
    std::cerr << "   capture size: " << vid_caps->minwidth << "x" << vid_caps->minheight ;
    std::cerr << " - " << vid_caps->maxwidth << "x" << vid_caps->maxheight << std::endl ;
#endif
  }

  static void
  debugWin(struct video_window *vid_win) {
#if DEBUG_LEVEL>=2
    std::cerr << "v4l> video_window" << std::endl ;
    std::cerr << "   image: " << vid_win->x << "," << vid_win->y << " " << vid_win->width << "x" << vid_win->height << std::endl ;
    std::cerr << "   chromakey: " << vid_win->chromakey << std::endl ;
    std::cerr << "   flags: " << vid_win->flags << std::endl ;
#endif
  }

  static void
  debugPict(struct video_picture *vid_pic) {
#if DEBUG_LEVEL>=2
    std::cerr << "v4l> video_picture" << std::endl ;
    std::cerr << "   brightness: " << vid_pic->brightness << std::endl ;
    std::cerr << "   hue: " << vid_pic->hue << std::endl ;
    std::cerr << "   colour: " << vid_pic->colour << std::endl ;
    std::cerr << "   contrast: " << vid_pic->contrast << std::endl ;
    std::cerr << "   whiteness: " << vid_pic->whiteness << std::endl ;
    std::cerr << "   depth: " << vid_pic->depth << std::endl ;
    std::cerr << "   palette: " << vid_pic->palette << std::endl ;
#endif
  }

  static void
  debugChannel(struct video_channel *vid_channel) {
#if DEBUG_LEVEL>=2
    std::cerr << "v4l> video_channel" << std::endl ;
    std::cerr << "   channel: " << vid_channel->channel << std::endl ;
    std::cerr << "   name: '" << vid_channel->name << "'" << std::endl ;
    std::cerr << "   tuners: " << vid_channel->tuners << std::endl ;
    std::cerr << "   flags: " << vid_channel->flags << std::endl ;
    std::cerr << "   type: " << (vid_channel->type==VIDEO_TYPE_TV?"TV":"CAMERA") << std::endl ;
    std::cerr << "   norm: " << vid_channel->norm << std::endl ;
#endif
  }

  // ------------------------------------------------------------------------------------------------------

  v4lImageSource::v4lImageSource(const URI& uri, Image::Encoding encoding) {
    _iState = CLOSED ;

    source_encoding = target_encoding = encoding ;
    
    std::string device, channel, path = uri.path ;
    std::string::size_type lpath = path.length() ;
    std::string::size_type pos = path.find("/",1) ;
    if (pos!=std::string::npos) {
	 device.assign(path,1,pos-1) ;
	 if (pos<lpath-1) channel.assign(path,pos+1,lpath-pos) ;
	 else channel = "any" ;
    } else {
	 if (lpath>1) device.assign(path,1,lpath-1) ;
	 else device = "any" ;
	 channel = "any" ;
    }

    std::string query = uri.query ;

    std::string size ;
    URI::getQueryArg(query, "size", &size) ;

    double framerate=0.0, pause=0.0 ;
    if (URI::getQueryArg(query, "framerate", &framerate) && framerate)
	 pause = 1.0/framerate ;

    _use_mmap = URI::getQueryArg(query, "mmap") ;

    bool locked = URI::getQueryArg(query, "locked") ;

    // ----------------------------------------------------------------------------------------------------
    // Open the specified device

    char fulldevicename[64] ;
    if (device=="any") {
	 strcpy(fulldevicename,"/dev/video0") ;
	 device = "video0" ;
    } else sprintf(fulldevicename,"/dev/%s",device.c_str()) ;

    char hostname[128] ;
    if (!gethostname(hostname,128)) 
	 _vsSignature = hostname ;
    else
	 _vsSignature = "xxx" ;
    _vsSignature = _vsSignature + device + "/"+channel ;

    // Try several times (gives time for previous device user to release it)
    for (int trials=0; ; ++trials) {
	 try { vsGetVideoHardware(_vsSignature) ; } catch (...) {}

	 _videodev = open (fulldevicename, O_RDWR);
	 if (_videodev!=-1)  break ; // succeeded

	 if (trials>12) {
#if DEBUG_LEVEL>=1
	   std::cerr << "v4l> can't open " << fulldevicename ;
	   if (errno<sys_nerr) std::cerr << " (" << sys_errlist[errno] << ")" ;
	   std::cerr << std::endl ;
#endif
	   return ;
	 }
    }

    // ----------------------------------------------------------------------------------------------------
    // Get device capability information and default settings (palette, brightness, etc.)

    struct video_capability vid_caps ;
    if (ioctl (_videodev, VIDIOCGCAP, &vid_caps) == -1) {
	 std::cerr << "v4l> can't read device capability " ;
	 if (errno<sys_nerr) std::cerr << " (" << sys_errlist[errno] << ")" ;
	 std::cerr << std::endl ;
	 close(_videodev) ;
	 return ;
    }

#if DEBUG_LEVEL>=2
    debugCaps(&vid_caps) ;
#endif

    if (! (vid_caps.type & VID_TYPE_CAPTURE)) {
	 // This is not a capture device
	 close(_videodev) ;
	 return ;
    }

    struct video_picture vid_pic ;
    if (ioctl(_videodev, VIDIOCGPICT, &vid_pic) == -1) {
	 std::cerr << "v4l> VIDIOCGPICT failed" << std::endl ;
	 close(_videodev) ;
	 return ;
    }
#if DEBUG_LEVEL>=2
    debugPict(&vid_pic) ;
#endif

    struct video_window vid_win ;
    if (ioctl(_videodev, VIDIOCGWIN, &vid_win) == -1) {
	 std::cerr << "v4l> VIDIOCGWIN failed" << std::endl ;
	 close(_videodev) ;
	 return ;
    }
#if DEBUG_LEVEL>=2
    debugWin(&vid_win) ;
#endif

    // ----------------------------------------------------------------------------------------------------  
    // Channel selection

    struct video_channel vid_channel ;

    if (channel=="any") {
	 vid_channel.channel = 0 ;
	 if (ioctl(_videodev,VIDIOCGCHAN,&vid_channel) == -1) {
	   std::cerr << "v4l> can't get info from default channel" ;
	   if (errno<sys_nerr) std::cerr << " (" << sys_errlist[errno] << ")" ;
	   std::cerr << std::endl ;
	   close(_videodev) ;
	   return ;
	 }
    } else {
	 for (vid_channel.channel=0; vid_channel.channel<vid_caps.channels; vid_channel.channel++) {
	   if (ioctl(_videodev,VIDIOCGCHAN,&vid_channel) == -1) {
		std::cerr << "v4l> can't get information on channel #" << vid_channel.channel ;
		if (errno<sys_nerr) std::cerr << " (" << sys_errlist[errno] << ")" ;
		std::cerr << std::endl ;
	   } else
		if (!strcasecmp(channel.c_str(),vid_channel.name)) break ;
	 }  
	 if (vid_channel.channel==vid_caps.channels) {
	   std::cerr << "v4l> unable to find channel '" << channel << "'" << std::endl ;
	   close(_videodev) ;
	   return ;
	 }
    }
#if DEBUG_LEVEL>=2
    debugChannel(&vid_channel) ;
#endif

    // Activate the selected channel
    if (ioctl(_videodev,VIDIOCSCHAN,&vid_channel.channel) == -1) {
	 std::cerr << "v4l> can't switch to specified channel" ;
	 if (errno<sys_nerr) std::cerr << " (" << sys_errlist[errno] << ")" ;
	 std::cerr << std::endl ;
	 close(_videodev) ;
	 return ;
    }

    // ----------------------------------------------------------------------------------------------------  
    // Choosing the right palette

    struct compatPal {
	 Image::Encoding encoding ;
	 int v4lpal ;
	 int bpp ;
    } ;

    struct compatPal compatPals[] = {
	 {Image::RGB, VIDEO_PALETTE_RGB24, 24},         // 24 bits per pixel
	 {Image::YpCbCr420, VIDEO_PALETTE_YUV420P, 12}, // 12 bits per pixel
	 {Image::L, VIDEO_PALETTE_GREY, 8},             //  8 bits per pixel
	 {Image::OPAQUE,0,0}
    } ;

    int icompat=-1, iexact=-1 ;
    for (int palette=0; compatPals[palette].bpp; ++palette) {
	 vid_pic.palette = compatPals[palette].v4lpal ;
	 vid_pic.depth = compatPals[palette].bpp ;
	 if (ioctl(_videodev, VIDIOCSPICT, &vid_pic) != -1) {
	   if (target_encoding==compatPals[palette].encoding) iexact = palette ;
	   if (icompat==-1) icompat = palette ;
	 } 
	 // else std::cerr << Image::EncodingName(compatPals[palette].encoding) << " is not supported" << std::endl ;
    }

    int i = (iexact!=-1) ? iexact : icompat ;

    if (i==-1) {
	 std::cerr << "v4l> Couldn't find any compatible palette" << std::endl ;
	 close(_videodev) ;
	 return ;
    }

    source_encoding = compatPals[i].encoding ;
    vid_pic.palette = compatPals[i].v4lpal ;
    vid_pic.depth = compatPals[i].bpp ;
    if (ioctl(_videodev, VIDIOCSPICT, &vid_pic) == -1) {
	 std::cerr << "v4l> Failed to set palette and depth (this is really weird)" << std::endl ;
	 close(_videodev) ;
	 return ;
    }

    // ----------------------------------------------------------------------------------------------------
    // Memory map setup (if needed)

    if (_use_mmap) {
	 if (ioctl (_videodev, VIDIOCGMBUF, &_mmap_mbuf) == -1) {
	   std::cerr << "v4l> tried to use mmap, but VIDIOCGMBUF failed" ;
	   if (errno<sys_nerr) std::cerr << " (" << sys_errlist[errno] << ")" ;
	   std::cerr << "reverting to read..." << std::endl ;
	   _use_mmap = 0 ;
	 } else {
	   // Map device into memory
	   _data = (unsigned char *)mmap(0, _mmap_mbuf.size, PROT_READ|PROT_WRITE,MAP_SHARED,_videodev,0) ;
	   if ((unsigned char *)_data == (unsigned char *)-1) {
		std::cerr << "v4l> mmap failed, reverting to read" ;
		if (errno<sys_nerr) std::cerr << " (" << sys_errlist[errno] << ")" ;
		std::cerr << std::endl ;
		_use_mmap = 0 ;
	   } else {
		_mmap_vid.frame  = 0 ;
		_mmap_vid.format = vid_pic.palette ;
		_use_mmap = 1 ;
	   }
	 }
    } // _use_mmap

    // ----------------------------------------------------------------------------------------------------  
    // Capture window

    if (size=="") size = "SIF" ;
    int iwin = 0 ;
    for (iwin=0; Image::StandardSizes[iwin].name!=0; ++iwin)
	 if (size==Image::StandardSizes[iwin].name) break ;
    if (Image::StandardSizes[iwin].name==0) {
	 std::cerr << "v4l> Image size '" << size << "' unknown..." << std::endl ;
	 close(_videodev) ;
	 return ;
    }
    // Dimensions requested by the client
    _width = _rwidth = Image::StandardSizes[iwin].width ;
    _height = _rheight = Image::StandardSizes[iwin].height ;

    for (; Image::StandardSizes[iwin].name!=0; ++iwin) {
	 vid_win.x = vid_win.y = 0 ; // fixme ?
	 vid_win.width = Image::StandardSizes[iwin].width ;
	 vid_win.height = Image::StandardSizes[iwin].height ;

	 if (ioctl(_videodev, VIDIOCSWIN, &vid_win) != -1) {
	   // It worked, but the driver might do a little less than requested
	   ioctl(_videodev, VIDIOCGWIN, &vid_win) ;
	   _width = vid_win.width ;
	   _height = vid_win.height ;
	   break ;
	 }

#if DEBUG_LEVEL>=1
	 std::cerr << "v4l> can't capture at " << vid_win.width << "x" << vid_win.height << std::endl ;
#endif
    }

    if (Image::StandardSizes[iwin].name==0) {
	 std::cerr << "v4l> Couldn't find a supported size less or equal than '" << size << "'" << std::endl ;
	 close(_videodev) ;
	 return ;
    }

    _post_resize = (_width!=_rwidth) || (_height!=_rheight) ;
  
    float bpp = Image::getBytesPerPixel(source_encoding);
    if (bpp == 0)
	 {
	   bpp = 1.5;
	 }
    _transferSize = (int)(_width*_height*bpp) ;

    if (!_use_mmap)
	 _data = new unsigned char [_transferSize] ;
    else {
	 _mmap_vid.width  = _width ;
	 _mmap_vid.height = _height ;
	 ioctl(_videodev, VIDIOCMCAPTURE, &_mmap_vid) ;
    }

    // ----------------------------------------------------------------------------------------------------

#if DEBUG_LEVEL>=1
    std::cerr << "v4l> '" << vid_caps.name << "'/'" << vid_channel.name << "'" ;
    std::cerr << ": "<< _width << "x" << _height << " " << Image::getEncodingName(source_encoding) ;
    std::cerr << "(" << _transferSize << " " << Image::getBytesPerPixel(source_encoding) << ")";
    std::cerr << "  --> " << _rwidth << "x" << _rheight << " " << Image::getEncodingName(target_encoding) ; 
    if (_use_mmap) std::cerr << " (mmap)" ;
    std::cerr << std::endl ;
#endif

    // ----------------------------------------------------------------------------------------------------

    _vsSignal = 0 ;
    if (!locked) {
	 // Open the nucleo multicast control channel
	 try {
	   _vsSignal = new UdpReceiver(vsSignalGroup,vsSignalPort) ;
	   subscribeTo(_vsSignal) ;
	 } catch (...) {
	   _vsSignal = 0 ;
	 }
    }

    _pace = (unsigned long)(pause*TimeKeeper::second) ;

    _errors = 0 ;
    _iState = OPENED ;
  }

  // ------------------------------------------------------------------------------------------------------

  bool
  v4lImageSource::start(void) {
    if (_iState!=OPENED) return false ;

    _iState = WAITING_FOR_IMAGE ;
    frameCount = 0 ; previousImageTime = TimeStamp::undef ; sampler.start() ;
    _tk = TimeKeeper::create(_pace, true) ;
    subscribeTo(_tk) ;
    return true ;
  }

  // ------------------------------------------------------------------------------------------------------

  void
  v4lImageSource::react(Observable *obs) {
    // std::cerr << "v4lImageSource::react " ;

    if (_vsSignal && obs==_vsSignal) {
	 // std::cerr << "preempt " ;
	 unsigned char *data ;
	 unsigned int size ;
	 if (_vsSignal->receive(&data, &size) && !strncmp(_vsSignature.c_str(),(const char *)data,size)) {
#if DEBUG_LEVEL>=1
	   std::cerr << "v4L> stream preempted..." << std::endl ;
#endif
	   _iState = CLOSED ;
	   return ;
	 }
    }

    if (obs==_tk) {
	 // std::cerr << "timeout " ;
	 if (_iState==WAITING_FOR_IMAGE) {
	   // std::cerr << "ready " ;
	   _iState = READY ;
	   if (!_pendingNotifications) notifyObservers() ;
	 }
    }

    // std::cerr << std::endl ;
  }

  // ------------------------------------------------------------------------------------------------------

  bool
  v4lImageSource::getNextImage(Image *img, TimeStamp::inttype reftime) {
    if (_iState!=READY) return 0 ;

    if (_use_mmap) {
	 if (ioctl (_videodev, VIDIOCSYNC, &_mmap_vid.frame) == -1) {
	   std::cerr << "v4l> VIDIOCSYNC failed for frame #" << _mmap_vid.frame ;
	   if (errno<sys_nerr) std::cerr << " (" << sys_errlist[errno] << ")" ;
	   std::cerr << std::endl ;
	   _errors++ ;
	   if (_errors>10) { _iState = CLOSED ; notifyObservers() ; }
	   return 0 ;
	 }
    } else {
	 int nbbytes = read(_videodev, _data, _transferSize) ;

	 if (nbbytes==-1) {
	   perror("v4l> error while reading") ;
	   _errors++ ;
	   if (_errors>10) { _iState = CLOSED ; notifyObservers() ; }
	   return 0 ;
	 } else if (nbbytes!=_transferSize) {
	   std::cerr << "v4l> " << nbbytes << " bytes read instead of " << _transferSize << std::endl ;
	   _errors++ ;
	   if (_errors>10) { _iState = CLOSED ; notifyObservers() ; }
	   return 0 ;
	 }
    }

    _errors = 0 ;
  
    img->setTimeStamp() ;

    if (_use_mmap) {
	 img->setData((unsigned char *)_data+_mmap_mbuf.offsets[_mmap_vid.frame], _transferSize, Image::NONE) ;

	 _mmap_vid.frame = (_mmap_vid.frame+1)%_mmap_mbuf.frames ;

	 int retry = 10 ;
	 while (retry) {
	   int ret = ioctl (_videodev, VIDIOCMCAPTURE, &_mmap_vid) ;
	   if (!ret) break ; else --retry ;
	   usleep(50000) ;
	 }
	 if (!retry) {
	   std::cerr << "v4l> VIDIOCMCAPTURE failed" ;
	   if (errno<sys_nerr) std::cerr << " (" << sys_errlist[errno] << ")" ;
	   std::cerr << std::endl ;
	   _iState = CLOSED ; notifyObservers() ;
	 }
    } else {
	 img->setData(_data, _transferSize, Image::NONE) ;
    }

    frameCount++ ; sampler.tick() ;
    _iState = WAITING_FOR_IMAGE ;

    // Do the hard stuff (resize, convert) here to give the driver
    // some time to get the next frame
    img->setEncoding(source_encoding) ;
    img->setDims(_width, _height) ;
    if (!convertImage(img, target_encoding)) return false ;
    if (_post_resize) resizeImage(img, _rwidth, _rheight) ;
    return true ;
  }

  // ------------------------------------------------------------------------------------------------------

  bool
  v4lImageSource::stop(void) {
    if (_iState==CLOSED) return false ;

    sampler.stop() ;

    if (_iState==WAITING_FOR_IMAGE || _iState==READY) {
	 if (_use_mmap) {
	   if ( ioctl(_videodev, VIDIOCSYNC, &_mmap_vid.frame) == -1) {
		std::cerr << "v4l> last VIDIOCSYNC failed" ;
		if (errno<sys_nerr) std::cerr << " (" << sys_errlist[errno] << ")" ;
		std::cerr << std::endl ;
	   }
	   munmap (_data, _mmap_mbuf.size) ; 
	 } else
	   delete [] _data ;
    }

    close (_videodev) ;
    unsubscribeFrom(_tk) ;
    delete _tk ;
    _iState = CLOSED ;

    notifyObservers() ;
    return true ;
  }

  // ------------------------------------------------------------------------------------------------------

  v4lImageSource::~v4lImageSource(void) {
    if (_vsSignal) {
	 unsubscribeFrom(_vsSignal) ;
	 delete _vsSignal ;
    }
    stop() ;
  }

}
