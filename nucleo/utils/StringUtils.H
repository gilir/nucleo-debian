#ifndef n_StringUtils_H
#define n_StringUtils_H

/*
 *
 * nucleo/utils/StringUtils.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <cstring>
#include <ctype.h>

#include <string>

namespace nucleo {

  // ------------------------------------------------------------------
  // Case insensitive strings
  
#if defined(__GNUC__) && __GNUC__<3
  struct ci_char_traits : public std::string_char_traits<char>
#else
  struct ci_char_traits : public std::char_traits<char>
#endif
  {

    static bool eq(char c1, char c2) {
	 return toupper(c1) == toupper(c2) ;
    }

    static bool ne(char c1, char c2) {
	 return toupper(c1) != toupper(c2) ;
    }

    static bool lt(char c1, char c2) {
	 return toupper(c1) <  toupper(c2) ;
    }

    static int compare(const char* s1, const char* s2, size_t n) {
	 return strncasecmp( s1, s2, n ) ;
    }

    static const char* find(const char* s, int n, char a) {
	 while( n-- > 0 && toupper(*s)!=toupper(a) ) ++s;
	 return s;
    }
    
  } ;

  typedef std::basic_string<char, ci_char_traits> cistring ;

  // ------------------------------------------------------------------
  // std::string utils

  void trimString(std::string &s, std::string badboys=" \t\n\r") ;
  std::string extractNextWord(std::string &s) ;


  std::string findAndReplace(const std::string& source,
					    const std::string& target, const std::string& replacement) ;

}

#endif
