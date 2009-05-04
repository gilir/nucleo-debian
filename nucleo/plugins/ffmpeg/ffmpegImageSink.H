#ifndef n_ffmpegImageSink_H_
#define n_ffmpegImageSink_H_

/*
 *
 * nucleo/plugins/ffmpeg/ffmpegImageSink.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/sink/ImageSink.H>
#include <nucleo/core/URI.H>
#include <nucleo/network/udp/UdpSender.H>

#include <stdint.h>

extern "C" {
#include <libavformat/avformat.h>
}

namespace nucleo {

  class ffmpegImageSink : public ImageSink {

  private:

    URI uri ;

    AVOutputFormat *output_format ;
    AVFormatContext *format_context ;
    AVStream *vstream ;
    AVFrame srcPic, convPic, *picture ;
    int video_outbuf_size ;
    uint8_t *video_outbuf ;
    int srcEncoding ;

    std::string filename ;   // if used as a file sink

    UdpSender *sender ;      // if used as an mpegts-udp sink
    static int udp_callback(void *opaque, uint8_t *buffer, int size) ;

    bool init(Image *img) ;

  public:

    ffmpegImageSink(const URI &uri) ;
    ~ffmpegImageSink() { stop() ; }

    ImageSink::state getState(void) {
	 return (output_format!=0?STARTED:STOPPED) ;
    }

    bool start(void) ;
    bool handle(Image *img) ;
    bool stop(void) ;

  } ;

}

#endif
