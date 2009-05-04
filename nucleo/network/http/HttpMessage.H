#ifndef n_HttpMessage_H
#define n_HttpMessage_H

/*
 *
 * nucleo/network/http/HttpMessage.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/StringUtils.H>
#include <nucleo/network/tcp/TcpConnection.H>
#include <nucleo/core/TimeStamp.H>

#include <list>
#include <string>

namespace nucleo {

  // ------------------------------------------------------------------

  struct HttpHeader {
    cistring key ;
    std::string value ;
  } ;

  class HttpMessage {

  public:

    typedef enum  {
	 NEED_STARTLINE,
	 NEED_MP_BOUNDARY,
	 NEED_MP_HEADERS,
	 NEED_BODY,
	 COMPLETE
    } State ;

  private:

    unsigned int _readBufferSize ;
    char *_readBuffer ;

    State _state ;

    std::string _data ;

    std::string _eol, _meol ;

    bool _isMultipart ;
    std::string _multipartBoundary ;
    std::list<HttpHeader> _multipartHeaders ;

    int _contentLength ;

    std::string _startLine ;
    std::list<HttpHeader> _headers ;
    std::string _body ;

    State _parseStartLineAndHeaders(void) ;
    State _skipBoundary(void) ;
    State _parseMultipartHeaders(void) ;
    State _parseBody(void) ;

  public:

    HttpMessage(void) ;
    ~HttpMessage(void) { delete [] _readBuffer ; }

    // ------------------------------------------------------------

    std::string const &eol(void) { return _eol ; }
    std::string const &meol(void) { return _meol ; }

    std::string const &startLine(void) { return _startLine ; }
    std::list<HttpHeader> const &headers(void) { return _headers ; }
    std::string const &body(void) { return _body ; }

    // ------------------------------------------------------------

    int getContentLength(void) { return _contentLength ; }

    bool getHeader(const cistring &key, std::string *var) ;
    bool getHeader(const cistring &key, int *var) ;
    bool getHeader(const cistring &key, unsigned int *var) ;
    bool getHeader(const cistring &key, double *var) ;
    bool getHeader(const cistring &key, TimeStamp::inttype *var) ;

    // ------------------------------------------------------------

    void feed(const std::string &data) {
	 _data.append(data) ;
    }
    int feedFromStream(const int fd) ;

    State getState(void) {
	 return _state ; 
    }
    State parseData(void) ;
    State completeData(void) {
	 if (_state==NEED_BODY) _state = COMPLETE ;
	 return _state ;
    }

    int dataWaiting(void) { return _data.length() ; }

    // ------------------------------------------------------------

    bool parseFromStream(int fd) ;
    bool parseFromStream(TcpConnection *connection) {
	 if (connection)
	   return parseFromStream(connection->getFd()) ;
	 else
	   return false ;
    }

    // ------------------------------------------------------------

    void reset(bool resetData=false) ;
    void next(bool resetData=false) ;

    // ------------------------------------------------------------

    void debug(std::ostream& out, bool printbody=false) const ;
  } ;

  extern const char *oneCR, *twoCR, *oneLF, *twoLF, *oneCRLF, *twoCRLF ;

  // ------------------------------------------------------------------

}

#endif
