/*
 *
 * test/test-HttpMessage.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/http/HttpMessage.H>

#include <iostream>
#include <string>

using namespace nucleo ;

#define ADD_CONTENT_LENGTH 1

void
test1(std::string &data) {
  //         012345678901234
  data = "HTTP/1.0 200 OK" ;
  //               5   6
  data.append("\015\012") ;
  //           78901234567890123456789
  data.append("Content-Type: text/html") ;
  data.append("\015\012") ;
  data.append("Content-Length: 22") ;
  data.append("\015\012") ;
  data.append("\015\012") ;
  data.append("Il fait beau chez toto") ;
  data.append("HTTP/1.0 200 OK again") ;
  data.append("\015\012") ;
  data.append("Content-Type: text/html") ;
  data.append("\015\012") ;
  data.append("\015\012") ;
  data.append("C'est cool !") ;
}

void
test2(std::string &data) {
  data = "HTTP/1.0 200 OK" ;
  data.append("\015\012") ;
  data.append("Expires: Thu, 01 Dec 1994 16:00:00 GMT") ;
  data.append("\015\012") ;
  data.append("Cache-Control: no-cache") ;
  data.append("\015\012") ;
  data.append("Pragma: no-cache") ;
  data.append("\015\012") ;
  data.append("Content-type: multipart/x-mixed-replace;boundary=-->") ;
  data.append("\015\012") ;
  data.append("Connexion: keep-alive") ;
  data.append("\015\012") ;

  data.append("\015\012") ;
  data.append("-->") ;
  data.append("\015\012") ;
  data.append("Server: multipartEncoder/1.6") ;
  data.append("\015\012") ;
  data.append("Content-Type: text/html") ;
  data.append("\015\012") ;
#if ADD_CONTENT_LENGTH
  data.append("Content-Length: 6") ;
  data.append("\015\012") ;
#endif
  data.append("\015\012") ;
  data.append("coucou") ;

  data.append("\015\012") ;
  data.append("-->") ;
  data.append("\015\012") ;
  data.append("Server: multipartEncoder/1.6") ;
  data.append("\015\012") ;
  data.append("Content-Type: text/html") ;
  data.append("\015\012") ;
#if ADD_CONTENT_LENGTH
  data.append("Content-Length: 7") ;
  data.append("\015\012") ;
#endif
  data.append("\015\012") ;
  data.append("bonjour") ;

  data.append("\015\012") ;
  data.append("-->") ;
  data.append("\015\012") ;
  data.append("Server: multipartEncoder/1.6") ;
  data.append("\015\012") ;
  data.append("Content-Type: text/html") ;
  data.append("\015\012") ;
#if ADD_CONTENT_LENGTH
  data.append("Content-Length: 5") ;
  data.append("\015\012") ;
#endif
  data.append("\015\012") ;
  data.append("salut") ;
}

int
main(void) {
  std::string response ;
  test2(response) ;

  HttpMessage msg ;
  msg.feed(response) ;

  for (;;)
    if (msg.parseData()!=HttpMessage::COMPLETE) {
	 std::cout << "Bad message !" << std::endl ;
	 break ;
    } else {
	 msg.debug(std::cout, 1) ;
	 msg.next() ;
    }

  std::cout << std::endl ;
}
