/*
 *
 * tests/test-URI.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/URI.H>

#include <iostream>
#include <string>

using namespace nucleo ;

int
main(void) {
  const char *tests[] = {
    "roussel@localhost/circa",
    "xmpp:roussel@localhost/circa",
    "xmpp://roussel@localhost/circa",
    "http://user:password@host:1212/path1/path2?query1=query2#frag",
    "/first/second?user=nicolas+roussel&host=sgi3&time=now",
    "//user@host/toto/tutu",
    "/pub/python",
    "index.html",
    "/",
    "?coucou=toto",
    "http://:8001",
    "mailto:user@host.domain",
    "mcast://225.0.0.250:8123",
    "mcast://<broadcast>:8080",
    "http://host1.lri.fr:3128/http://host2.lri.fr:5555/video?zoom=4",
    0
  } ;

  URI *uri ;
  int i ;
  for (i=0; tests[i]; ++i) {
    std::string original = tests[i] ;
    uri = new URI(original) ;
    std::cout << std::endl << "-------------------------------------------------" << std::endl ;
    std::string processed = uri->asString() ;
    std::cout << original << std::endl
		    << processed << std::endl << std::endl ;
    if (original!=processed) std::cerr << "FAILED!" << std::endl ;
    
    uri->debug(std::cout) ;
    delete uri ;
  }

  std::cout << std::endl << "-------------------------------------------------" ;
  std::cout << std::endl << "-------------------------------------------------" << std::endl ;

  const char *testsQuery[] = {
    "findme=30.0&zoom=2&length=50000",
    "zoom=2&findme=30&length=50000",
    "zoom=2&length=50000&findme=nicolas",
    "findme&zoom=2&length=50000",
    "zoom=2&notfindme&findme",
    "findmenot=2&length=50000&findme",
    "qslkdfjhsd",
    "findme&toto",
    "toto&findme",
    0
  } ;

  for (i=0; testsQuery[i]; ++i) {
    std::cerr << "query: " << testsQuery[i] << " --> " ;
    std::string arg ;
    if (URI::getQueryArg(testsQuery[i], "findme", &arg))
	 std::cout << "findme='" << arg << "'" << std::endl ;
    else
	 std::cout << "findme not found" << std::endl ;
  }

}
