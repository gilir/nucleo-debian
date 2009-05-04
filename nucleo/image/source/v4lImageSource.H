#ifndef n_v4lImageSource_H
#define n_v4lImageSource_H

/*
 *
 * nucleo/image/source/v4lImageSource.H  --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/TimeKeeper.H>
#include <nucleo/core/FileKeeper.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/network/udp/UdpReceiver.H>
#include <nucleo/core/URI.H>

#include <linux/types.h>
#include <linux/videodev.h>

namespace nucleo {

  // Native encoding is RGB (could be YUV...)
  class v4lImageSource  : public ImageSource {

  private:

    int _videodev ;                // Video4Linux device

    bool _use_mmap ;               // Should we try to use mmap ?
    struct video_mmap _mmap_vid ;  // Video4Linux mmap structure
    struct video_mbuf _mmap_mbuf ; // Mapped buffer parameters

    Image::Encoding source_encoding ;   // Encoding of images grabbed

    unsigned long _pace ;
    TimeKeeper *_tk ;

    std::string _vsSignature ;
    UdpReceiver *_vsSignal ;       // Used to request video hardware
    // from current nucleo user
    int _post_resize ;
    int _rwidth, _rheight ;
    int _errors ;

    enum { OPENED, WAITING_FOR_IMAGE, READY, CLOSED } _iState ;

  protected:

    unsigned char *_data ;
    int _transferSize ;
    int _width, _height ;

    void react(Observable*) ;

  public:

    v4lImageSource(const URI &uri, Image::Encoding encoding) ;
    ~v4lImageSource(void) ;

    ImageSource::state getState(void) { return (_iState==CLOSED) ? STOPPED : STARTED ; }

    bool start(void) ;
    bool getNextImage(Image *img, TimeStamp::inttype reftime) ;
    bool stop(void) ;

  } ;

}

#endif
