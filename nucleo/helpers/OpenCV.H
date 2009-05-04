#ifndef n_OpenCV_H
#define n_OpenCV_H

/*
 *
 * nucleo/helpers/OpenCV.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/Image.H>
#include <nucleo/image/encoding/Conversion.H>

#ifndef _CV_H_
#include <cv.h> // OpenCV
#endif

namespace nucleo {

  // The returned image should be released with cvReleaseImageHeader
  // Note that the nucleo image might be converted to another encoding

  inline IplImage *
  getOpenCVImage(Image *image) {
    // std::cerr << "getOpenCVImage: " << image->getDescription() << std::endl ;

    const unsigned int width = image->getWidth() ;
    const unsigned int height = image->getHeight() ;

    int depth, channels ;
    switch (image->getEncoding()) {
    case Image::ARGB:
	 depth = IPL_DEPTH_8U ;
	 channels = 4 ;
	 break ;
    case Image::RGB:
	 depth = IPL_DEPTH_8U ;
	 channels = 3 ;
	 break ;
    case Image::L:
	 depth = IPL_DEPTH_8U ;
	 channels = 1 ;
	 break ;
    default:
	 convertImage(image, Image::ARGB) ;
	 depth = IPL_DEPTH_8U ;
	 channels = 4 ;
	 break ;
    }

    IplImage *opencv_image = cvCreateImageHeader(cvSize(width,height), depth, channels) ;
    if (!opencv_image) {
	 std::cerr << "getOpenCVImage: cvCreateImageHeader failed?" << std::endl ;
	 return 0 ;
    }

    opencv_image->dataOrder = 0 ; // interleaved
    opencv_image->origin = 0 ; // top-left
    opencv_image->imageSize = image->getSize() ;
    opencv_image->imageData = opencv_image->imageDataOrigin = (char*)image->getData() ;
    opencv_image->widthStep = (int)(width*image->getBytesPerPixel()) ;
    return opencv_image ;
  }

}

#endif
