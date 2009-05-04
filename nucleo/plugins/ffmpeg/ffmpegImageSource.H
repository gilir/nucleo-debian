#ifndef n_ffmpegImageSource_H_
#define n_ffmpegImageSource_H_

/*
 *
 * nucleo/plugins/ffmpeg/ffmpegImageSource.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/source/ImageSource.H>
#include <nucleo/core/TimeKeeper.H>

#include <stdint.h>

extern "C" {
#include <libavformat/avformat.h>
}

namespace nucleo {

  class ffmpegImageSource : public ImageSource {

  protected:

    std::string filename ;

    AVFormatContext *fctx ;
    long time_base ;
    int video_stream ;
    AVCodecContext *cctx ;
    TimeKeeper *timer ;

    void react(Observable *) ;

  public:

    ffmpegImageSource(const URI &uri, Image::Encoding e) ;
    virtual ~ffmpegImageSource(void) { stop() ; }

    ImageSource::state getState(void) { return (timer?STARTED:STOPPED) ; }

    bool start(void) ;
    bool getNextImage(Image *img, TimeStamp::inttype reftime) ;
    bool stop(void) ;

  } ;

}

#endif
