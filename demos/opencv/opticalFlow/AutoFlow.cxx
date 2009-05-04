#include "AutoFlow.H"

AutoFlow::AutoFlow(const char *src, int max) : OpticalFlow(src) {
  init(max) ;
}

AutoFlow::AutoFlow(ImageSource *src, int max) : OpticalFlow(src) {
  init(max) ;
}

void
AutoFlow::init(int max) {
  if (max<1) throw std::runtime_error("max: grid size must be >0") ;
  max_features = max ;
  nbFeatures = 0 ;
  features1 = new CvPoint2D32f [max_features] ;
  features2 = new CvPoint2D32f [max_features] ;
  eig_image = temp_image = pyramid1 = pyramid2 = 0 ;
  status = new char [max_features] ;
  winSize = cvSize(3,3) ; // 2,3,4,5,6,7
  level = 2 ; // 2,3,4
  crit = cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,.03) ;
  quality_level = 0.04 ; // 1.0 would keep only the "best" corner
  min_distance = 9.0 ;
  std::cerr << "AutoFlow::init: max_features=" << max_features << std::endl ;
}

void
AutoFlow::handleFirstFrame(void) {
  std::cerr << "AutoFlow::handleFirstFrame: " ;
  current.debug(std::cerr) ;
  std::cerr << std::endl ;

  CvSize size = {current.getWidth(), current.getHeight()} ;
  eig_image = cvCreateImage(size, IPL_DEPTH_32F, 1) ;
  temp_image = cvCreateImage(size, IPL_DEPTH_32F, 1) ;
  pyramid1 = cvCreateImage(size, IPL_DEPTH_8U, 1 ) ;
  pyramid2 = cvCreateImage(size, IPL_DEPTH_8U, 1 ) ;

  // std::cerr << "AutoFlow::handleFirstFrame: done" << std::endl ;
}

void
AutoFlow::updateFlow(void) {
#if 0
  std::cerr << "AutoFlow::updateFlow" << std::endl ;
  std::cerr << "    " << previous.getDescription() << std::endl ;
  std::cerr << "    " << current.getDescription() << std::endl ;
#endif
  IplImage *img1 = getOpenCVImage(&previous) ;
  IplImage *img2 = getOpenCVImage(&current) ;

  nbFeatures = max_features ;
  cvGoodFeaturesToTrack(img1, eig_image, temp_image, features1, &nbFeatures,
				    quality_level, min_distance) ;

  if (nbFeatures>0) {
    int flags = (frameCount>2) ? CV_LKFLOW_PYR_A_READY : 0 ;
    cvCalcOpticalFlowPyrLK(img1, img2, pyramid1, pyramid2,
					  features1, features2, nbFeatures,
					  winSize, level, status, NULL, crit, flags) ;
  }

  IplImage *tmp ;
  tmp = pyramid2 ; pyramid2 = pyramid1 ; pyramid1 = tmp ;

  cvReleaseImageHeader(&img1) ;
  cvReleaseImageHeader(&img2) ;
}

int
AutoFlow::getNbPairs(void) {
  return nbFeatures ;
}

OpticalFlow::FeaturePair
AutoFlow::getPair(int i) {
  if (i<0 || i>=nbFeatures) throw std::runtime_error("AutoFlow: invalid pair index") ;
  return OpticalFlow::FeaturePair(status[i],
						    features1[i].x,features1[i].y,
						    features2[i].x,features2[i].y) ;
}

AutoFlow::~AutoFlow(void) {
  cvReleaseImageHeader(&eig_image) ;
  cvReleaseImageHeader(&temp_image) ;
  cvReleaseImageHeader(&pyramid1) ;
  cvReleaseImageHeader(&pyramid2) ;
  delete [] features1 ;
  delete [] features2 ;
  delete [] status ;
}
