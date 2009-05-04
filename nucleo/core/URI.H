#ifndef n_URI_H
#define n_URI_H

/*
 *
 * nucleo/core/URI.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
  See also :

    http://www.ics.uci.edu/pub/ietf/uri/rfc2396.txt
    http://www.ics.uci.edu/~fielding/
    http://www.ics.uci.edu/~fielding/url/
  */

#include <nucleo/core/TimeStamp.H>

#include <string>

namespace nucleo {

  // ----------------------------------------------------------------------------

  struct URI {

    std::string scheme ;

    std::string opaque ;

    // If not opaque
    std::string user, password ;
    std::string host ;
    int port ;
    std::string path ;

    std::string query ;
    std::string fragment ;
 
  public:

    typedef enum {
	 NONE = 0,
	 NORMAL = 1,
	 UNSAFE = 2,
	 RESERVED = 4
    } URIENCODING ;

    static std::string encode(const std::string &src, int flags = NORMAL) ;
    static std::string decode(const std::string &src) ;

    static bool getQueryArg(const std::string &q, const std::string &key, std::string *value=0) ;
    static bool getQueryArg(const std::string &q, const std::string &key, bool *value) ;
    static bool getQueryArg(const std::string &q, const std::string &key, int *value) ;
    static bool getQueryArg(const std::string &q, const std::string &key, unsigned int *value) ;
    static bool getQueryArg(const std::string &q, const std::string &key, unsigned long *value) ;
    static bool getQueryArg(const std::string &q, const std::string &key, double *value) ;
    static bool getQueryArg(const std::string &q, const std::string &key, float *value) ;
    static bool getQueryArg(const std::string &q, const std::string &key, TimeStamp::inttype *value) ;

    // --------------------------------------------------------------------------

    URI(void) {}
    URI(URI& src) ;
    URI(const std::string& s) { load(s) ; }

    URI& operator = (const URI& src) ;

    void clear(void) ;
    void load(const std::string &uri) ;

    std::string asString(void) const ;

    void debug(std::ostream& out) const ;

  } ;

  // ----------------------------------------------------------------------------

}

#endif
