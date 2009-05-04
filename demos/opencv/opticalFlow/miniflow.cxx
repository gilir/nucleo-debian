#include <nucleo/nucleo.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/processing/basic/Paint.H>
#include <nucleo/helpers/OpenCV.H>
#include <nucleo/image/sink/ImageSink.H>

#include <stdexcept>

using namespace nucleo ;

static inline void showVector(float fx1, float fy1, float fx2, float fy2,
						Image *flow_image) {
  unsigned int x1 = (unsigned int)fx1 ;
  unsigned int y1 = (unsigned int)fy1 ;
  unsigned int x2 = (unsigned int)fx2 ;
  unsigned int y2 = (unsigned int)fy2 ;
  drawRectangle(flow_image, x1-1,y1-1,x1+1,y1+1, 0,0,0,255) ;
  drawLine(flow_image, x1,y1,x2,y2, 100,100,100,255) ;
}

int
main(int argc, char **argv) {
  try {
    char *SOURCE="videoin:", *FLOW = "glwindow:?title=flow" ;
    int GRID_SIZE=0, GOOD_FEATURES=150 ;
    int TMIN=3, TMAX=400 ;

    if (parseCommandLine(argc, argv, "i:o:g:G:t:T:", "ssiiii",
					&SOURCE,&FLOW, &GRID_SIZE,&GOOD_FEATURES,&TMIN,&TMAX)<0) {
	 std::cerr << std::endl << argv[0] 
			 << " [-i source] [-o sink]"
			 << " [-g grid-size | -G nb-good-features]"
			 << " [-t min-dist-threshold] [-T max-dist-threshold]"
			 << std::endl ;
	 exit(1) ;
    }

    Image current, previous, flow_image ;
    IplImage *eig_image=0, *temp_image=0, *pyramid1=0, *pyramid2=0 ;
    int maxFeatures = GRID_SIZE ? GRID_SIZE*GRID_SIZE : GOOD_FEATURES ;    
    CvPoint2D32f features1[maxFeatures], features2[maxFeatures] ;
    char status[maxFeatures] ;
    CvSize winSize = cvSize(4,4) ; // 2,3,4,5,6,7
    int level = 2 ; // 2,3,4
    CvTermCriteria crit = cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,
								 20, .03) ;

    ImageSource *source = ImageSource::create(SOURCE) ;
    source->start() ;
    ImageSink *control = ImageSink::create("glwindow:?title=input") ;
    control->start() ;
    ImageSink *flow = ImageSink::create(FLOW) ;
    flow->start() ;

    unsigned int frameCount = 0 ;

    while (flow->getState()!=ImageSink::STOPPED 
		 && control->getState()!=ImageSink::STOPPED
		 && source->getState()!=ImageSource::STOPPED) {
	 ReactiveEngine::step() ;

	 if (!source->getNextImage(&current)) continue ;

	 frameCount++ ;
	 control->handle(&current) ;

	 unsigned int width = current.getWidth() ;
	 unsigned int height = current.getHeight() ;
	 convertImage(&current, Image::L) ;
	 if (frameCount==1) {
	   flow_image.prepareFor(width, height, Image::RGB) ;
	   CvSize size = {width, height} ;
	   eig_image = cvCreateImage(size, IPL_DEPTH_32F, 1) ;
	   temp_image = cvCreateImage(size, IPL_DEPTH_32F, 1) ;
	   pyramid1 = cvCreateImage(size, IPL_DEPTH_8U, 1 ) ;
	   pyramid2 = cvCreateImage(size, IPL_DEPTH_8U, 1 ) ;
	   previous.stealDataFrom(current) ;
	   continue ;
	 }

	 // ---------------

	 IplImage *img1 = getOpenCVImage(&previous) ;
	 IplImage *img2 = getOpenCVImage(&current) ;

	 int nbFeatures = 0 ;
	 if (GRID_SIZE) {
	   double offset = 5.0 ;
	   double nx = (width-2.0*offset)/(GRID_SIZE-1) ;
	   double ny = (height-2.0*offset)/(GRID_SIZE-1) ;
	   for (int n=0; n<GRID_SIZE; ++n)
		for (int m=0; m<GRID_SIZE; ++m) {
		  features1[nbFeatures].x = offset + m*nx ;
		  features1[nbFeatures].y = offset + n*ny ;
		  nbFeatures++ ;
		}
	 } else {
	   nbFeatures = maxFeatures ;
	   cvGoodFeaturesToTrack(img1, eig_image, temp_image, features1, &nbFeatures,
						.02 /*quality_level*/, 10. /*min_distance*/) ;
#if 0
	   if (nbFeatures) // Seems this only makes things slower...
		cvFindCornerSubPix(img1, features1, nbFeatures, 
					    winSize, cvSize(-1,-1), crit) ;
#endif
	 }

	 if (nbFeatures>0)
	   cvCalcOpticalFlowPyrLK(img1, img2, pyramid1, pyramid2,
						 features1, features2, nbFeatures,
						 winSize, level, status, NULL, crit, 0) ;

	 cvReleaseImageHeader(&img1) ;
	 cvReleaseImageHeader(&img2) ;

	 previous.stealDataFrom(current) ;

	 // ---------------

	 paintImage(&flow_image,255,255,255,255) ;
	 for (int i=0; i<nbFeatures; ++i) {
	   if (status[i]!=1) continue ;
	   if (features2[i].x<0 || features2[i].x>=width) continue ;
	   if (features2[i].y<0 || features2[i].y>=height) continue ;
	   float dx = features2[i].x - features1[i].x ;
	   float dy = features2[i].y - features1[i].y ;
	   float d = dx*dx + dy*dy ;
	   if (d<TMIN || d>TMAX) continue ;
	   showVector(features1[i].x,features1[i].y,
			    features2[i].x,features2[i].y,
			    &flow_image) ;
	 }
	 flow->handle(&flow_image) ;
    }

    cvReleaseImageHeader(&eig_image) ;
    cvReleaseImageHeader(&temp_image) ;
    cvReleaseImageHeader(&pyramid1) ;
    cvReleaseImageHeader(&pyramid2) ;

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }

  return 0 ;
}
