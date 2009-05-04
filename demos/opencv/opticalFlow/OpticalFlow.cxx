#include "OpticalFlow.H"

// -------------------------------------------------------------------

OpticalFlow::OpticalFlow(const char *src) {
  deleteSource = true ;
  source = ImageSource::create(src,Image::L) ;
  subscribeTo(source) ;
  source->start() ;
  frameCount = 0 ;
}

OpticalFlow::OpticalFlow(ImageSource *src, bool startSource) {
  deleteSource = false ;
  source = src ;
  subscribeTo(source) ;
  if (startSource) source->start() ;
  frameCount = 0 ;
}

// -------------------------------------------------------------------

void
OpticalFlow::handleFirstFrame(void) {
  std::cerr << "OpticalFlow::handleFirstFrame: " << current.getDescription() << std::endl ;
}

void OpticalFlow::updateFlow(void) {
  if (! (frameCount%100))
    std::cerr << "OpticalFlow::updateFlow: " << source->getMeanRate() << " fps" << std::endl ;
}

void
OpticalFlow::react(Observable *obs) {
  if (obs==source) {
    Image tmp ;
    if (source->getNextImage(&tmp)) {
	 frameCount++ ;
	 convertImage(&tmp, Image::L) ;
	 if (frameCount==1) {
	   current.stealDataFrom(tmp) ;
	   if (current.dataIsLinked()) current.acquireData() ;
	   handleFirstFrame() ;
	 } else {
	   previous.stealDataFrom(current) ;
	   current.stealDataFrom(tmp) ;
	   if (current.dataIsLinked()) current.acquireData() ;
	   updateFlow() ;
	 }
    }
    notifyObservers() ;
  }
}

// -------------------------------------------------------------------

OpticalFlow::~OpticalFlow(void) {
  unsubscribeFrom(source) ;
  if (deleteSource) delete source ;
}
