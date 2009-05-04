#include <nucleo/nucleo.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/RegionOfInterest.H>
#include <nucleo/image/processing/basic/Resize.H>
#include <nucleo/image/processing/basic/Paint.H>
#include <nucleo/helpers/OpenCV.H>
#include <nucleo/image/sink/ImageSink.H>

#include "HaarClassifier.H"

#include <stdexcept>

using namespace nucleo ;

int
main(int argc, char **argv) {
  try {
    char *SOURCE = "videoin:" ;
    char *ENCODING = "PREFERRED" ;
    char *CASCADE = HAARCASCADES"haarcascade_frontalface_alt.xml" ;
    char *SINK = "glwindow:?fps" ;
    int ROISTEP = 30 ;
    int ROIEXPAND = 20 ;

    if (parseCommandLine(argc, argv, "i:e:c:o:s:r:", "ssssii",
					&SOURCE,&ENCODING,&CASCADE,&SINK,&ROISTEP,&ROIEXPAND)<0) {
	 std::cerr << std::endl << argv[0] 
			 << " [-i source] [-e encoding] [-c cascade] [-s roi-reset-step] [-r roi-expand] [-o sink]"
			 << std::endl ;
	 exit(1) ;
    }

    ImageSource *source = ImageSource::create(SOURCE, Image::getEncodingByName(ENCODING)) ;
    source->start() ;

    Image image ;
    RegionOfInterest roi ;
    HaarClassifier finder(CASCADE) ;

    ImageSink *sink = ImageSink::create(SINK) ;
    sink->start() ;

    while (sink->getState()!=ImageSink::STOPPED && source->getState()!=ImageSource::STOPPED) {
	 ReactiveEngine::step() ;
	 
	 if (source->getNextImage(&image)) {
	   Image tmp(image) ;
	   RegionOfInterest wholeImage(image) ;
	   if (!roi.maxx || !(source->getFrameCount()%ROISTEP)) roi = wholeImage ;
	   // std::cerr << "ROI: " << roi.asString() << std::endl ;
	   cropImage(&tmp, roi.minx,roi.miny,roi.maxx,roi.maxy) ;

	   IplImage *opencv_image = getOpenCVImage(&tmp) ;
	   finder.findObjects(opencv_image) ;
	   if (finder.objects->total) {
		RegionOfInterest bbox ;
		for (int i=finder.objects->total; i; --i) {
		  CvRect* r = (CvRect*)cvGetSeqElem(finder.objects, i-1) ;
		  RegionOfInterest face(r->x, r->y, r->x+r->width, r->y+r->height) ;
		  face.slide(roi.minx, roi.miny) ;
		  bbox.grow(face) ;
		  drawRectangle(&image, face.minx,face.miny,face.maxx,face.maxy,
					 150,150,150,255) ;
		}
		// std::cerr << "BBOX: " << bbox.asString() << std::endl ;		
		bbox.expand(ROIEXPAND) ;
		bbox.constrain(wholeImage) ;
		roi = bbox ;
	   }
	   cvReleaseImageHeader(&opencv_image) ;

	   if (!finder.objects->total) convertImage(&image, Image::L) ;
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
