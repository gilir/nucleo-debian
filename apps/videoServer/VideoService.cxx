#include "VideoService.H"
#include "Notifier.H"

#include <nucleo/core/URI.H>
#include <nucleo/utils/FileUtils.H>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

static const char *commandNames[] = {
  "ERROR", "GRAB", "PUSH", "NUDP", "SENDFILE", "REDIRECT"
} ;

std::string
VideoService::getResourceFilename(const char *root, std::string path) {
  std::string filename = reducePath(path) ;
  trimString(filename," ./") ; // why?

  std::string fullpath = serverConfig.get(root,std::string("/tmp")) ;
  if (fullpath=="") return filename ;

  if (fullpath[fullpath.size()-1]!='/') fullpath = fullpath+"/" ;
  fullpath = fullpath + filename ;
  return fullpath ;
}

void
VideoService::parseFromConnection(TcpConnection *connection) {
  if (serverConfig.get("http-user-lookup", false))
    try { http_user = connection->userLookUp() ; } catch (...) {}
  try { http_host = connection->machineLookUp() ; } catch (...) {}

  HttpMessage request ;
  if (!request.parseFromStream(connection)) return ;

  request.getHeader("referer", &http_referer) ;
  request.getHeader("user-agent", &http_user_agent) ;

  std::string http_startline = request.startLine() ;
  std::string http_method = extractNextWord(http_startline) ;
  std::string http_resource = extractNextWord(http_startline) ;

  URI uri(std::string("videoServer:")+http_resource) ;
  std::string path = uri.path ;
  std::string query = uri.query ;

  URI::getQueryArg(query, "size", &image_size) ;
  URI::getQueryArg(query, "framerate", &image_rate) ;
  URI::getQueryArg(query, "blur", &image_blur) ;
  URI::getQueryArg(query, "quality", &image_quality) ;

  std::string c = path.substr(1,4) ;
  if (c=="grab") {
    cmd = GRAB ;
  } else if (c=="push") {
    std::string useragent ;
    request.getHeader("user-agent", &useragent) ;
    if (useragent.find("nucleo/")!=std::string::npos
	   || useragent.find("Gecko/20")!=std::string::npos
	   || useragent.find("MSIE 5.23; Mac_PowerPC")!=std::string::npos)
	 cmd = PUSH ;
    else
	 cmd = GRAB ;
  } else if (c=="nudp") {
    cmd = NUDP ;
    URI::getQueryArg(query, "nudp", &nudpInfo) ;
  } else {
    cmd = SENDFILE ;
    arg = getResourceFilename("document-root", path) ;
    return ;
  }

  std::string sourceType(path,6,5) ;
  if (sourceType=="video") {
    arg = serverConfig.get("image-source", std::string("videoin:")) ;
#if 0
    if (query!="") {
	 if (arg.find("?")!=std::string::npos) arg = arg+"&"+query ;
	 else arg = arg+"?"+query ;
    }
#endif
  } else if (sourceType=="movie") {
    arg = getResourceFilename("movie-root", path.substr(12, path.size()-12)) ;
#if 0
    if (query!="") {
	 if (arg.find("?")!=std::string::npos) arg = arg+"&"+query ;
	 else arg = arg+"?"+query ;
    }
#endif
  } else if (sourceType=="relay") {
    URI::getQueryArg(query, "src", &arg) ;
  } else {
    cmd = ERROR ;
    arg = "404 Not Found" ;
  }
}

void
VideoService::parseFromNotifier(std::string program) {
  notifier = 0 ;

  try {
    notifier = new Notifier(program) ;

    std::stringstream nMsg ;
    nMsg << commandNames[cmd] << " " << arg << std::endl
	    << "http-user: " << http_user << std::endl
	    << "http-host: " << http_host << std::endl
	    << "http-referer: " << http_referer << std::endl
	    << "http-user-agent: " << http_user_agent << std::endl
	    << "image-size: " << image_size << std::endl
	    << "image-rate: " << image_rate << std::endl
	    << "image-blur: " << image_blur << std::endl
	    << "image-quality: " << image_quality << std::endl
	    << std::endl ;
    std::string tmp = nMsg.str() ;
    std::cerr << "---------" << std::endl << tmp << "---------" << std::endl ;
    notifier->send(tmp) ;  

    std::string line = notifier->receive("\n") ;
    // std::cerr << "RECEIVED: " << line << std::endl ;
    std::string theCmd = extractNextWord(line) ;
    for (cmd=(command)5; cmd>0; cmd=(command)(cmd-1))
	 if (theCmd==commandNames[cmd]) break ;
    arg = line ;

    for (;;) {
	 line = notifier->receive("\n") ;
	 std::string key = extractNextWord(line) ;
	 if (key=="image-size:")
	   image_size = line ;
	 else if (key=="image-rate:")
	   image_rate = atof(line.c_str()) ;
	 else if (key=="image-blur:")
	   image_blur = atoi(line.c_str()) ;
	 else if (key=="image-quality:")
	   image_quality = atoi(line.c_str()) ;
	 else {
	   // std::cerr << "RECEIVED: \"" << key << "\" \"" << line << "\"" << std::endl ;
	   break ;
	 }
    }

    return ;
  } catch (std::runtime_error e) {
    std::cerr << e.what() << std::endl ;
  } catch (...) {
  }

  if (notifier) {
    delete notifier ;
    notifier = 0 ;
  }
}

VideoService::VideoService(ConfigDict &dict, TcpConnection *connection,
					  std::string id) {
  // std::cerr << "Creating VideoService " << this << std::endl ;

  serverConfig = dict ;
  uuid = id ;

  image_size = serverConfig.get("image-size",std::string("QCIF")) ;
  image_rate = serverConfig.get("image-rate",0) ;
  image_blur = serverConfig.get("image-blur",0) ;
  image_quality = serverConfig.get("image-quality",60) ;

  http_user="?" ; http_host="?" ;
  http_referer="?" ; http_user_agent="?" ;

  cmd = ERROR ;
  arg = "400 Bad Request" ;
  nudpInfo="?" ;

  parseFromConnection(connection) ;
  // debug(std::cerr) ;

  std::string notifprog = serverConfig.get("notifier", std::string("notifier")) ;
  parseFromNotifier(notifprog) ;
  debug(std::cerr) ;
}

void
VideoService::debug(std::ostream& out) const {
  out << "VideoService (" << this << ")" << std::endl ;

  out << "  http-user: " << http_user << "@" << http_host << std::endl ;   
  if (http_referer!="?")
    out << "  http-referer: " << http_referer << std::endl ;
  if (http_user_agent!="?")
    out << "  http-user-agent: " << http_user_agent << std::endl ;

  out << "  -- " << commandNames[cmd] << " " << arg ;
  if (cmd==NUDP) out << " (" << nudpInfo << ")" ;
  out << " --" << std::endl ;

  out << "  image: "
	 << "size=" << image_size
	 << ", rate=" << image_rate
	 << ", blur=" << image_blur
	 << ", quality=" << image_quality << std::endl ;
}
