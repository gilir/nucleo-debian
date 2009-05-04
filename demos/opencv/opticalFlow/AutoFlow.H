#ifndef AutoFlow_H
#define AutoFlow_H

#include "OpticalFlow.H"

class AutoFlow : public OpticalFlow {

protected:

  int max_features, nbFeatures ;
  CvPoint2D32f *features1, *features2 ;
  IplImage *eig_image, *temp_image, *pyramid1, *pyramid2 ;
  char *status ;
  CvSize winSize ;
  int level ;
  CvTermCriteria crit ;
  double quality_level, min_distance ;

  void init(int max) ;
  void handleFirstFrame(void) ;
  void updateFlow(void) ;

public:

  AutoFlow(const char *src, int max) ;
  AutoFlow(ImageSource *src, int max) ;

  int getNbPairs(void) ;
  FeaturePair getPair(int i) ;

  ~AutoFlow(void) ;

} ;


#endif
