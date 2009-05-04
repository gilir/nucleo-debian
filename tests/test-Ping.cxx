/*
 *
 * tests/test-Ping/ping.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/AppUtils.H>
#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/TimeStamp.H>
#include <nucleo/network/tcp/Ping.H>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

using namespace nucleo ;

class Tester : public ReactiveObject {

protected:

  Ping *ping ;

  void react(Observable *obs) {
    if (obs!=ping) return ;
    std::cerr << TimeStamp::createAsString() << ": " << ping->getState()*100 << "% reachable" << std::endl ;
  }

public:

  Tester(unsigned long ms, char **uris, int nburis) {
    ping = new Ping(ms) ;
    subscribeTo(ping) ;
    for (int i=0; i<nburis; ++i) ping->watch(uris[i]) ;
    ping->checkState() ;
  }

  ~Tester(void) {
    unsubscribeFrom(ping) ;
    delete ping ;
  }

} ;

int
main(int argc, char **argv) {
  try {
    char *ENGINE = getenv("NUCLEO_ENGINE") ;
    ReactiveEngine::setEngineType(ENGINE?ENGINE:"default") ;

    int MILLISECONDS = 5000 ;
    int firstArg = parseCommandLine(argc, argv, "t:", "i", &MILLISECONDS) ;
    if (firstArg<0 || argc-firstArg<1) {
	 std::cerr << std::endl << argv[0] ;
	 std::cerr << " [-t milliseconds] host1 [... hostn]" << std::endl ;
	 std::cerr << "(e.g. " << argv[0] << " http://www.lri.fr)" << std::endl ;
	 std::cerr << std::endl << std::endl ;
	 exit(1) ;
    }
    Tester tester(MILLISECONDS, argv+firstArg, argc-firstArg) ;
    ReactiveEngine::run() ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }

  return 0 ;
}
