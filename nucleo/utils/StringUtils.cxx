/*
 *
 * nucleo/utils/StringUtils.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/utils/StringUtils.H>

#include <iostream>

namespace nucleo {

#if 0
#if defined(__GNUC__)
  template class std::basic_string<char, nucleo::ci_char_traits> ;
#endif
#endif

  void
  trimString(std::string &s, std::string badboys) {
    std::string::size_type end = s.find_last_not_of(badboys) ;
    std::string::size_type begin = s.find_first_not_of(badboys) ;
    if (end==std::string::npos && begin==std::string::npos)
	 s.clear() ;
    else {
	 if (end!=std::string::npos) s.resize(end+1) ;
	 if (begin!=std::string::npos) s.erase(0,begin) ;
    }
  }

  std::string
  extractNextWord(std::string &s) {
    std::string word = "" ;
    std::string::size_type i ;

    // Skip leading whitespaces
    i = s.find_first_not_of(" \t", 0) ;
    // std::cerr << "s=" << s << " i1=" << i << std::endl ;
    if (i>0) s.erase(0,i) ;

    i = s.find_first_of(" \t", 0) ;
    // std::cerr << "s=" << s << " i2=" << i << std::endl ;
    word.assign(s, 0, i) ;
    
    i = s.find_first_not_of(" \t", i) ;
    // std::cerr << "s=" << s << " i3=" << i << std::endl ;
    s.erase(0,i) ;

    return word ;
  }

  std::string
  findAndReplace(const std::string& source,
			  const std::string& target, const std::string& replacement) {
    std::string str = source ;
    std::string::size_type pos=0, found ;
    if (target.size () > 0) {
	 while ((found = str.find(target, pos)) != std::string::npos) {
	   str.replace(found, target.size(), replacement) ;
	   pos = found + replacement.size() ;
	 }
    }
    return str ;
  }

}
