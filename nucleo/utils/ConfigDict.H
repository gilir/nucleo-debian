#ifndef n_ConfigDict_H
#define n_ConfigDict_H

/*
 *
 * nucleo/utils/ConfigDict.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <string>
#include <map>
#include <sstream>

namespace nucleo {

  class ConfigDict {

  private:

    typedef std::map<std::string, std::string> ConfigDictImpl ;

    std::string _configfile ;
    ConfigDictImpl _map ;

  public:

    ConfigDict(void) {}
    ConfigDict(std::string filename) { loadFrom(filename) ; }
    ConfigDict(const char *filename) { loadFrom(filename) ; }

    bool loadFrom(std::string filename) { return loadFrom(filename.c_str()) ; }
    bool loadFrom(const char *filename) ;
    std::string getFilename(void) { return _configfile ; }

    template <class X> X get(std::string key, X defval, bool setdef=false) ;

    std::string get(std::string key, const char *defval, bool setdef=false) {
	 ConfigDictImpl::iterator i = _map.find(key) ;
	 if (i==_map.end()) {
	   if (setdef) _map[key] = defval ;
	   return defval ;
	 }
	 return i->second ;
    }

    template <class X> void set(std::string key, X val) ;

    std::string dump(void) ;

  } ;

  template <class X> inline X ConfigDict::get(std::string key, X defval, bool setdef) {
    ConfigDictImpl::iterator i = _map.find(key) ;
    std::stringstream stream ;
    if (i==_map.end()) {
	 if (setdef) {
	   stream << defval ;
	   stream >> _map[key] ;
	 }
	 return defval ;
    }
    stream << i->second ;
    X result ;
    stream >> result ;
    return result ;
  }

  template <> inline bool ConfigDict::get(std::string key, bool defval, bool setdef) {
    ConfigDictImpl::iterator i = _map.find(key) ;
    if (i==_map.end()) {
	 if (setdef) _map[key] = defval?"true":"false" ;
	 return defval ;
    }
  
    if (i->second=="true" || i->second=="1") return true ;
    if (i->second=="false" || i->second=="0") return false ;
    return defval ;
  }

  template <> inline std::string ConfigDict::get(std::string key, std::string defval, bool setdef) {
    ConfigDictImpl::iterator i = _map.find(key) ;
    if (i==_map.end()) {
	 if (setdef) _map[key] = defval ;
	 return defval ;
    }
    return i->second ;
  }

  template <> inline void ConfigDict::set(std::string key, std::string value) {
    _map[key] = value ;
  }

  template <class X> inline void ConfigDict::set(std::string key, X val) {
    std::stringstream stream ;
    stream << val ;
    _map[key] = stream.str() ;
  }

}

#endif
