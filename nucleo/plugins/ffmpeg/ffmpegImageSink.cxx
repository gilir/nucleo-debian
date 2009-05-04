/*
 *
 * nucleo/plugins/ffmpeg/ffmpegImageSink.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>
#include <nucleo/utils/ByteOrder.H>
#include <nucleo/plugins/ffmpeg/ffmpegImageSink.H>
#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/image/processing/basic/Resize.H>

#include <stdexcept>
#include <iomanip>

#include <cstring>

#define NO_SOFTWARE_SCALER 1

// ------------------------------------------------------------------------------------

extern "C" {
  void *
  ffmpegImageSink_factory(const nucleo::URI *uri) {
    return new nucleo::ffmpegImageSink(*uri) ;
  }
}

// ------------------------------------------------------------------------------------

namespace nucleo {

#define MPEGTS_PACKET_SIZE 188
#define MPEGTS_SYNC 0x47
#define MPEGTS_NULLPACKET_PID 0x1FFF
#define MPEGTS_TRANSPORT_LOAD 7
#define MPEGTS_TRANSPORT_SIZE MPEGTS_PACKET_SIZE*MPEGTS_TRANSPORT_LOAD

#define FREEPLAYER_VIDEO_PID 0x0044
#define FREEPLAYER_AUDIO_PID 0x0045

  int
  ffmpegImageSink::udp_callback(void *opaque, uint8_t *buffer, int size) {
    ffmpegImageSink *sink = (ffmpegImageSink*)opaque ;

    int nbPackets = size/MPEGTS_PACKET_SIZE ;
    int padding = MPEGTS_TRANSPORT_LOAD - nbPackets ;
    uint8_t *packet ;

#if 0
    std::cerr << nbPackets << " packets:" ;
    packet = buffer ;
    for (int i=0; i<nbPackets; ++i) {
	 unsigned short pid = ((packet[1] & 0x1f) << 8) + packet[2] ;
	 std::cerr << " 0x" << std::hex << std::setfill('0') << std::setw(4) << pid << std::dec  ;
#if 0
	 if (pid==0x0100) {
	   packet[1] = (packet[1]&0xe0) | (FREEPLAYER_VIDEO_PID >> 8) ;
	   packet[2] = (FREEPLAYER_VIDEO_PID & 0xff) ;
	 }
#endif
	 packet += MPEGTS_PACKET_SIZE ;
    }
    std::cerr << std::endl ;
#endif

    if (size==MPEGTS_TRANSPORT_SIZE)
	 return (sink->sender->send(buffer,size)==size) ? 0 : -1 ;

    // Warning: this works only if buffer is the one we passed to
    // init_put_byte (i.e. it has enough room for holding
    // MPEGTS_TRANSPORT_SIZE bytes)

    packet = buffer + size ;
    for (int i=0; i<padding; ++i) {
	 packet[0] = MPEGTS_SYNC ;
	 packet[1] = (MPEGTS_NULLPACKET_PID >> 8) ;
	 packet[2] = (MPEGTS_NULLPACKET_PID & 0xff) ;
	 packet += MPEGTS_PACKET_SIZE ;
    }
    return (sink->sender->send(buffer,MPEGTS_TRANSPORT_SIZE)==MPEGTS_TRANSPORT_SIZE) ? 0 : -1 ;
  }

  // ------------------------------------------------------------------------------------

  ffmpegImageSink::ffmpegImageSink(const URI &u) {
    av_register_all();

    filename = "" ;
    output_format = 0 ;
    format_context = 0 ;
    vstream = 0 ;
    video_outbuf = 0 ;
    picture = &srcPic ;
    sender = 0 ;

    uri = u ;
  }
       
  bool
  ffmpegImageSink::start(void) {
    if (output_format) return false ;

    std::string query = uri.query ;
    std::string format = "mp4" ;
    bool guessFormat = !URI::getQueryArg(query, "format", &format) ;

    output_format = 0 ;
    if (uri.scheme=="mpegts-udp") {
	 output_format = guess_format("mpegts", NULL, NULL) ;
    } else {
	 filename = uri.opaque!="" ? uri.opaque : uri.path ;
	 if (guessFormat) output_format = guess_format(NULL, filename.c_str(), NULL) ;
	 if (!output_format) output_format = guess_format(format.c_str(), NULL, NULL) ;
    } 

    std::string message ;
    if (!output_format)
	 message = "Unable to find the requested format" ;
    else if (output_format->video_codec == CODEC_ID_NONE) {
	 output_format = 0 ;
	 message = "Not a video format" ;
    } else if (output_format->flags & AVFMT_NOFILE) {
	 output_format = 0 ;
	 message = "AVFMT_NOFILE unsupported" ;
    } else if (output_format->flags & AVFMT_RAWPICTURE) {
	 output_format = 0 ;
	 message = "AVFMT_RAWPICTURE unsupported" ;
    }

    if (!output_format) {
	 std::cerr << "ffmpegImageSink: " << message << std::endl ;
	 return false ;
    }

    // ----------------

    format_context = av_alloc_format_context();
    if (!format_context) {
	 std::cerr << "ffmpegImageSink: unable to create the AVFormatContext" << std::endl ;
	 stop() ;
	 return false ;
    }
    format_context->oformat = output_format;
    snprintf(format_context->filename, sizeof(format_context->filename), 
		   "%s", filename.c_str()) ;
 
    vstream = av_new_stream(format_context, FREEPLAYER_VIDEO_PID) ;
    if (!vstream) {
	 std::cerr << "ffmpegImageSink: unable to create the AVStream" << std::endl ;
	 stop() ;
	 return false ;
    }

    // ----------------

    frameCount = 0 ; sampler.start() ;
    return true ;
  }

  // ------------------------------------------------------------------------------------

  bool
  ffmpegImageSink::init(Image *img) {
    unsigned int framerate = 25 ;
    unsigned int bitrate = 6000 ;
    unsigned int tolerance = 4000 ;
    unsigned int gopsize = 8 ;
    unsigned int qmin = 2 ;
    unsigned int qmax = 31 ;

    std::string query = uri.query ;
    URI::getQueryArg(query, "framerate", &framerate) ;
    URI::getQueryArg(query, "bitrate", &bitrate) ;
    URI::getQueryArg(query, "tolerance", &tolerance) ;
    URI::getQueryArg(query, "gopsize", &gopsize) ;
    URI::getQueryArg(query, "qmin", &qmin) ;
    URI::getQueryArg(query, "qmax", &qmax) ;

    AVCodecContext *codec_context = vstream->codec ;
    codec_context->codec_id = output_format->video_codec ;
    codec_context->codec_type = CODEC_TYPE_VIDEO ;
    codec_context->pix_fmt = PIX_FMT_YUV420P ;
    codec_context->width = img->getWidth() ;  // must be a multiple of two
    codec_context->height = img->getHeight() ; // must be a multiple of two
    codec_context->me_method = ME_EPZS ; // ME_ZERO
    // ---
    codec_context->bit_rate = bitrate*1000 ;
    codec_context->bit_rate_tolerance = tolerance*1000 ;
    codec_context->time_base.den = framerate ; // frames per second
    codec_context->time_base.num = 1 ;
    codec_context->gop_size = gopsize ; // at most one intra frame every gop_size frames
    codec_context->qmin = qmin ;
    codec_context->qmax = qmax ;
    // ---
    // some formats want stream headers to be seperate
    if (!strcmp(format_context->oformat->name, "mp4")
	   || !strcmp(format_context->oformat->name, "mov")
	   || !strcmp(format_context->oformat->name, "3gp"))
	 codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER ;

    if (av_set_parameters(format_context, NULL) < 0) {
	 std::cerr << "ffmpegImageSink: invalid output format parameters" << std::endl ;
	 return false ;
    }

    dump_format(format_context, 0, filename.c_str(), 1) ; // FIXME: for debug only

    // ------------------

    AVCodec *codec = avcodec_find_encoder(codec_context->codec_id);
    if (!codec) {
	 std::cerr << "ffmpegImageSink: codec not found" << std::endl ;
	 return false ;
    }

    if (avcodec_open(codec_context, codec) < 0) {
	 std::cerr << "ffmpegImageSink: could not open codec" << std::endl ;
	 vstream->codec = 0 ;
	 return false ;
    }

    // ------------------

    if (filename!="") {
	 sender = 0 ;
	 if (url_fopen(&format_context->pb, filename.c_str(), URL_WRONLY) < 0) {
	   std::cerr << "ffmpegImageSink: could not open " << filename << std::endl ;
	   return false ;
	 }
    } else {
	 unsigned int buffer_size = MPEGTS_PACKET_SIZE*MPEGTS_TRANSPORT_LOAD ;
	 uint8_t *buffer = new uint8_t [buffer_size] ;
	 int port = uri.port ;
	 if (!port) port = 1234 ;
	 sender = new UdpSender(uri.host.c_str(), port) ;
	 if (init_put_byte(format_context->pb, buffer, buffer_size,
				    1, (void *)this,
				    0, udp_callback, 0) < 0) {
	   std::cerr << "ffmpegImageSink: init_put_byte failed" << std::endl ;
	   return false ;
	 }
	 format_context->pb->is_streamed = 1 ; // no seek
	 format_context->pb->max_packet_size = buffer_size ;
    }
    
    av_write_header(format_context) ;

    video_outbuf_size = 256*1024 ; // FIXME ?
    video_outbuf = new uint8_t [video_outbuf_size] ;

    switch (img->getEncoding()) {
    case Image::ARGB: 
	 // PIX_FMT_RGBA32 is endian-sensitive...
	 srcEncoding = ByteOrder::isLittleEndian() ? PIX_FMT_RGB24 : PIX_FMT_RGBA32 ; 
	 break ; 
    case Image::L: srcEncoding = PIX_FMT_GRAY8 ; break ;
    case Image::YpCbCr420: srcEncoding = PIX_FMT_YUV420P ; break ;
    default: srcEncoding = PIX_FMT_RGB24 ; break ;
    }

    avcodec_get_frame_defaults(&srcPic) ;
    if (codec_context->pix_fmt==srcEncoding)
	 picture = &srcPic ;
    else {
	 avcodec_get_frame_defaults(&convPic) ;
	 int size = avpicture_get_size(codec_context->pix_fmt, codec_context->width, codec_context->height) ;
	 avpicture_fill((AVPicture *)&convPic,
				 new uint8_t [size],
				 codec_context->pix_fmt, codec_context->width, codec_context->height) ;
	 picture = &convPic ;
    }

    return true ;
  }

  // ------------------------------------------------------------------------------------

  bool
  ffmpegImageSink::handle(Image *img) {
    if (!output_format) return false ;

    if (!frameCount && !init(img)) {
	 stop() ;
	 return false ;
    }

    int64_t pts = av_rescale_rnd(sampler.read(), vstream->time_base.den, 1000*vstream->time_base.num, AV_ROUND_NEAR_INF) ;

    AVCodecContext *codec_context = vstream->codec ;
    if (srcEncoding==PIX_FMT_RGB24) convertImage(img, Image::RGB) ;
    resizeImage(img, codec_context->width, codec_context->height) ;
    avpicture_fill((AVPicture *)&srcPic, img->getData(), srcEncoding,
			    codec_context->width, codec_context->height) ;
#if NO_SOFTWARE_SCALER
    if (codec_context->pix_fmt != srcEncoding)
	 img_convert((AVPicture *)&convPic, codec_context->pix_fmt, 
			   (AVPicture *)&srcPic, srcEncoding, codec_context->width, codec_context->height);
#else
#pragma Núcleo doesn't support the FFmpeg software scaler (yet). Sorry...
#endif
    picture->pts = pts ;

    int out_size = avcodec_encode_video(codec_context, video_outbuf, video_outbuf_size, picture) ;
    if (!out_size) {
	 std::cerr << "ffmpegImageSink: avcodec_encode_video returned 0" << std::endl ;
	 stop() ;
	 return false ;
    }

    // std::cerr << "out_size = " << out_size << std::endl ;

    AVPacket pkt ;
    av_init_packet(&pkt) ;
    pkt.stream_index = vstream->index ;
    pkt.data = video_outbuf ;
    pkt.size = out_size ;
    if (codec_context->coded_frame) {
	 pkt.pts = pkt.dts = pts ;
	 if (codec_context->coded_frame->key_frame) pkt.flags |= PKT_FLAG_KEY ;
    }
    if (av_write_frame(format_context, &pkt) != 0) {
	 std::cerr << "ffmpegImageSink: error while writing video frame" << std::endl ;
	 stop() ;
	 return false ;
    }

    // std::cerr << "ffmpegImageSink::handle, line " << __LINE__ << std::endl ;

    frameCount++ ; sampler.tick() ;

    return true ;
  }

  // ------------------------------------------------------------------------------------

  bool
  ffmpegImageSink::stop(void) {
    if (!output_format) return false ;

    if (vstream) {
	 if (picture==&convPic) delete [] convPic.data[0] ;
	 if (vstream->codec) {
	   avcodec_close(vstream->codec) ;
	   av_write_trailer(format_context) ;
	   if (!sender) url_fclose(format_context->pb) ;
	 }
	 av_freep(&vstream) ;
    }

    if (sender) delete sender ;

    if (format_context) av_free(format_context) ;

    if (video_outbuf) delete [] video_outbuf ;

    filename = "" ;
    output_format = 0 ;
    format_context = 0 ;
    vstream = 0 ;
    video_outbuf = 0 ;
    picture = &srcPic ;
    sender = 0 ;

    sampler.stop() ;

    return true ;
  }

}
