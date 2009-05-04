/*
 *
 * nucleo/utils/ConfigDict.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>
#include <nucleo/utils/ConfigDict.H>
#include <nucleo/utils/StringUtils.H>
#include <nucleo/utils/FileUtils.H>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

namespace nucleo {

  // -------------------------------------------------------------------------

  bool
  ConfigDict::loadFrom(const char *filename) {
    _map.clear() ;

    if (!filename) {
	 _configfile = "" ;
	 return false ;
    }

    _configfile = filename ;

    unsigned int size = 0 ;
    try {
	 size = getFileSize(filename) ;
    } catch (...) {
	 std::cerr << "ConfigDict: Unable to read configuration from " << filename << std::endl ;
	 return false ;
    }

    char *ptr = new char [size+1] ;
    int fd = open(filename, O_RDONLY) ;
    read(fd, ptr, size) ;
    ptr[size] = '\0' ;
    close(fd) ;
    std::string configuration = ptr ;
    delete [] ptr ;

    std::string::size_type blocklen = configuration.length() ;
    std::string::size_type pos=0 ;
    for (bool loop=true; loop; ) {
	 // --- Get one line --------------------------------------
	 std::string line ;
	 std::string::size_type end_of_line = configuration.find("\n", pos) ;
	 if (end_of_line==std::string::npos) {
	   line.assign(configuration, pos, blocklen-pos) ;
	   loop = false ;
	 } else {
	   line.assign(configuration, pos, end_of_line-pos) ;
	   pos = end_of_line+1 ;
	 }
	 if (line.find("#") == 0) continue ;
	 // --- Add key/value to the hash map ---------------------
	 std::string::size_type psep = line.find(":") ;
	 if (psep!=std::string::npos) {
	   std::string key, value ;
	   key.assign(line.c_str(),0,psep) ;
	   trimString(key) ;
	   value.assign(line,psep+1,line.length()-psep) ;
	   trimString(value) ;
	   char *k = new char [key.length()+1] ;
	   strcpy(k, key.c_str()) ;
	   char *v = new char [value.length()+1] ;
	   strcpy(v, value.c_str()) ;
	   //std::cerr << "Adding (" << k << ":" << v << ")" << std::endl ;
	   _map[k] = v ;
	 }
    }

    return true ;
  }

  // -------------------------------------------------------------------------

  std::string
  ConfigDict::dump(void) {
    std::stringstream out ;
    if (_configfile!="") out << "# Source: " << _configfile << std::endl << std::endl ;
    if (!_map.size())
	 out << "# Empty configuration" << std::endl ;
    else {
	 for (ConfigDictImpl::iterator i=_map.begin(); i!=_map.end(); ++i)
	   out << (*i).first << ": " << (*i).second << std::endl ;
    }
    return out.str() ;
  }

  // -------------------------------------------------------------------------

}
