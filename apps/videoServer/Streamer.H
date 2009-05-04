/*
 *
 * apps/videoServer/Streamer.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef Streamer_H
#define Streamer_H

#include <nucleo/core/ReactiveObject.H>
#include <nucleo/network/tcp/TcpConnection.H>

using namespace nucleo ;

class Streamer : public ReactiveObject {
protected:
  TcpConnection *connection ;
public:
} ;

#endif
