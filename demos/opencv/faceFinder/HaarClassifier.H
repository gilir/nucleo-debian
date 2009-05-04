#ifndef HaarClassifier_H
#define HaarClassifier_H

#include <nucleo/helpers/OpenCV.H>

class HaarClassifier {

protected:

  CvHaarClassifierCascade *cascade ;
  CvMemStorage *storage ;

public:

  CvSeq *objects ;

  HaarClassifier(const char *cascadeFilename) ;
  ~HaarClassifier(void) ;

  void findObjects(IplImage *opencv_image) ;

} ;

#endif
