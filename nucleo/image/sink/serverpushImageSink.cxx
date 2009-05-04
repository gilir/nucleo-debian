/*
 *
 * nucleo/image/sink/serverpushImageSink.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/sink/serverpushImageSink.H>

#include <nucleo/utils/FileUtils.H>
#include <nucleo/image/encoding/Conversion.H>

#include <cstdio>
#include <unistd.h>
#include <stdexcept>
#include <sstream>

namespace nucleo {

  serverpushImageSink::serverpushImageSink(const URI &uri) {
    std::string filename = uri.opaque!="" ? uri.opaque : uri.path ;
    _fd = createFile(filename.c_str()) ;
    if (_fd==-1) throw std::runtime_error("serverpushImageSink: can't create file "+filename) ;
    _spush = new ServerPush(_fd) ;

    std::string arg, query=uri.query ;
    if (!URI::getQueryArg(query, "quality", &_quality)) _quality = 60 ;

    std::string encoding ;
    if (URI::getQueryArg(query, "encoding", &encoding))
	 _encoding = Image::getEncodingByName(encoding) ;
    else
	 _encoding = Image::JPEG ;

    _active = false ;
    _close = true ;
  }

  serverpushImageSink::serverpushImageSink(int fd,
								   Image::Encoding encoding,
								   unsigned int quality,
								   bool closeAfter) {
    _fd = fd ;
    _spush = new ServerPush(_fd) ;
    _active = false ;
    _close = closeAfter ;
    _quality = quality ;
    _encoding = encoding ;
  }

  serverpushImageSink::~serverpushImageSink(void) {
    if (_close) close(_fd) ;
    delete _spush ;
  }

  bool
  serverpushImageSink::start(void) {
    _active = true ;
    frameCount = 0 ; sampler.start() ;
    return true ;
  }

  bool
  serverpushImageSink::stop(void) {
    _active = false ;
    sampler.stop() ;
    return true ;
  }

  bool
  serverpushImageSink::handle(Image *img) {
    Image tmp(*img) ;
    if (!convertImage(&tmp, _encoding, _quality)) return false ;
    std::string mimetype = tmp.getMimeType() ;
    TimeStamp::inttype timestamp = tmp.getTimeStamp() ;

    std::stringstream ssheaders ;
    ssheaders << "nucleo-framerate: " << sampler.average() << "\r\n" ; // FIXME: deprecated
    ssheaders << "nucleo-timestamp: " << (timestamp==TimeStamp::undef ? TimeStamp::createAsInt() : timestamp) << "\r\n" ;
    ssheaders << "nucleo-image-width: " << tmp.getWidth() << "\r\n" ;
    ssheaders << "nucleo-image-height: " << tmp.getHeight() ;
    std::string headers = ssheaders.str() ;

    _spush->push(mimetype.c_str(),
			  (char *)tmp.getData(), (int)tmp.getSize(),
			  headers.c_str()) ;
    frameCount++ ; sampler.tick() ;
    return true ;
  }
    
}
