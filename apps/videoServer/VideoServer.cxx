/*
 *
 * apps/videoServer/VideoServer.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "VideoServer.H"
#include "VideoService.H"
#include "FileStreamer.H"
#include "VideoStreamer.H"

#include <nucleo/core/UUID.H>

#include <iostream>
#include <stdexcept>

VideoServer::VideoServer(char *configfile) {
  config.loadFrom(configfile) ;
  std::cerr << config.dump() << std::endl ;

  int port = config.get("server-port", 5555) ;
  int backlog = config.get("server-backlog", 10) ;
  server = new TcpServer(port, backlog, true) ;
  subscribeTo(server) ;

  std::string rdvName = config.get("rendezvous-name", std::string("videoServer")) ;
  std::string rdvResource = config.get("rendezvous-resource", std::string("/grab/video")) ;
  DNSTextRecord record ;
  record["path"] = rdvResource ;
  announcer = new DNSServiceAnnouncer(rdvName.c_str(), "_http._tcp.", port, &record) ;
}

VideoServer::~VideoServer(void) {
  delete announcer ;

  unsubscribeFrom(server) ;
  delete server ;
}

void
VideoServer::react(Observable *obs) {
  try {
    if (obs!=server) return ; // just in case...

    TcpConnection *connection = server->getNewClient() ;

    std::string uuid = UUID::createAsString() ;
    VideoService *service = new VideoService(config,
									connection,
									uuid) ;

    std::stringstream response_stream ;
    if (service->cmd==VideoService::ERROR) {
	 response_stream << "HTTP/1.0 " << service->arg << twoCRLF
				  << service->arg ;
    } else if (service->cmd==VideoService::REDIRECT) {
	 response_stream << "HTTP/1.0 302 Moved Temporarily" << oneCRLF
				  << "Location: " << service->arg << twoCRLF ;
    } else if (service->cmd==VideoService::SENDFILE) {
	 try {
	   new FileStreamer(connection, service) ;
	 } catch (std::runtime_error e) {
	   std::cerr << e.what() << std::endl ;
	   response_stream << "HTTP/1.0 404 Not Found" << twoCRLF
				    << "404 Not found" ;
	 }
    } else {
	 try {
	   new VideoStreamer(connection, service) ;
	 } catch (std::runtime_error e) {
	   std::cerr << e.what() << std::endl ;
	   response_stream << "HTTP/1.0 400 Bad Request" << twoCRLF
				    << "400 Bad Request" ;
	 }
    }

    std::string response = response_stream.str() ;
    if (response!="") {
	 // std::cerr << "Sending response..." << std::endl << response ;
	 connection->send(response.c_str(), response.size(), true) ;
	 delete connection ;
	 delete service ;
    }

  } catch (std::runtime_error e) {
    std::cerr << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << e.what() << std::endl ;
  } catch (...) {
    std::cerr << "Unknown exception" << std::endl ;
  }
}
