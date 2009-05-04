/*
 *
 * nucleo/xml/XmlText.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/xml/XmlText.H>
#include <nucleo/utils/StringUtils.H>

namespace nucleo {

  namespace XmlText {

    std::string
    escape(std::string s) {
	 s = findAndReplace(s, "&", "&amp;") ;
	 s = findAndReplace(s, "\"", "&quot;") ;
	 s = findAndReplace(s, "'", "&apos;") ;
	 s = findAndReplace(s, "<", "&lt;") ;
	 s = findAndReplace(s, ">", "&gt;") ;
	 return s ;
    }

    std::string
    unescape(std::string s) {
	 s = findAndReplace(s, "&quot;", "\"") ;
	 s = findAndReplace(s, "&apos;", "'") ;
	 s = findAndReplace(s, "&lt;", "<") ;
	 s = findAndReplace(s, "&gt;", ">") ;
	 s = findAndReplace(s, "&amp;", "&") ;
	 return s ;
    }

  }

}
