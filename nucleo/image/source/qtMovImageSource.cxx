/*
 *
 * nucleo/image/source/qtMovImageSource.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/source/qtMovImageSource.H>
#include <nucleo/image/encoding/Conversion.H>

#include <stdexcept>
#include <sstream>

namespace nucleo {

  // --------------------------------------------------------------------------

  pascal OSErr
  DrawCompleteProc(Movie theMovie, long ptr) {
    qtMovImageSource *src = (qtMovImageSource*)ptr ;
    src->lastImage.setTimeStamp() ;
    src->frameCount++ ; src->sampler.tick() ;
    if (!src->_pendingNotifications) src->notifyObservers() ;
    return noErr ;
  }

  qtMovImageSource::qtMovImageSource(const URI &u, Image::Encoding e) {
    std::string filename = u.opaque!="" ? u.opaque : u.path ;
    timer = 0 ;

    EnterMovies() ;

    // ---------

    CFStringRef inPath = CFStringCreateWithCString(NULL, filename.c_str(), CFStringGetSystemEncoding());
    if (!inPath)
	 throw std::runtime_error("qtMovImageSource: CFStringCreateWithCString failed") ;

    OSType myDataRefType ;
    Handle myDataRef = NULL ;
    OSErr result = QTNewDataReferenceFromFullPathCFString(inPath, kQTPOSIXPathStyle, 0, &myDataRef, &myDataRefType);
    if (result)
	 throw std::runtime_error("qtMovImageSource: QTNewDataReferenceFromFullPathCFString failed") ;

    short actualResId = DoTheRightThing ;
    result = NewMovieFromDataRef(&movie, newMovieActive,
                                 &actualResId, myDataRef, myDataRefType);
    if (result)
	 throw std::runtime_error("qtMovImageSource: NewMovieFromDataRef failed") ;

    DisposeHandle(myDataRef);

#if 0
    if (!GetMovieIndTrackType(movie, 1, VideoMediaType, movieTrackMediaType)) {
	 DisposeMovie(movie) ;
	 std::stringstream msg ;
	 msg << filename << " has no supported video track" ;
	 throw std::runtime_error(msg.str()) ;
    }
#endif

    // ---------

#if 0
    TimeBase b = GetMovieTimeBase(_theMovie) ;
    TimeRecord tr ;
    memset(&tr, 0, sizeof(TimeRecord)) ;
#if 1
    tr.value.lo = 9700 ;
#endif
    TimeScale ts = 1000 ;
    GetTimeBaseTime(b,ts,&tr) ;
    SetTimeBaseTime(b, &tr) ;
#endif

    Rect bounds ;
    GetMovieBox(movie, &bounds) ;
    unsigned int width = bounds.right - bounds.left ;
    unsigned int height = bounds.bottom - bounds.top ;

    // k24RGBPixelFormat is MUCH slower
    OSType pixelFormat = k32ARGBPixelFormat ;
    target_encoding = (e==Image::PREFERRED) ? Image::ARGB : e ;
    qtEncoding = Image::ARGB ;

    lastImage.prepareFor(width, height, qtEncoding) ;
    GWorldPtr offWorld ;
    QTNewGWorldFromPtr(&offWorld, pixelFormat, &bounds, NULL, NULL, 0,
				   lastImage.getData(), (long)(lastImage.getBytesPerPixel(qtEncoding)*width)) ;

    SetGWorld(offWorld, NULL) ;

    player = NewMovieController(movie, &bounds, mcTopLeftMovie|mcNotVisible) ;
    SetMovieGWorld(movie, offWorld, NULL);

#if 0
    float framerate = 0.0 ;
    if (URI::getQueryArg(u.query, "framerate", &framerate)) {

	 
	 Fixed newrate = FloatToFixed(framerate) ;
	 std::cerr << GetTimeBaseEffectiveRate(GetMovieTimeBase(movie))
			 << " --> " << newrate
			 << " (" << framerate << ")" << std::endl ;
	 SetMoviePreferredRate(movie, newrate) ;
	 OSErr err = GetMoviesError() ;
	 std::cerr << "err = " << err << std::endl ;
    }
#endif
  }

  bool
  qtMovImageSource::start(void) {
    if (timer) return false ;

    SetMovieActive(movie, true);
    callback = NewMovieDrawingCompleteUPP(DrawCompleteProc);
    SetMovieDrawingCompleteProc(movie, movieDrawingCallWhenChanged, callback, (long)this); 
    SetMovieTimeValue(movie, 0) ;
    MCDoAction(player, mcActionPrerollAndPlay, (void*)Long2Fix(1));

    timer = TimeKeeper::create(TimeKeeper::second/50, true) ;
    subscribeTo(timer) ;
    frameCount = 0 ; previousImageTime = TimeStamp::undef ; sampler.start() ;

    return true ;
  }

  bool
  qtMovImageSource::getNextImage(Image *img, TimeStamp::inttype reftime) {
    if (!timer || reftime>=lastImage.getTimeStamp()) return false ;
    previousImageTime = lastImage.getTimeStamp() ;
    bool ok = convertImage(&lastImage, target_encoding) ;
    if (ok) img->linkDataFrom(lastImage) ;
    return ok ;
  }

  void
  qtMovImageSource::react(Observable*) {
    if (!timer) return ;

    MCIdle(player) ;
    if (IsMovieDone(movie)) {
	 stop() ;
	 notifyObservers() ;
    }

#if 0
    TimeScale ts = 1000 ;
    TimeBase b = GetMovieTimeBase(_theMovie) ;
    std::cerr << "[" << GetTimeBaseStartTime(b,ts,0) 
		    << " - " << GetTimeBaseTime(b,ts,0)
		    << " - " << GetTimeBaseStopTime(b,ts,0) << "] "
		    << std::endl ;
#endif
  }

  qtMovImageSource::~qtMovImageSource() {
    stop() ;

    DisposeMovieController(player) ;
    DisposeMovie(movie) ;
    DisposeMovieDrawingCompleteUPP(callback) ;
    // FIXME: should call DisposeGWorld(...) ; 
  }

  bool
  qtMovImageSource::stop() {
    if (!timer) return false ;

    sampler.stop() ;

    unsubscribeFrom(timer) ;
    delete timer ;
    timer = 0 ;
    return true ;
  }

  // --------------------------------------------------------------------------

  TimeStamp::inttype
  qtMovImageSource::getDuration(void) {
    TimeValue duration = GetMovieDuration(movie) ;
    TimeScale timescale = GetMovieTimeScale (movie) ;
#if 0
    TimeBase timebase = GetMovieTimeBase(movie) ;
    double seconds = (double)duration/timescale ;
    std::cerr << "timebase: " << timebase << std::endl ;
    std::cerr << "timescale: " << timescale << std::endl ;
    std::cerr << "duration: " << duration << " (" << seconds << " seconds)" << std::endl ;
#endif
    return TimeStamp::inttype(duration)*1000/TimeStamp::inttype(timescale) ;
  }

  // --------------------------------------------------------------------------

}
