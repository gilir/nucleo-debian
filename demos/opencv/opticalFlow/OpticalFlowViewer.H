#ifndef OpticalFlowViewer_H
#define OpticalFlowViewer_H

#include <nucleo/gl/window/glWindow.H>
#include <nucleo/gl/text/glString.H>

#include "OpticalFlow.H"

using namespace nucleo ;

class OpticalFlowViewer : public ReactiveObject {

protected:

  OpticalFlow *flow ;
  int tmin, tmax ;

  glWindow *window ;
  unsigned int window_width, window_height ;

  bool showImage ;
  glTexture *texture ;

  enum verbosity {V_NOTHING=0, V_FPS=1, V_FPS_AND_STATS=2, V_MAX=3} verb ;
  glString *message ;

  void react(Observable *obs) ;

public:

  OpticalFlowViewer(OpticalFlow *flow, int tmin, int tmax) ;
  ~OpticalFlowViewer(void) ;

} ;

#endif
