/*
 *
 * apps/videoServer/FileStreamer.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef FileStreamer_H
#define FileStreamer_H

#include "Streamer.H"
#include "VideoService.H"

using namespace nucleo ;

class FileStreamer : public Streamer {

protected:

  int fd ;

  void react(Observable *) ;

public:

  FileStreamer(TcpConnection *connection, VideoService *service) ;
  ~FileStreamer(void) ;

} ;

#endif
