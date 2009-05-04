/*
 *
 * tests/test-rdv.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/AppUtils.H>
#include <nucleo/utils/SignalUtils.H>

#include <nucleo/network/dnssd/DNSServiceAnnouncer.H>
#include <nucleo/network/dnssd/DNSServiceBrowser.H>

#include <nucleo/core/ReactiveEngine.H>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

using namespace nucleo ;

void exitProc(int s) { ReactiveEngine::stop() ; }

class Reactor : public ReactiveObject {

protected:

  DNSServiceBrowser *browser ;

  void react(Observable*) {
    DNSServiceBrowser::Event *event = browser->getNextEvent() ;
    if (event) {
	 event->debug(std::cerr) ;
	 delete event ;
    }
  }

public:

  Reactor(DNSServiceBrowser *b) {
    browser = b ;
    subscribeTo(browser) ;
  }

} ;

int
main(int argc, char **argv) {
  char *ENGINE = getenv("NUCLEO_ENGINE") ;
  ReactiveEngine::setEngineType(ENGINE?ENGINE:"default") ;

  bool NO_ANNOUNCER=false, NO_FINDER=false ; 
  char *NAME = "nucleo" ;
  char *TYPE = "_http._tcp." ;
  int PORT = 8079 ;

  int firstArg = parseCommandLine(argc, argv, "afn:t:p:", "bbssib",
						    &NO_ANNOUNCER, &NO_FINDER,
						    &NAME, &TYPE, &PORT) ;
  if (firstArg<0) {
    std::cerr << std::endl << argv[0] << " [-n name] [-t type] [-p port] [-a (no announcer)] [-f (no finder)]" << std::endl ;
    exit(1) ;
  }

  DNSTextRecord textRecord ;
  for (int i=firstArg; i<argc-1; i+=2)
    textRecord[argv[i]] = argv[i+1] ;

  try {

    DNSServiceAnnouncer *announcer = 0 ;
    DNSServiceBrowser *browser = 0 ;
    Reactor *reactor = 0 ;

    if (!NO_ANNOUNCER) {
	 announcer = new DNSServiceAnnouncer(NAME, TYPE, PORT, &textRecord) ;
	 if (!announcer) throw std::runtime_error("Unable to create DNSServiceAnnouncer") ;
	 std::cerr << "DNSServiceAnnouncer created" << std::endl ;
    }

    if (!NO_FINDER) {
	 browser = new DNSServiceBrowser(TYPE) ;
	 if (!browser) throw std::runtime_error("Unable to create DNSServiceBrowser") ;
	 reactor = new Reactor(browser) ;
	 std::cerr << "ServiceFinder and Reactor created" << std::endl ;
    }

    trapAllSignals(exitProc) ;
    std::cerr << "Here we go!" << std::endl ;
    ReactiveEngine::run() ;
    std::cerr << "done!" << std::endl ;

    delete reactor ;
    delete browser ;
    delete announcer ;

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (...) {
    std::cerr << "Unknown error" << std::endl ;
  }

  return 0 ;
}
