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
    char *METHOD = "dilate" ;
    int ITERATIONS = 4 ;
    char *SINK = "glwindow:?fps" ;

    if (parseCommandLine(argc, argv, "i:o:m:n:", "sssi",
					&SOURCE,&SINK,&METHOD,&ITERATIONS)<0) {
	 std::cerr << std::endl << argv[0] 
			 << " [-i source] [-m method (erode or dilate)] [-n iterations] [-o sink]"
			 << std::endl ;
	 exit(1) ;
    }

    ImageSource *source = ImageSource::create(SOURCE) ;
    source->start() ;

    Image image ;

    ImageSink *sink = ImageSink::create(SINK) ;
    sink->start() ;

    while (sink->getState()!=ImageSink::STOPPED && source->getState()!=ImageSource::STOPPED) {
	 ReactiveEngine::step() ;
	 if (source->getNextImage(&image)) {
	   IplImage *opencv_image = getOpenCVImage(&image) ;

#if 1
	   if (strcmp(METHOD,"dilate"))
		cvErode(opencv_image, opencv_image, 0, ITERATIONS) ;
	   else
		cvDilate(opencv_image, opencv_image, 0, ITERATIONS) ;
#else
	   cvErode(opencv_image, opencv_image, 0, ITERATIONS) ;
	   cvDilate(opencv_image, opencv_image, 0, ITERATIONS) ;
#endif

	   cvReleaseImageHeader(&opencv_image) ;
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
