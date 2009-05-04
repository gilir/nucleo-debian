/*
 *
 * nucleo/image/sink/novImageSink.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/sink/novImageSink.H>

#include <nucleo/utils/FileUtils.H>
#include <nucleo/utils/ByteOrder.H>
#include <nucleo/image/encoding/Conversion.H>

#include <sys/uio.h>

#include <stdexcept>

namespace nucleo {

  novImageSink::novImageSink(const URI &uri) {
    filename = uri.opaque!="" ? uri.opaque : uri.path ;

    if (!URI::getQueryArg(uri.query, "quality", &quality)) quality = 60 ;

    std::string encodingname ;
    if (URI::getQueryArg(uri.query, "encoding", &encodingname))
	 encoding = Image::getEncodingByName(encodingname) ;
    else
	 encoding = Image::JPEG ;

    status = STOPPED ;
  }

  bool
  novImageSink::start(void) {
    if (status==STARTED) return false ;

    fd = createFile(filename.c_str()) ;
    if (fd==-1) {
	 std::cerr << "novImageSink: can't create file " << filename << std::endl ;
	 return false ;
    }

    status = STARTED ;
    frameCount = 0 ; sampler.start() ;
    return true ;
  }

  bool
  novImageSink::stop(void) {
    if (status==STOPPED) return false ;

    sampler.stop() ;
    close(fd) ;
    status = STOPPED ;
    return true ;
  }

  bool
  novImageSink::handle(Image *img, void *xtra, uint32_t xtra_size) {
    Image tmp(*img) ;
    if (!convertImage(&tmp, encoding, quality)) return false ;
    uint32_t img_size = (uint32_t)tmp.getSize() ;

    ImageDescription desc ;
    desc.timestamp = tmp.getTimeStamp() ;
    desc.img_size = img_size ;
    desc.img_encoding = tmp.getEncoding() ;
    desc.img_width = tmp.getWidth() ;
    desc.img_height = tmp.getHeight() ;
    desc.xtra_size = xtra_size ;
    desc.swapifle() ;

    struct iovec iov[3] ;
    iov[0].iov_base = &desc ;
    iov[0].iov_len = sizeof(desc) ;
    iov[1].iov_base = tmp.getData() ;
    iov[1].iov_len = img_size ;
    iov[2].iov_base = xtra ;
    iov[2].iov_len = xtra?xtra_size:0 ;
    writev(fd, iov, xtra?3:2) ;

    frameCount++ ; sampler.tick() ;
    return true ;
  }

  void
  novImageSink::ImageDescription::swapifle(void) {
    if (!ByteOrder::isLittleEndian()) return ;
    timestamp = (TimeStamp::inttype)ByteOrder::swap64ifle((int64_t)timestamp) ;
    img_size = ByteOrder::swap32ifle(img_size) ;
    img_encoding = (Image::Encoding)ByteOrder::swap32ifle((uint32_t)img_encoding) ;
    img_width = ByteOrder::swap32ifle(img_width) ;
    img_height = ByteOrder::swap32ifle(img_height) ;
    xtra_size = ByteOrder::swap32ifle(xtra_size) ;
  }
    
}
