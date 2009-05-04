#ifndef n_nserverImageSink_H
#define n_nserverImageSink_H

/*
 *
 * nucleo/image/sink/nserverImageSink.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/tcp/TcpServer.H>
#include <nucleo/network/tcp/TcpUtils.H>
#include <nucleo/network/http/ServerPush.H>
#include <nucleo/network/NetworkUtils.H>
#include <nucleo/image/sink/ImageSink.H>
#include <nucleo/image/encoding/Conversion.H>

#include <queue>
#include <sstream>
#include <stdexcept>

#include <sys/socket.h>

namespace nucleo {

  class nserverImageSink : public ImageSink {

  private:

    class spsClient : public ReactiveObject {
    private:
	 int socket ;
	 ServerPush *serverpush ;
    public:
	 spsClient(int fd) {
	   socket = fd ;
	   serverpush = new ServerPush(socket) ;
	 }
	 bool push(const char *content_type, char *ptr, int size, std::string header) {
	   serverpush->push(content_type, ptr, size, header.c_str()) ;
	   return true ;
	 }
	 ~spsClient() {
	   shutdown(socket,2) ;
	   close(socket) ;
	   delete serverpush ;
	 }
    } ;

    Image::Encoding _encoding ;
    unsigned int _quality ;

    TcpServer *_server ;
    int _port, _backLog ;
    unsigned int _maxClients ;
    std::queue<spsClient *> _clients ;
    bool _debug ;

  protected:

    void react(Observable*) {
	 TcpConnection *connection = _server->getNewClient() ;
	 while (connection) {
	   if (_debug)
		std::cerr << "New client: " << connection->machineLookUp() << std::endl ;
	   int fd = connection->getFd() ;
	   setDefaultTcpSocketOptions(fd, false) ;
	   if (_clients.size()==_maxClients) {
		spsClient *client = _clients.front() ;
		_clients.pop() ;
		delete client ;
	   }
	   _clients.push(new spsClient(fd)) ;

	   connection = _server->getNewClient() ;
	 }
    }

  public:

    nserverImageSink(int port=0,
				 Image::Encoding encoding=Image::JPEG, unsigned int quality=60,
				 unsigned int maxclients=10, int backlog=10, bool debug=false) {
	 _server = 0 ;
	 _port = port ;
	 _encoding = encoding ;    
	 _quality = quality ;
	 _backLog = backlog ;
	 _maxClients = maxclients ;
	 _debug = debug ;
    }

    nserverImageSink(const URI &uri) {
	 _server = 0 ;
	 _port = uri.port ;

	 std::string query=uri.query ;

	 std::string encoding ;
	 _encoding = Image::JPEG ;    
	 if (URI::getQueryArg(query, "encoding", &encoding))
	   _encoding = Image::getEncodingByName(encoding) ;
    
	 _quality = 60 ;
	 URI::getQueryArg(query, "quality", &_quality) ;

	 _backLog = 10 ;
	 URI::getQueryArg(query, "backlog", &_backLog) ;

	 _maxClients = 10 ;
	 URI::getQueryArg(query, "maxclients", &_maxClients) ;

	 _debug = URI::getQueryArg(query, "debug") ;
    }

    ~nserverImageSink(void) {
	 stop() ;
    }

    ImageSink::state getState(void) {
	 return (_server!=0?STARTED:STOPPED) ;
    }

    bool start(void) {
	 if (_server) return false ;
	 _server = new TcpServer(_port, _backLog, false) ; 
	 subscribeTo(_server) ;
	 if (!_port) {
	   std::cerr << "nserver listening on " << getURI() ;
	   // std::cerr << " (backlog=" << _backLog << ", maxclients=" << _maxClients << ")" ;
	   std::cerr << std::endl ;
	 }
	 frameCount = 0 ; sampler.start() ;
	 return true ;
    }

    int getPortNumber(void) {
	 if (!_server) return -1 ;
	 return _server->getPortNumber() ;
    }

    std::string getURI(void) {
	 std::stringstream uri ;
	 uri << "http://" << getHostName() << ":" << _server->getPortNumber() ;
	 return uri.str() ;
    }

    bool handle(Image *img) {
	 if (_clients.empty()) return false ;

	 // img->debug(std::cerr) ; std::cerr << " <" ;
	 
	 Image copy(*img) ;
	 if (!convertImage(&copy, _encoding, _quality)) return false ;

	 std::string mimetype = copy.getMimeType() ;
	 TimeStamp::inttype timestamp = copy.getTimeStamp() ;
	 char *ptr = (char *)copy.getData() ;
	 int size = (int)copy.getSize() ;
	 
	 std::stringstream ssheaders ;
	 ssheaders << "nucleo-framerate: " << sampler.average() << "\r\n" ; // FIXME: deprecated
	 ssheaders << "nucleo-timestamp: " << (timestamp==TimeStamp::undef ? TimeStamp::createAsInt() : timestamp) << "\r\n" ;
	 ssheaders << "nucleo-image-width: " << copy.getWidth() << "\r\n" ;
	 ssheaders << "nucleo-image-height: " << copy.getHeight() ;
	 std::string headers = ssheaders.str() ;

	 frameCount++ ; sampler.tick() ;

	 for (unsigned int i=0; i<_clients.size(); ++i) {
	   spsClient *client = _clients.front() ;
	   _clients.pop() ;
	   try {
		client->push(mimetype.c_str(), ptr, size, headers) ;
		_clients.push(client) ;
		// std::cerr << "+" << std::flush ;
	   } catch (std::runtime_error e) {
		// std::cerr << "-" << std::flush ;
	   }
	 }
	 // std::cerr << ">" << std::endl ;

	 return true ;
    }

    bool stop(void) {
	 if (!_server) return false ;

	 while (! _clients.empty()) {
	   spsClient *client = _clients.front() ;
	   _clients.pop() ;
	   delete client ;
	 }
	 delete _server ;

	 sampler.stop() ;
	 return true ;
    }
    
  } ;

}

#endif
