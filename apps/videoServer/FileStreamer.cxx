/*
 *
 * apps/videoServer/FileStreamer.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "FileStreamer.H"

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/utils/FileUtils.H>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdexcept>

FileStreamer::FileStreamer(TcpConnection *c, VideoService *s) {
  connection = c ;

  fd = open(s->arg.c_str(), O_RDONLY) ;
  if (fd==-1) {
    std::string msg = "FileStreamer: unable to open " ;
    msg = msg + s->arg ;
    throw std::runtime_error(msg) ;
  }

  delete s ;
  selfNotify() ;
}

void
FileStreamer::react(Observable *obs) {
  char buffer[10240] ;
  ssize_t length = read(fd, buffer, 10240) ;
  if (length>0) {
    connection->send(buffer, length, true) ;
    selfNotify() ;
  } else {
    delete this ;
  }
}

FileStreamer::~FileStreamer(void) {
  close(fd) ;
  delete connection ;
}
