#ifndef VideoService_H
#define VideoService_H

#include "Notifier.H"

#include <nucleo/utils/ConfigDict.H>
#include <nucleo/image/Image.H>
#include <nucleo/network/tcp/TcpConnection.H>
#include <nucleo/network/http/HttpMessage.H>

using namespace nucleo ;

class VideoService {

protected:

  std::string getResourceFilename(const char *root, std::string path) ;

  void parseFromConnection(TcpConnection *connection) ;
  void parseFromNotifier(std::string program) ;

public:

  ConfigDict serverConfig ;
  std::string uuid ;

  // extracted from the TCP connection and the HTTP request
  std::string http_user, http_host ;
  std::string http_referer, http_user_agent ;

  // obtained from the server configuration and/or the query string
  double image_rate ;
  std::string image_size ;
  unsigned int image_blur ;
  unsigned int image_quality ;

  // the actual query
  typedef enum {ERROR, GRAB, PUSH, NUDP, SENDFILE, REDIRECT} command ;
  command cmd ;
  std::string arg ;      // resolved resource or command parameter
  std::string nudpInfo ; // only for NUDP

  Notifier *notifier ;

  VideoService(ConfigDict &dict, TcpConnection *connection, std::string uuid) ;
  ~VideoService(void) { 
    delete notifier ;
  }

  void debug(std::ostream& out) const ;

} ;

#endif
