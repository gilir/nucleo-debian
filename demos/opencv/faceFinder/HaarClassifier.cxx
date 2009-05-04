#include "HaarClassifier.H"

#include <stdexcept>
#include <iostream>

HaarClassifier::HaarClassifier(const char *cascadeFilename) {
  cascade = (CvHaarClassifierCascade*)cvLoad(cascadeFilename) ;
  if (!cascade)
    throw std::runtime_error("Could not load classifier cascade") ;
  storage = cvCreateMemStorage(0) ;
  objects = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvAvgComp), storage) ;
}


void
HaarClassifier::findObjects(IplImage *opencv_image) {
  cvClearSeq(objects) ;
  cvClearMemStorage(storage) ;
  objects = cvHaarDetectObjects(opencv_image, cascade, storage,
						  1.2,                       // scale_factor
						  3,                         // min_neighbors
						  CV_HAAR_DO_CANNY_PRUNING,  // flags
						  cvSize(40, 40)) ;          // min_size
}

HaarClassifier::~HaarClassifier(void) {
  cvReleaseMemStorage(&storage) ;
  cvReleaseHaarClassifierCascade(&cascade) ;
}
