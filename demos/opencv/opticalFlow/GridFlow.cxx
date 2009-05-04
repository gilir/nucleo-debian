#include "GridFlow.H"

#include <stdexcept>

GridFlow::GridFlow(const char *src, int grid_size) : OpticalFlow(src) {
  init(grid_size) ;
}

GridFlow::GridFlow(ImageSource *src, int grid_size) : OpticalFlow(src) {
  init(grid_size) ;
}

void
GridFlow::init(int gs) {
  if (gs<1) throw std::runtime_error("GridFlow: grid size must be >0") ;
  grid_size = gs ;
  nbFeatures = grid_size*grid_size ;
  features1 = new CvPoint2D32f [nbFeatures] ;
  features2 = new CvPoint2D32f [nbFeatures] ;
  pyramid1 = pyramid2 = 0 ;
  status = new char [nbFeatures] ;
  track_error = new float [nbFeatures] ;
  winSize = cvSize(3,3) ; // 2,3,4,5,6,7
  level = 2 ; // 2,3,4
  crit = cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,.03) ;
  std::cerr << "GridFlow::init: grid size is " << grid_size << std::endl ;
}

void
GridFlow::handleFirstFrame() {
  std::cerr << "GridFlow::handleFirstFrame: " ;
  current.debug(std::cerr) ;
  std::cerr << std::endl ;

  unsigned int width = current.getWidth() ;
  unsigned int height = current.getHeight() ;

  CvSize size = {width, height} ;
  pyramid1 = cvCreateImage(size, IPL_DEPTH_8U, 1 ) ;
  pyramid2 = cvCreateImage(size, IPL_DEPTH_8U, 1 ) ;

  int iFeature = 0 ;
  double offset = 5.0 ;
  double nx = (width-2.0*offset)/(grid_size-1) ;
  double ny = (height-2.0*offset)/(grid_size-1) ;
  for (int n=0; n<grid_size; ++n)
    for (int m=0; m<grid_size; ++m) {
	 features1[iFeature].x = offset + m*nx ;
	 features1[iFeature].y = offset + n*ny ;
	 iFeature++ ;
    }
}

void
GridFlow::updateFlow(void) {
  std::cerr << "GridFlow::updateFlow" << std::endl ;	   
  IplImage *img1 = getOpenCVImage(&previous) ;
  IplImage *img2 = getOpenCVImage(&current) ;
  
  int flags = (frameCount>2) ? CV_LKFLOW_PYR_A_READY : 0 ;
  cvCalcOpticalFlowPyrLK(img1, img2, pyramid1, pyramid2,
					features1, features2, nbFeatures,
					winSize, level, status, track_error, crit, flags) ;

#if 0
  int nbRemoved = 0 ;
  for (int i=0; i<nbFeatures; ++i) {
    if (!status[i]) continue ;   
    // if (track_error[i]<15) continue ;
    float dx = features2[i].x - (int)features2[i].x ;
    float dy = features2[i].y - (int)features2[i].y ;
    std::cerr << dx << " " << dy << std::endl ;
    if (!dx && !dy) continue ;
    nbRemoved++ ;
    status[i] = 0 ;
  }
  std::cerr << "Removed " << nbRemoved << " features" << std::endl ;
#endif

  IplImage *tmp ;
  tmp = pyramid2 ; pyramid2 = pyramid1 ; pyramid1 = tmp ;

  cvReleaseImageHeader(&img1) ;
  cvReleaseImageHeader(&img2) ;
}

int
GridFlow::getNbPairs(void) {
  return nbFeatures ;
}

OpticalFlow::FeaturePair
GridFlow::getPair(int i) {
  if (i<0 || i>=nbFeatures) throw std::runtime_error("GridFlow: invalid pair index") ;
  return OpticalFlow::FeaturePair(status[i],
						    features1[i].x,features1[i].y,
						    features2[i].x,features2[i].y) ;
}

GridFlow::~GridFlow(void) {
  cvReleaseImageHeader(&pyramid1) ;
  cvReleaseImageHeader(&pyramid2) ;
  delete [] features1 ;
  delete [] features2 ;
  delete [] status ;
  delete [] track_error ;
}
