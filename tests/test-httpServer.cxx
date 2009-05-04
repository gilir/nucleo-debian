/*
 *
 * tests/test-httpServer.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/AppUtils.H>
#include <nucleo/utils/SignalUtils.H>

#include <nucleo/network/tcp/TcpServer.H>
#include <nucleo/network/tcp/TcpConnection.H>
#include <nucleo/network/http/HttpMessage.H>
#include <nucleo/network/dnssd/DNSServiceAnnouncer.H>

#include <nucleo/core/URI.H>

#include <unistd.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

using namespace nucleo ;

bool signaled=false ;
void exitProc(int s) { signaled = true ; }

void
handleRequest(TcpConnection *client, HttpMessage *request) {
  std::string startline = request->startLine() ;
  std::string method = extractNextWord(startline) ;
  std::string resource = extractNextWord(startline) ;

  char *preamble = "HTTP/1.0 200 Ok\015\012Content-Type: text/html\015\012\015\012" ;
  write(client->getFd(), preamble, strlen(preamble)) ;

  URI uri(resource) ;
  std::string path = uri.path ;
  std::string query = uri.query ;

  std::string user = client->userLookUp() ;
  std::string machine = client->machineLookUp() ;

  std::stringstream msg ;
  msg << "<body>Hello " << user << " (" << machine << ") </body>";
  std::string response = msg.str() ;

  write(client->getFd(), response.c_str(), response.size()) ;
}

int
main(int argc, char **argv) {
  int PORT = 0 ;

  if (parseCommandLine(argc, argv, "p:", "i", &PORT)<0) {
    std::cerr << std::endl << argv[0] << " [-p port]" ;
    std::cerr << std::endl ;
    exit(1) ;
  }
  
  TcpServer server(PORT) ;
  std::cerr << "Server listening on http://" << server.getHostName() << ":" << server.getPortNumber() << std::endl ;

  DNSTextRecord textRecord ;
  DNSServiceAnnouncer announcer = DNSServiceAnnouncer("test-httpServer", "_http._tcp.",
										    server.getPortNumber(),
										    &textRecord) ;
  if (!announcer.getStatus()!=DNSServiceAnnouncer::NOERROR)
    std::cerr << "Unable to announce service" << std::endl ;

  trapAllSignals(exitProc) ;

  while (!signaled) {
    try {
	 TcpConnection *client = server.waitForNewClient() ;
	 
	 HttpMessage request ;
	 if (! request.parseFromStream(client->getFd())) {
	   char *response = "HTTP/1.0 400 Bad Request\015\012\015\012" ;
	   write(client->getFd(), response, strlen(response)) ;
	 } else {
	   handleRequest(client, &request) ;
	 }
 
	 delete client ;
    } catch (std::runtime_error e) {
	 std::cerr << "Runtime error: " << e.what() << std::endl ;
    } catch (std::exception e) {
	 std::cerr << "Error: " << e.what() << std::endl ;
    }
  }
  
  return 0 ;
}
