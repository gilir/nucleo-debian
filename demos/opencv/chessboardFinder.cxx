#include <nucleo/utils/AppUtils.H>
#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/helpers/OpenCV.H>
#include <nucleo/image/sink/ImageSink.H>

#include <stdexcept>

using namespace nucleo ;

int
main(int argc, char **argv) {
  try {
    char *SOURCE = "videoin:" ;
    char *SINK = "glwindow:?fps" ;
    int ROWS=6, COLUMNS=8 ;

    if (parseCommandLine(argc, argv, "i:o:r:c:", "ssii",
					&SOURCE,&SINK,&ROWS,&COLUMNS)<0) {
	 std::cerr << std::endl << argv[0] 
			 << " [-i source] [-c columns] [-r rows] [-o sink]"
			 << std::endl ;
	 exit(1) ;
    }

    ImageSource *source = ImageSource::create(SOURCE) ;
    source->start() ;

    Image image ;
    CvSize pattern_size = {COLUMNS-1,ROWS-1} ;
    CvPoint2D32f corners[COLUMNS*ROWS] ;

    ImageSink *sink = ImageSink::create(SINK) ;
    sink->start() ;

    while (sink->getState()!=ImageSink::STOPPED) {
	 ReactiveEngine::step() ;
	 if (source->getNextImage(&image)) {
	   IplImage *opencv_image = getOpenCVImage(&image) ;
	   int corner_count = 0 ;
	   int found = cvFindChessboardCorners(opencv_image, pattern_size,
								    corners, &corner_count,
								    CV_CALIB_CB_ADAPTIVE_THRESH) ;
	   cvDrawChessboardCorners(opencv_image, pattern_size,
						  corners, corner_count, found) ;
	   cvReleaseImageHeader(&opencv_image) ;

	   if (!found) convertImage(&image, Image::L) ;
	   sink->handle(&image) ;
	 }
    }

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }

  return 0 ;
}
