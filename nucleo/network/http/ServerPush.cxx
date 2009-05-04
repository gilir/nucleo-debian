/*
 *
 * nucleo/network/http/ServerPush.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/http/ServerPush.H>

#include <sys/uio.h>
#include <sys/errno.h>

#include <unistd.h>
#include <cstdio>

#include <sstream>
#include <stdexcept>

namespace nucleo {

  // ------------------------------------------------------------------

  ServerPush::ServerPush(int fd) {
	 _fd = fd ;

	 std::stringstream rs ;
	 rs << "HTTP/1.0 200 OK" << oneCRLF
	    << "Cache-Control: no-cache" << oneCRLF
	    << "Pragma: no-cache" << oneCRLF
	    << "Content-type: multipart/x-mixed-replace;boundary=-nUcLeO->" << oneCRLF
	    << "Connexion: keep-alive" << oneCRLF ;
	 std::string response = rs.str() ;

	 const char *data = response.c_str() ;
	 int size = response.size() ;
	 if (write(_fd, data, size)!=size)
	   throw std::runtime_error("ServerPush: write failed") ;
    }

    void
    ServerPush::push(const char *content_type, char *content, int content_length,
	    const char *headers) {
	 std::stringstream hs ;
	 if (headers) hs << headers << oneCRLF ;
	 hs << "Content-Type: " << content_type << oneCRLF
	    << "Content-Length: " << content_length << oneCRLF
	    << oneCRLF ;
	 std::string sheaders = hs.str() ;

	 struct iovec iov[3] ;   
	 iov[0].iov_base = (char *)"\r\n-nUcLeO->\r\n" ;
	 iov[0].iov_len = 13 ;
	 iov[1].iov_base = (char *)sheaders.c_str() ;
	 iov[1].iov_len = sheaders.size() ;
	 iov[2].iov_base = content ;
	 iov[2].iov_len = content_length ;

	 int size = iov[0].iov_len + iov[1].iov_len + iov[2].iov_len ;
	 if (writev(_fd, iov, 3)!=size)
	   throw std::runtime_error("ServerPush::push: writev failed") ;
    }

  // ------------------------------------------------------------------

}
