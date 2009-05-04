/*
 *
 * demos/misc/disbonjour.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/nucleo.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/network/dnssd/DNSServiceAnnouncer.H>
#include <nucleo/network/dnssd/DNSServiceBrowser.H>
#include <nucleo/image/ImageBridge.H>
#include <nucleo/image/sink/nserverImageSink.H>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstdlib>

#include <unistd.h>

using namespace nucleo ;

class NiceGuy : public ReactiveObject {

protected:

  std::string name ;
  ImageSource *source ;
  Image image ;
  nserverImageSink *server ;
  DNSServiceAnnouncer *announcer ;
  DNSServiceBrowser *browser ;
  std::map<std::string,ImageBridge*> friends ;

  void react(Observable *obs) {
    if (source->getNextImage(&image))
	 server->handle(&image) ;

    DNSServiceBrowser::Event *event = 0 ;
    while ( (event = browser->getNextEvent()) ) {
	 std::map<std::string,ImageBridge*>::iterator f = friends.find(event->service.name) ;
	 if (f!=friends.end()) {
	   ImageBridge *viewer = (*f).second ;
	   if (viewer) {
		delete viewer ;
		friends[event->service.name] = 0 ;
	   }
	   if (event->event==DNSServiceBrowser::Event::FOUND) {
		std::stringstream sourceURI, sinkURI ;
		sourceURI << "http://" << event->service.host << ":" << event->service.port ;
		std::string uri = sourceURI.str() ;
		std::cerr << "found " << event->service.name << " (" << uri << ")" << std::endl ;
		sinkURI << "glwindow:?title=" << event->service.name << "->" << name ;
		friends[event->service.name] = new ImageBridge(uri, sinkURI.str()) ;
	   } else if (event->event==DNSServiceBrowser::Event::LOST) {
		std::cerr << "lost " << event->service.name << std::endl ;
	   }
	 } else {
	   std::cerr << "I don't care about " << event->service.name << std::endl ;
	 }

	 delete event ;
    }
  }

public:

  NiceGuy(char *src, char *n, char *type, int port) {
    name = n ;

    server = new nserverImageSink(port) ;
    if (!server) throw std::runtime_error("Unable to create server") ;
    subscribeTo(server) ;
    server->start() ;

    source = ImageSource::create(src) ;
    if (!source) throw std::runtime_error("Unable to create image source") ;
    subscribeTo(source) ;
    source->start() ;

    browser = new DNSServiceBrowser(type) ;
    if (!browser) throw std::runtime_error("Unable to create ServiceBrowser") ;
    subscribeTo(browser) ;

    announcer = new DNSServiceAnnouncer(n, type, server->getPortNumber()) ;
    if (!announcer) throw std::runtime_error("Unable to create ServiceAnnouncer") ;
  }

  void lookFor(std::string name) {
    std::map<std::string,ImageBridge*>::iterator i = friends.find(name) ;
    if (i!=friends.end()) return ;

    friends[name] = 0 ;
  }

  ~NiceGuy(void) {
    unsubscribeFrom(source) ;
    delete source ;
    unsubscribeFrom(server) ;
    delete server ;
    unsubscribeFrom(browser) ;
    delete browser ;
    delete announcer ;

    // FIXME: we should also delete the image sinks associated to our
    // friends...
  }

} ;

// ---------------------------------------------------------------------------

int
main(int argc, char **argv) {

  try {
    char *SOURCE = 0 ;
    char *NAME = "disbonjour" ;
    char *TYPE = "_http._tcp." ;
    int PORT = 0 ;

    int firstArg = parseCommandLine(argc, argv, "i:n:t:p:", "sssi",
							 &SOURCE, &NAME, &TYPE, &PORT) ;

    if (firstArg<0) {
	 std::cerr << std::endl << argv[0] ;
	 std::cerr << " [-i source] [-n name] peer1 ... peern" << std::endl ;
	 std::cerr << std::endl << std::endl ;
	 exit(1) ;
    }

    if (!SOURCE) SOURCE = getenv("NUCLEO_SRC1") ;

    NiceGuy ng(SOURCE, NAME, TYPE, PORT) ;
    for (int i=firstArg; i<argc; ++i) ng.lookFor(argv[i]) ;

    ReactiveEngine::run() ;

    return 0 ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (...) {
    std::cerr << "Unknown error" << std::endl ;
  }

  return -1 ;
}
