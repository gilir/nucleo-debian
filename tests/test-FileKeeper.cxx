/*
 *
 * tests/test-FileKeeper.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/FileKeeper.H>
#include <nucleo/utils/FileUtils.H>

#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace nucleo ;

class Reactor : public ReactiveObject {

protected:

  int _fd ;
  FileKeeper *_readable, *_writable ;
  int _total ;

  void react(Observable *obs) {
    if (obs==_readable) {
	 // std::cerr << "r" << std::endl ;
	 if (! (_readable->getState()&FileKeeper::R)) std::cerr << "not readable!" << std::endl ;
	 int size = getavail(_fd) ;
	 char *buffer = new char [size] ;
	 int bytesread = read(_fd, buffer, size) ;
	 delete [] buffer ;
	 _total += bytesread ;
	 std::cerr << size << " - " << bytesread << " - " << _total << std::endl ;
	 if (bytesread<1) ReactiveEngine::stop() ;
    } else if (obs==_writable) {
	 // std::cerr << "w" << std::endl ;
	 if (! (_writable->getState()&FileKeeper::W)) std::cerr << "not writable!" << std::endl ;
    }
  }

public:

  Reactor(int fd) {
    _total = 0 ;
    _fd = fd ;
    _readable = FileKeeper::create(fd, FileKeeper::R) ; 
    subscribeTo(_readable) ;
    _writable = FileKeeper::create(fd, FileKeeper::W) ; 
    subscribeTo(_writable) ;
  }

} ;

int
main(int argc, char **argv) {
  char *ENGINE = getenv("NUCLEO_ENGINE") ;
  ReactiveEngine::setEngineType(ENGINE?ENGINE:"default") ;

#if 0
  int fd = open("/tmp/test-FileKeeper.tmp", O_RDWR) ;
  Reactor r(fd) ;
#else
  Reactor r(0) ;
#endif

  ReactiveEngine::run() ;

  return 0 ;
}
