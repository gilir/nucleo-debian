/*
 *
 * nucleo/network/http/HttpMessage.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/FileKeeper.H>
#include <nucleo/utils/FileUtils.H>
#include <nucleo/network/http/HttpMessage.H>

#include <cstdlib>
#include <unistd.h>

#include <iostream>
#include <string>

namespace nucleo {

  // ------------------------------------------------------------------

  const char * oneCR = "\015" ;
  const char * twoCR = "\015\015" ;
  const char * oneLF = "\012" ;
  const char * twoLF = "\012\012" ;
  const char * oneCRLF = "\015\012" ;
  const char * twoCRLF = "\015\012\015\012" ;

  // ------------------------------------------------------------------

  static const unsigned int READBUFFER_MAX_SIZE = 4194304 ;
  static const unsigned int READBUFFER_INIT_SIZE = 32768 ;
  static const unsigned int READBUFFER_INC_SIZE = 1024 ;

  static const char *stateNames[] = {
    "NEED_STARTLINE",
    "NEED_MP_BOUNDARY",
    "NEED_MP_HEADERS",
    "NEED_BODY",
    "COMPLETE"
  } ;

  // ------------------------------------------------------------------

  static std::string::size_type
  findEolMarker(const std::string &block, std::string &eol) {
    std::string::size_type pos = block.find(twoCRLF) ;
    if (pos!=std::string::npos) eol = oneCRLF ;
    else {
	 pos = block.find(twoLF) ;
	 if (pos!=std::string::npos) eol = oneLF ;
	 else {
	   pos = block.find(twoCR) ;
	   if (pos!=std::string::npos) eol = oneCR ;
	 }
    }
    return pos ;
  }

  static void
  parseHeaders(const std::string &block, std::string &eol, std::list<HttpHeader> &headers) {
    std::string::size_type blocklen = block.length() ;
    std::string::size_type eollen = eol.length() ;
    std::string::size_type pos=0 ;
    for (bool loop=true; loop; ) {
	 // --- Get one header line -------------------------------
	 std::string header ;
	 std::string::size_type end_of_header = block.find(eol, pos) ;
	 if (end_of_header==std::string::npos) {
	   header.assign(block, pos, blocklen-pos) ;
	   loop = false ;
	 } else {
	   header.assign(block, pos, end_of_header-pos) ;
	   pos = end_of_header+eollen ;
	 }
	 // --- Add the header to the list ------------------------
	 std::string::size_type psep = header.find(":") ;
	 if (psep!=std::string::npos) {
	   HttpHeader h ;
	   h.key.assign(header.c_str(),0,psep) ;
	   h.value.assign(header,psep+1,header.length()-psep) ;
	   trimString(h.value) ;
	   headers.push_back(h) ;
	 }
    }
  }

  // ------------------------------------------------------------------

  HttpMessage::HttpMessage(void) {
    _readBufferSize = READBUFFER_INIT_SIZE ;
    _readBuffer = new char [_readBufferSize] ;
    _eol = oneCRLF ;
    _meol = oneCRLF ;
    reset() ;
  }

  void
  HttpMessage::reset(bool resetData) {
    _state = NEED_STARTLINE ;
    _isMultipart = 0 ;
    _multipartBoundary = "" ;
    _multipartHeaders.clear() ;
    _contentLength = -1 ;
    _startLine = "" ;
    _headers.clear() ;
    _body = "" ;
    if (resetData) _data = "" ;
  }

  void
  HttpMessage::next(bool resetData) {
    if (_isMultipart) {
	 _state = NEED_STARTLINE ;
	 _multipartHeaders.clear() ;
	 _contentLength = -1 ;
	 _body = "" ;
	 if (resetData) _data = "" ;
    } else reset(resetData) ;
  }

  // ------------------------------------------------------------------

  bool
  HttpMessage::getHeader(const cistring &key, std::string *var) {
    std::list<HttpHeader>::const_iterator i ;

    // First, check the multipart headers
    for( i=_multipartHeaders.begin(); i!=_multipartHeaders.end(); ++i )
	 if ((*i).key==key) {
	   var->assign((*i).value.c_str()) ;
	   return true ;
	 }
    // Then check the global headers
    for( i=_headers.begin(); i!=_headers.end(); ++i )
	 if ((*i).key==key) {
	   var->assign((*i).value.c_str()) ;
	   return true ;
	 }
    return false ;
  }

  bool
  HttpMessage::getHeader(const cistring &key, int *var) {
    std::string tmp ;
    bool found = getHeader(key, &tmp) ;
    if (found) *var = atoi(tmp.c_str()) ;
    return found ;
  }

  bool
  HttpMessage::getHeader(const cistring &key, unsigned int *var) {
    std::string tmp ;
    bool found = getHeader(key, &tmp) ;
    if (found) *var = (unsigned int)atoi(tmp.c_str()) ;
    return found ;
  }

  bool
  HttpMessage::getHeader(const cistring &key, double *var) {
    std::string tmp ;
    bool found = getHeader(key, &tmp) ;
    if (found) *var = (unsigned int)atof(tmp.c_str()) ;
    return found ;
  }

  bool
  HttpMessage::getHeader(const cistring &key, TimeStamp::inttype *var) {
    std::string tmp ;
    bool found = getHeader(key, &tmp) ;
    if (found) *var = TimeStamp::createAsIntFromString(tmp) ;
    return found ;
  }

  // ------------------------------------------------------------------

  HttpMessage::State
  HttpMessage::_parseStartLineAndHeaders(void) {
    if (_isMultipart) return NEED_MP_BOUNDARY ;
    std::string::size_type eHeaders = findEolMarker(_data, _eol) ;
    // Couldn't find eol marker, abort
    if (eHeaders==std::string::npos) return _state ;
    // Start line
    std::string::size_type bHeaders = _data.find(_eol) ;
    _startLine.assign(_data, 0, bHeaders) ;
    // Headers
    std::string::size_type l = _eol.length() ;
    std::string block(_data, bHeaders+l, eHeaders-bHeaders-1) ;
    _headers.clear() ;
    parseHeaders(block, _eol, _headers) ;
    // Leave the other bytes
    _data.erase(0,eHeaders+2*l) ;
    return NEED_MP_BOUNDARY ;
  }

  HttpMessage::State
  HttpMessage::_skipBoundary(void) {
    if (_isMultipart) {
	 std::string::size_type pos = _data.find(_multipartBoundary) ;
	 if (pos!=std::string::npos) {
	   _data.erase(0,pos+_multipartBoundary.length()) ;
	   return NEED_MP_HEADERS ;
	 }
	 return _state ;
    }
    return NEED_MP_HEADERS ;
  }

  HttpMessage::State
  HttpMessage::_parseMultipartHeaders(void) {
    if (_isMultipart) {
	 std::string::size_type eHeaders = findEolMarker(_data, _meol) ;
	 // Couldn't find meol marker, abort
	 if (eHeaders==std::string::npos) return _state ;
	 // Parse multipart headers
	 std::string::size_type l = _meol.length() ;
	 std::string block(_data, l, eHeaders-1) ;
	 _multipartHeaders.clear() ;
	 parseHeaders(block, _eol, _multipartHeaders) ;
	 // Leave the other bytes
	 _data.erase(0,eHeaders+2*l) ;
    } 
    return NEED_BODY ;
  }

  HttpMessage::State
  HttpMessage::_parseBody(void) {
    if (_contentLength!=-1) {
	 int needed = _contentLength-_body.length() ;
	 int ready = _data.length() ;
#if 0
	 if (needed>ready) {
	   _body.append(_data) ;
	   _data = "" ;
	 } else {
	   _body.append(_data,0,needed) ;
	   _data.erase(0, needed) ;
	   return COMPLETE ;
	 }
#else
	 if (needed<=ready) {
	   _body.append(_data,0,needed) ;
	   _data.erase(0, needed) ;
	   return COMPLETE ;
	 }
#endif
    } else if (_isMultipart) {
	 std::string::size_type pos = _data.find(_multipartBoundary) ;
	 if (pos!=std::string::npos) {
	   _body.append(_data, 0, pos) ;
	   _data.erase(0, pos) ;
	   return COMPLETE ;
	 } else {
	   _body.append(_data) ;
	   _data = "" ;
	 }
    } else {
	 _body.append(_data) ;
	 _data = "" ;
	 if (_startLine.find("GET")!=std::string::npos || _startLine.find("HEAD")!=std::string::npos)
	   return COMPLETE ;
    }
    return _state ;
  }

  // ------------------------------------------------------------------

  HttpMessage::State
  HttpMessage::parseData(void) {

    for (;;) {
	 State newstate=_state ;
	 // std::cerr << "STATE = " << stateNames[_state] << std::endl ;
	 switch(_state) {
	 case NEED_STARTLINE:
	   newstate = _parseStartLineAndHeaders() ;
	   break ;
	 case NEED_MP_BOUNDARY:
	   newstate = _skipBoundary() ;	 
	   break ;
	 case NEED_MP_HEADERS:
	   newstate = _parseMultipartHeaders() ;	 
	   break ;
	 case NEED_BODY:
	   newstate = _parseBody() ;
	   break ;
	 case COMPLETE:
	   newstate = COMPLETE ;
	   break ;
	 }

	 if (newstate==_state) break ;

	 switch(newstate) {
	 case NEED_STARTLINE:
	   break ;
	 case NEED_MP_BOUNDARY: {
	   std::string ctype ;
	   if (!_isMultipart && getHeader("content-type",&ctype)) {
		std::string serverpush("multipart/x-mixed-replace;boundary=") ;
		std::string::size_type pos = ctype.find(serverpush) ;
		if (pos!=std::string::npos) {
		  _isMultipart = true ;
		  _multipartBoundary.assign(ctype.c_str()+pos+serverpush.length()) ;
		}
	   }
	 } break ;
	 case NEED_MP_HEADERS:
	   break ;
	 case NEED_BODY:
	   getHeader("content-length",&_contentLength) ;
	   break ;
	 case COMPLETE: {
	   unsigned int newSize=_readBufferSize, bodySize=_body.size() ;
	   while (newSize<bodySize) newSize+=READBUFFER_INC_SIZE ;
	   if (newSize>_readBufferSize && newSize<READBUFFER_MAX_SIZE) {
		delete [] _readBuffer ;
		_readBufferSize = newSize ;
		_readBuffer = new char [newSize] ;
		// std::cerr << "HttpMessage: readBufferSize = " << _readBufferSize << std::endl ;
	   }
	 } break ;
	 }

	 _state = newstate ;
    
    }

    return _state ;
  }

  // ------------------------------------------------------------------

  int
  HttpMessage::feedFromStream(const int fd) {
    int bytesread = read(fd, _readBuffer, _readBufferSize) ;
    // std::cerr << "HttpMessage: read " << bytesread << " bytes (buffer size is " << _readBufferSize << ")" << std::endl ;
    if (bytesread>0) _data.append(_readBuffer, bytesread) ;
    // if (bytesread==-1) perror("coucou") ;
    return bytesread ;
  }

  // ------------------------------------------------------------------

  bool
  HttpMessage::parseFromStream(int fd) {
    FileKeeper *fk = FileKeeper::create(fd, FileKeeper::R) ;
    for(;;) {
	 ReactiveEngine::step() ;
	 if (! (fk->getState()&FileKeeper::R)) continue ;
	 if (feedFromStream(fd)<1) {
	   completeData() ;
	   parseData() ;
	   break ;
	 }
	 if (parseData()==HttpMessage::COMPLETE) break ;
    }

    delete fk ;
    return (_state==COMPLETE) ;
  }

  // ------------------------------------------------------------------

  void
  HttpMessage::debug(std::ostream& out, bool printbody) const {
    out << std::endl << "---------> " << stateNames[_state] << std::endl ;
    out << "---------- startLine" << std::endl ;
    out << _startLine ;
    out << std::endl << "---------- headers" << std::endl ;
    
    std::list<HttpHeader>::const_iterator i ;

    for( i=_headers.begin(); i!=_headers.end(); ++i )
	 out << (*i).key.c_str() << " = " << (*i).value.c_str() << std::endl ;

    if (_isMultipart) {
	 out << "---------- Multipart headers (boundary=\"" ;
	 out << _multipartBoundary ;
	 out << "\")" << std::endl ;

	 for( i=_multipartHeaders.begin(); i!=_multipartHeaders.end(); ++i )
	   out << (*i).key.c_str() << " = " << (*i).value.c_str() << std::endl ;
    }
    out << "---------- body (size=" << _body.size() << ")" << std::endl ;
    if (printbody) out << _body ; else out << "..." ;
    out << std::endl << "-------------------------------------------------" << std::endl << std::endl ;
  }

  // ------------------------------------------------------------------

}
