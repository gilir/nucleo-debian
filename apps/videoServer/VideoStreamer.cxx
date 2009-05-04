/*
 *
 * apps/videoServer/VideoStreamer.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "VideoStreamer.H"

#include <nucleo/image/sink/serverpushImageSink.H>
#include <nucleo/image/sink/nudpImageSink.H>
#include <nucleo/image/processing/convolution/Blur.H>
#include <nucleo/image/encoding/Conversion.H>

#include <iostream>
#include <stdexcept>

// ---------------------------------------------------------------

VideoStreamer::VideoStreamer(TcpConnection *c, VideoService *s) {
  connection = c ;
  service = s ;

  std::stringstream uri ;
  uri << service->arg
	 << (service->arg.find("?")!=std::string::npos ? "&" : "?")
	 << "framerate=" << service->image_rate
	 << "&size=" << service->image_size ;

  source = ImageSource::create(uri.str()) ;
  subscribeTo(source) ;
  source->start() ;   
  if (source->getState()!=ImageSource::STARTED) {
    unsubscribeFrom(source) ;
    delete source ;
    throw std::runtime_error("VideoStreamer: unable to start source") ;
  }

  if (service->cmd==VideoService::NUDP) {
    std::stringstream uri_stream ;
    uri_stream << "nudp://" << service->nudpInfo 
			<< "?encoding=JPEG&quality=" << service->image_quality ;
    URI uri(uri_stream.str()) ;
    sink = new nudpImageSink(uri) ;
  } else if (service->cmd==VideoService::PUSH) {
    sink = new serverpushImageSink(connection->getFd(), Image::JPEG, service->image_quality) ;
  } else { // GRAB
    sink = 0 ;
  }

  if (sink) {
    subscribeTo(sink) ;
    sink->start() ;
    if (sink->getState()==ImageSink::STOPPED) {
	 unsubscribeFrom(source) ;
	 delete source ;
	 unsubscribeFrom(sink) ;
	 delete sink ;
	 throw std::runtime_error("VideoStreamer: unable to start sink") ;
    }
  }

  subscribeTo(connection) ;

  // std::cerr << "New VideoStreamer: " << this << std::endl ;
}

// ---------------------------------------------------------------

void
VideoStreamer::react(Observable *obs) {
  bool terminate = false ;

  if (obs==connection) {
    terminate = true ;
  } else if (obs==source && source->getState()!=ImageSource::STOPPED) {
    try {
	 if (source->getNextImage(&img)) {
	   // img.debug(std::cerr) ; std::cerr << std::endl ;
	   if (service->image_blur)
		BlurFilter::filter(&img, BlurFilter::HandV, service->image_blur, 2) ;
	   if (sink) 
		sink->handle(&img) ;
	   else {
		convertImage(&img, Image::JPEG, service->image_quality) ;
		std::stringstream response_stream ;
		response_stream << "HTTP/1.0 200 Ok" << oneCRLF
					 << "Content-Type: " << img.getMimeType() << oneCRLF
					 << "Content-Length: " << img.getSize()
					 << twoCRLF ;
		std::string response = response_stream.str() ;
		connection->send(response.c_str(), response.size(), true) ;
		connection->send((const char *)img.getData(), img.getSize(), true) ;
		terminate = true ;
	   }
	 }
    } catch (std::runtime_error e) {
	 std::cerr << e.what() << std::endl ;
	 terminate = true ;
    }
  }

  if (source->getState()==ImageSource::STOPPED) terminate = true ;
  if (sink && sink->getState()==ImageSink::STOPPED) terminate = true ;

  if (terminate) delete this ;
}

// ---------------------------------------------------------------

VideoStreamer::~VideoStreamer(void) {
  // std::cerr << "Deleting VideoStreamer " << this << std::endl ;

  unsubscribeFrom(source) ;
  delete source ;
  if (sink) {
    unsubscribeFrom(sink) ;
    delete sink ;
  }
  unsubscribeFrom(connection) ;
  delete connection ;
  delete service ;
}
