#ifndef n_TcpUtils_H
#define n_TcpUtils_H

/*
 *
 * nucleo/network/tcp/TcpUtils.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <string>

namespace nucleo {

  void setDefaultTcpSocketOptions(int socket, bool server) ;

  std::string getRemoteTcpHost(int socket, int *port=0) ;

}

#endif
