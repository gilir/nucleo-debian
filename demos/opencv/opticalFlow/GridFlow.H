#ifndef GridFlow_H
#define GridFlow_H

#include "OpticalFlow.H"

class GridFlow : public OpticalFlow {

protected:

  int grid_size, nbFeatures ;
  CvPoint2D32f *features1, *features2 ;
  IplImage *pyramid1, *pyramid2 ;
  char *status ;
  float *track_error ;
  CvSize winSize ;
  int level ;
  CvTermCriteria crit ;

  void init(int grid_size) ;
  void handleFirstFrame(void) ;
  void updateFlow(void) ;

public:

  GridFlow(const char *src, int grid_size) ;
  GridFlow(ImageSource *src, int grid_size) ;

  int getNbPairs(void) ;
  FeaturePair getPair(int i) ;

  ~GridFlow(void) ;

} ;

#endif
