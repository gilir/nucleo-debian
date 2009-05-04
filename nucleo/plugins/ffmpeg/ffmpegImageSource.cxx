/*
 *
 * nucleo/plugins/ffmpeg/ffmpegImageSource.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include "ffmpegImageSource.H"

#include <nucleo/image/encoding/Conversion.H>

#define NO_SOFTWARE_SCALER 1

#if !NO_SOFTWARE_SCALER
extern "C" {
#include <libswscale/swscale.h>
}
#endif

// -------------------------------------------------------------------------------------------

extern "C" {
  void *ffmpegImageSource_factory(const nucleo::URI *uri, nucleo::Image::Encoding e) {
    return (void *)(new nucleo::ffmpegImageSource(*uri, e)) ;
  }
}

// -------------------------------------------------------------------------------------------

namespace nucleo {

  ffmpegImageSource::ffmpegImageSource(const URI &uri, Image::Encoding e) {
    av_register_all() ;

    filename = uri.opaque!="" ? uri.opaque : uri.path ;
    target_encoding = e ;

    timer = 0 ;
  }

  bool
  ffmpegImageSource::start(void) {
    int ret = av_open_input_file(&fctx, filename.c_str(), 0, 0, 0);
    if (ret<0) {
	 std::cerr << "ffmpegImageSource: failed to open " << filename << " (" << ret << ")" << std::endl ;
	 return false ;
    }

    ret = av_find_stream_info(fctx) ;
    if (ret<0) {
	 std::cerr << "ffmpegImageSource: failed to find codec parameters for " << filename << " (" << ret << ")" << std::endl ;
	 return false ;
    }

    dump_format(fctx, 1, filename.c_str(), 0) ;

    bool found_video_stream = false ;
    for (unsigned int i=0; i<fctx->nb_streams; ++i) {
	 if (fctx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
	   video_stream = i ;
	   found_video_stream = true ;
	 } else
	   fctx->streams[i]->discard = AVDISCARD_ALL ; // disable this track
    }

    if (!found_video_stream) {
	 std::cerr << "ffmpegImageSource: no video stream" << std::endl ;
	 return false ;
    }
  
    // --------------------------------------------------

    cctx = fctx->streams[video_stream]->codec ;
    time_base = TimeKeeper::second*fctx->streams[video_stream]->r_frame_rate.den/fctx->streams[video_stream]->r_frame_rate.num ;

    AVCodec *codec = avcodec_find_decoder(cctx->codec_id) ;
    if (!codec) {
	 std::cerr << "ffmpegImageSource: codec not found" << std::endl ;
	 return false ;
    }

    ret = avcodec_open(cctx, codec);
    if (ret<0) {
	 std::cerr << "ffmpegImageSource: unable to open codec" << std::endl ;
	 return false ;
    }

    // --------------------------------------------------

    timer = TimeKeeper::create(10*TimeKeeper::millisecond) ;
    subscribeTo(timer) ;

    frameCount = 0 ; previousImageTime = TimeStamp::undef ; sampler.start() ;

    return true ;
  }

  void
  ffmpegImageSource::react(Observable *obs) {
    if (!timer) return ;

    AVPacket packet ;
    AVFrame frame ;

    while (av_read_frame(fctx, &packet) == 0) {
	 Chronometer elapsed ;

	 if (packet.stream_index!=video_stream
		|| packet.dts==AV_NOPTS_VALUE) {
	   av_free_packet(&packet) ;
	   continue ;
	 }

#if 0
	 double pts = time_base*packet.dts;
	 std::cerr << "time_base: " << time_base << " dts: " << packet.dts << " pts: " << pts << std::endl ;
#endif

	 int got_picture = 0 ;
	 avcodec_decode_video(cctx, &frame, &got_picture, packet.data, packet.size) ;
	 av_free_packet(&packet) ;
	 if (!got_picture) continue ;

	 lastImage.setTimeStamp() ;
	 switch (cctx->pix_fmt) {
	 case PIX_FMT_GRAY8:
	   lastImage.setDims(cctx->width, cctx->height) ;
	   lastImage.setEncoding(Image::L) ;
	   lastImage.setData(frame.data[0], cctx->width*cctx->height, Image::NONE) ;
	   break ;
	 case PIX_FMT_RGB24:
	   lastImage.setDims(cctx->width, cctx->height) ;
	   lastImage.setEncoding(Image::RGB) ;
	   lastImage.setData(frame.data[0], (cctx->width*cctx->height)*3, Image::NONE) ;
	   break ;
	 default: // Convert everything else to RGB24
	   // PIX_FMT_YUV420P has three separate plans and ffmpeg converts
	   // them to RGB faster than nucleo does...
	   // PIX_FMT_RGBA32 and PIX_FMT_RGB565 are stored in cpu endianness
	   // On OS X (powerpc), they correspond to Image::ARGB and Image::RGB565
	   lastImage.prepareFor(cctx->width, cctx->height, Image::RGB) ;
	   AVFrame tmpframe ;
	   avpicture_fill((AVPicture*)&tmpframe, lastImage.getData(), PIX_FMT_RGB24,
				   cctx->width, cctx->height) ;
#if NO_SOFTWARE_SCALER
	   img_convert((AVPicture*)&tmpframe, PIX_FMT_RGB24,
				(AVPicture*)&frame, cctx->pix_fmt, cctx->width, cctx->height) ;
#else
#pragma Núcleo doesn't support the FFmpeg software scaler (yet). Sorry...
/*
	   SwsContext *swsc = 0 ;
	   swsc = sws_getCachedContext(swsc,
							 cctx->width, cctx->height, cctx->pix_fmt, 
							 cctx->width, cctx->height, PIX_FMT_RGB24,
							 SWS_PARAM_DEFAULT, NULL, NULL, NULL) ;
	   if (!swsc) {
		std::cerr << "PERDU!" << std::endl ;
	   } else 
		sws_scale(swsc, 
				frame.data, frame.linesize, 0, cctx->height,
				tmpframe.data, tmpframe.linesize) ;
*/
#endif
	   break ;
	 }
	 
	 frameCount++ ; sampler.tick() ;
#if 0
	 long delta = time_base - elapsed.read()*TimeKeeper::millisecond ;
	 if (delta<0) delta = 0 ;
	 // std::cerr << "time_base: " << time_base << " delta: " << delta << std::endl ;
	 timer->arm(delta*TimeKeeper::millisecond) ;
#else
	 timer->arm(time_base*TimeKeeper::millisecond) ;
#endif
	 if (!_pendingNotifications) notifyObservers() ;	 
	 break ;
    }

  }

  bool
  ffmpegImageSource::getNextImage(Image *img, TimeStamp::inttype reftime) {
    if (!timer || !frameCount || reftime>=lastImage.getTimeStamp()) return false ;
    previousImageTime = lastImage.getTimeStamp() ;
    bool ok = convertImage(&lastImage, target_encoding) ;
    if (ok) img->linkDataFrom(lastImage) ;
    return ok ;
  }

  bool
  ffmpegImageSource::stop(void) {
    if (!timer) return false ;

    unsubscribeFrom(timer) ;
    delete timer ;
    timer = 0 ;
    sampler.stop() ;

    return true ;
  }

}
