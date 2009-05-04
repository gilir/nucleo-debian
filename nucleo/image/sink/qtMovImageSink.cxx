/*
 *
 * nucleo/image/sink/qtMovImageSink.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/sink/qtMovImageSink.H>
#include <nucleo/image/processing/basic/Paint.H>

#include <stdexcept>

extern "C" {
  OSErr NativePathNameToFSSpec(char *, FSSpec *, long) ;
}

namespace nucleo {

  // ---------------------------------------------------

  qtMovImageSink::qtMovImageSink(const URI &uri) {
    _filename = uri.opaque!="" ? uri.opaque : uri.path ;

    std::string query = uri.query ;
    std::string codec, quality ;

    URI::getQueryArg(query, "codec", &codec) ;
    _codec = kRawCodecType ;
    if (codec=="video") {
	 _codec = kVideoCodecType ;
    } else if (codec=="raw") {
	 _codec = kRawCodecType ;
    } else if (codec=="cinepak") {
	 _codec = kCinepakCodecType ;
    } else if (codec=="dvpal") {
	 _codec = kDVCPALCodecType ;
    } else if (codec=="dvntsc") {
	 _codec = kDVCNTSCCodecType ;
    } else if (codec=="mpeg")
	 _codec = kMpegYUV420CodecType ;
    else if (codec=="mpeg4")
	 _codec = kMPEG4VisualCodecType ;
    else if (codec=="h261")
	 _codec = kH261CodecType ;
    else if (codec=="h263")
	 _codec = kH263CodecType ;
    else if (codec=="h264")
	 _codec = kH264CodecType ;
    else if (codec=="indeo4")
	 _codec = kIndeo4CodecType ;
    else if (codec=="sorenson3")
	 _codec = kSorenson3CodecType ;
    else if (codec=="pixlet")
	 _codec = kPixletCodecType ;
    else if (codec!="") {
	 std::cerr << "qtMovImageSink: unknown codec '" << codec << "'" 
			 << " (try video, raw, cinepak, dvpal, dvntsc, mpeg, mpeg4, h261, h263, h264, indeo4, sorenson3 or pixlet)"
			 << std::endl ;
    }

    URI::getQueryArg(query, "quality", &quality) ;
    _quality = codecNormalQuality ;
    if (quality=="min") {
	 _quality = codecMinQuality ;
    } else if (quality=="low") {
	 _quality = codecLowQuality ;
    } else if (quality=="normal") {
	 _quality = codecNormalQuality ;
    } else if (quality=="high") {
	 _quality = codecHighQuality ;
    } else if (quality=="max") {
	 _quality = codecMaxQuality ;
    } else if (quality=="lossless") {
	 _quality = codecLosslessQuality ;
    } else if (quality!="") {
	 std::cerr << "qtMovImageSink: unknown quality '" << quality << "'"
			 << " (try min, low, normal, high, max or lossless)"
			 << std::endl ;
    }

    _framerate = 0 ;
    URI::getQueryArg(query, "framerate", &_framerate) ;

    _movie = 0 ;
    _track = 0 ;
    _media = 0 ;
    _dataHandler = 0;
    _lastTime = 0 ;

    _state = CLOSED ;
  }

  qtMovImageSink::~qtMovImageSink() {
    stop() ;
  }

  // ---------------------------------------------------

  bool
  qtMovImageSink::start(void) {
    _state = SEMI_OPENED ;
    frameCount = 0 ; sampler.start() ;
    return true ;
  }

  // ---------------------------------------------------

  bool
  qtMovImageSink::createMovieFile(unsigned int width, unsigned int height) {
    EnterMovies() ;

    if (_filename[0]!='/' && _filename[0]!='.')
	 // FIXME: ugly fix for a weird bug. Without this,
	 // QTNewDataReferenceFromFullPathCFString fails on filenames
	 // like toto.mov
	 _filename = "./"+_filename ;

    CFStringRef outPath = CFStringCreateWithCString(NULL, _filename.c_str(), CFStringGetSystemEncoding());
    if (!outPath)
	 throw std::runtime_error("qtMovImageSource: CFStringCreateWithCString failed") ;

    OSType myDataRefType ;
    Handle myDataRef = NULL ;
    OSErr err = QTNewDataReferenceFromFullPathCFString(outPath, kQTPOSIXPathStyle, 0, &myDataRef, &myDataRefType);
    if (err!=noErr)
	 throw std::runtime_error("qtMovImageSource: QTNewDataReferenceFromFullPathCFString failed") ;

    err = CreateMovieStorage(myDataRef, myDataRefType, 'TVOD', 0, 
					    createMovieFileDeleteCurFile|createMovieFileDontCreateResFile, 
					    &_dataHandler, &_movie) ;
    if (err!=noErr) {
	 std::cerr << "qtMovImageSink: CreateMovieStorage failed for " << _filename << " (" << err << ")" << std::endl ;
	 return false ;
    }

    // ---------------------------------

    _track = NewMovieTrack(_movie,
					  FixRatio(width,1), FixRatio(height,1),
					  kNoVolume);
    err = GetMoviesError() ;
    if (err != noErr) {
	 std::cerr << "qtMovImageSink: NewMovieTrack failed (" << err << ")" << std::endl ;
	 return false ;
    }

    // ---------------------------------

    _media = NewTrackMedia(_track, VideoMediaType, 1000 /* Time scale */, NULL, 0) ;
    err = GetMoviesError() ;
    if (err != noErr) {
	 std::cerr << "qtMovImageSink: NewTrackMedia failed (" << err << ")" << std::endl ;
	 return false ;
    }

    // ---------------------------------
    
    err = BeginMediaEdits(_media);
    if (err != noErr) {
	 std::cerr << "qtMovImageSink: BeginMediaEdits failed (" << err << ")" << std::endl ;
	 return false ;
    }

    // ---------------------------------

    _image.prepareFor(width, height, Image::ARGB) ;

    Rect bounds ;
    bounds.top = bounds.left = 0 ;
    bounds.right = width ;
    bounds.bottom = height ;

    err = QTNewGWorldFromPtr(&_gworld, k32ARGBPixelFormat, &bounds,
					    NULL, NULL, 0,
					    _image.getData(), 4*width) ;

    if (err != noErr) {
	 std::cerr << "qtMovImageSink: QTNewGWorldFromPtr failed (" << err << ")" << std::endl ;
	 return false ;
    }

    _pxmpHdl = GetGWorldPixMap(_gworld) ;
    if (_pxmpHdl == NULL) {
	 std::cerr << "qtMovImageSink: GetGWorldPixMap failed (" << err << ")" << std::endl ;
	 return false ;
    }

    long maxComprSize = 0 ;
    err = GetMaxCompressionSize(_pxmpHdl,
						  &bounds, 
						  0, // let ICM choose depth
						  _quality, _codec, (CompressorComponent)anyCodec,
						  &maxComprSize);
    if (err != noErr) {
	 std::cerr << "qtMovImageSink: GetMaxCompressionSize failed (" << err << ")" << std::endl ;
	 return false ;
    } 

    _cmprDtHdl = NewHandle(maxComprSize);
    if (_cmprDtHdl == NULL) {
	 std::cerr << "qtMovImageSink: NewHandle failed (" << err << ")" << std::endl ;
	 return false ;
    }
    HLockHi(_cmprDtHdl);

    _imgDscHdl = (ImageDescriptionHandle)NewHandle(4);
    if (_imgDscHdl == NULL) {
	 std::cerr << "qtMovImageSink: ImageDescriptionHandle NewHandle failed (" << err << ")" << std::endl ;
	 return false ;
    }

    _state = OPENED ;
    return true ;
  }

  // ---------------------------------------------------

  bool
  qtMovImageSink::handle(Image *img) {
    if (_state==CLOSED) return false ;

    if (_state==SEMI_OPENED && !createMovieFile(img->getWidth(), img->getHeight())) {
	 stop() ;
	 return false ;
    }

    if (!_framerate) {
	 long duration = 10 ;
	 if (_lastTime>0) {
	   duration = img->getTimeStamp() - _lastTime ;
	   OSErr err = AddMediaSample(_media, _cmprDtHdl,
							0 /*no offset*/, (**_imgDscHdl).dataSize, 
							duration,
							(SampleDescriptionHandle)_imgDscHdl, 
							1 /*one sample*/, 0, NULL) ;
	   if (err!=noErr) {
		std::cerr << "qtMovImageSink: AddMediaSample failed (" << err << ", " << GetMoviesError()  << "), duration was " << duration << std::endl ;
	   } else 
		_lastTime = img->getTimeStamp() ;
	 } else
	   _lastTime = img->getTimeStamp() ;
    }

    drawImageInImage(img, &_image, 0, 0) ;

    Rect bounds = {0,0,_image.getHeight(),_image.getWidth()} ;
    OSErr err = CompressImage(_pxmpHdl, &bounds, _quality, _codec,
						_imgDscHdl, *_cmprDtHdl) ;
    if (err != noErr)
	 std::cerr << "qtMovImageSink: CompressImage failed (" << err << ", " << GetMoviesError() << ")" << std::endl ;

    if (_framerate) {
	 OSErr err = AddMediaSample(_media, _cmprDtHdl,
						   0 /*no offset*/, (**_imgDscHdl).dataSize, 
						   (long)(1000/_framerate),
						   (SampleDescriptionHandle)_imgDscHdl, 
						   1 /*one sample*/, 0, NULL) ;
	 if (err != noErr)
	   std::cerr << "qtMovImageSink: AddMediaSample failed (" << err << ", " << GetMoviesError()  << "), framerate is " << _framerate << std::endl ;
    }

    frameCount = 0 ; sampler.tick() ;
    
    return true ;
  }

  // ---------------------------------------------------

  bool
  qtMovImageSink::stop(void) {
    if (_state==CLOSED) return true ;

    bool result = true ;
    _state = CLOSED ;
    sampler.stop() ;

    if (_media) {

	 if (_lastTime>0) {
	   OSErr err = AddMediaSample(_media, _cmprDtHdl,
							0 /*no offset*/, (**_imgDscHdl).dataSize, 
							40,
							(SampleDescriptionHandle)_imgDscHdl, 
							1 /*one sample*/, 0, NULL) ;

	   if (err != noErr)
		std::cerr << "qtMovImageSink: final AddMediaSample failed (" << err << ", " << GetMoviesError()  << ")" << std::endl ;
	 }

	 OSErr err = EndMediaEdits(_media) ;
	 if (err != noErr) {
	   std::cerr << "qtMovImageSink: EndMediaEdits failed (" << err << ")" << std::endl ;
	   result = false ;
	 }

	 err = InsertMediaIntoTrack(_track, 0, 0, GetMediaDuration(_media), fixed1) ;
	 if (err != noErr) {
	   std::cerr << "qtMovImageSink: InsertMediaIntoTrack failed (" << err << ")" << std::endl ;
	   result = false ;
	 }

    }

    if (_imgDscHdl) DisposeHandle((Handle)_imgDscHdl) ;
    if (_cmprDtHdl) DisposeHandle(_cmprDtHdl) ;
    if (_gworld) DisposeGWorld(_gworld) ;

    if (_dataHandler) {
	 OSErr err = AddMovieToStorage(_movie, _dataHandler) ;
	 if (err)
	   std::cerr << "qtMovImageSink: AddMovieToStorage failed (" << err << ")" << std::endl ;
	 CloseMovieStorage(_dataHandler) ;
	 _dataHandler = 0 ;
    }

    if (_movie) DisposeMovie(_movie) ;

    _gworld = 0 ;
    _cmprDtHdl = 0 ;
    _imgDscHdl = 0 ;
    _movie = 0 ;
    _track = 0 ;
    _media = 0 ;
    _lastTime = 0 ;
    return result ;
  }

  // ---------------------------------------------------

}

