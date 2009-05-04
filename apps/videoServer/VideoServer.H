/*
 *
 * apps/videoServer/VideoServer.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef VideoServer_H
#define VideoServer_H

#include <nucleo/core/ReactiveObject.H>

#include <nucleo/utils/ConfigDict.H>
#include <nucleo/network/tcp/TcpServer.H>
#include <nucleo/network/dnssd/DNSServiceAnnouncer.H>

using namespace nucleo ;

class VideoServer : public ReactiveObject {

protected:

  ConfigDict config ;
  TcpServer *server ;
  DNSServiceAnnouncer *announcer ;

  void react(Observable *obs) ;

public:

  VideoServer(char *configfile) ;
  ~VideoServer(void) ;

} ;

#endif
