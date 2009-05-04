#ifndef n_ServerPush_H_
#define n_ServerPush_H_

/*
 *
 * nucleo/network/http/ServerPush.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/http/HttpMessage.H>

namespace nucleo {

  // ------------------------------------------------------------------

  class ServerPush {

  private:

    int _fd ;

  public:

    ServerPush(int fd) ;

    void
    push(const char *content_type, char *content, int content_length,
	    const char *headers=0) ;

  } ;

  // ------------------------------------------------------------------

}

#endif
