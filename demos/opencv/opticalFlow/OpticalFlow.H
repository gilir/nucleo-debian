#ifndef OpticalFlow_H
#define OpticalFlow_H

#include <nucleo/core/ReactiveObject.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/gl/texture/glTexture.H>
#include <nucleo/helpers/OpenCV.H>

#include <stdexcept>

using namespace nucleo ;

class OpticalFlow : public ReactiveObject {

protected:

  ImageSource *source ;
  bool deleteSource ;
  Image current, previous ;
  unsigned int frameCount ;

  virtual void handleFirstFrame(void) ;
  virtual void updateFlow(void) ;

  void react(Observable *obs) ;

public:

  OpticalFlow(const char *src) ;
  OpticalFlow(ImageSource *src, bool startSource=false) ;

  virtual ~OpticalFlow(void) ;

  unsigned long getFrameNumber(void) {
    return source->getFrameCount() ;
  }

  double getFrameRate(void) {
    return source->getMeanRate() ;
  }

  unsigned int getWidth(void) {
    return current.getWidth() ;
  }

  unsigned int getHeight(void) {
    return current.getHeight() ;
  }

  void updateTexture(glTexture *texture) {
    texture->update(&current) ;
  }

  struct FeaturePair {
    bool isValid ;
    float x1, y1 ;
    float x2, y2 ;
    FeaturePair(bool v, float a, float b, float c, float d) 
	 : isValid(v), x1(a), y1(b), x2(c), y2(d) {}
  } ;

  virtual int getNbPairs(void) {
    return 0 ;
  }

  virtual FeaturePair getPair(int i) {
    throw std::runtime_error("OpticalFlow: no vector available") ;
  }

} ;

#endif
