/*
 *
 * tests/test-SocketKeeper.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/FileKeeper.H>
#include <nucleo/network/tcp/TcpConnection.H>
#include <nucleo/utils/FileUtils.H>
#include <nucleo/utils/TimeUtils.H>

#include <unistd.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

using namespace nucleo ;

class Reactor : public ReactiveObject {

protected:

  TcpConnection *connection ;
  int totalbytes ;
  Chronometer chrono ;

  void react(Observable*) {
    chrono.tick() ;
    int fd = connection->getFd() ;
#if 1
    int size = getavail(fd) ;
    char *buffer = new char [size] ;
    int bytesread = read(fd, buffer, size) ;
    delete [] buffer ;
    totalbytes += bytesread ;
    std::cerr << size << " - " << bytesread << " - " << totalbytes << std::endl ;
    if (bytesread<1) {
	 std::cerr << chrono.average() << " ticks/sec" << std::endl ;
	 ReactiveEngine::stop() ;
    }
#else
    if (chrono.read()>10000) {
	 std::cerr << chrono.average() << " ticks/sec" << std::endl ;
	 ReactiveEngine::stop() ;
    }
#endif
  }

public:

  Reactor(TcpConnection *conn, const char *request) {
    connection = conn ;
    std::stringstream srequest ;
    srequest << "GET " << request << " HTTP/1.0\n\n" ;
    std::string s = srequest.str() ;
    write(connection->getFd(), s.c_str(), s.size()) ;

    totalbytes = 0 ;
    subscribeTo(connection) ;

    chrono.start() ;
  }

} ;

int
main(int argc, char **argv) {
  try {
    char *ENGINE = getenv("NUCLEO_ENGINE") ;
    ReactiveEngine::setEngineType(ENGINE?ENGINE:"default") ;

   char *HOST="localhost" ;
   int PORT=80 ;
   char *REQUEST="/" ;

   int test = (argc>1) ? atoi(argv[1]) : 0 ;

   switch (test) {
   case 0: HOST = "localhost" ; REQUEST = "/" ; break ;
   case 1: HOST = "127.0.0.1" ; REQUEST = "/~roussel/publications/dea-roussel.pdf" ; break ;
   case 2: HOST = "insitu.lri.fr" ; REQUEST = "/~roussel/publications/these-roussel.pdf" ; break ;
   case 3: HOST="lie.u-bourgogne.fr" ; PORT = 5555 ; REQUEST = "/push/video" ; break ;
   }
   
   std::cerr << "http://" << HOST << ":" << PORT << REQUEST << std::endl ;

   TcpConnection c(HOST, PORT) ;
   Reactor r(&c, REQUEST) ;

   ReactiveEngine::run() ;

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }

  return 0 ;
}
