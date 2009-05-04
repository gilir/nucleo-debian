/*
 *
 * apps/videoServer/VideoStreamer.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef VideoStreamer_H
#define VideoStreamer_H

#include "Streamer.H"
#include "VideoService.H"

#include <nucleo/network/tcp/TcpConnection.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/sink/ImageSink.H>

using namespace nucleo ;

class VideoStreamer : public Streamer {

protected:

  VideoService *service ;

  ImageSource *source ;
  Image img ;
  ImageSink *sink ;

  void react(Observable *) ;

public:

  VideoStreamer(TcpConnection *connection, VideoService *service) ;
  ~VideoStreamer(void) ;

} ;

#endif
