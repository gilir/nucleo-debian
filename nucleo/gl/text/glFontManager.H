#ifndef n_glFontManager_H
#define n_glFontManager_H

/*
 *
 * nucleo/gl/text/glFontManager.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
*/

#include <nucleo/gl/text/glFont.H>

#include <map>

namespace nucleo {

  // ------------------------------------------------------------------

  class glFontManager {

  protected:

    typedef std::map<std::string,glFont*> FontMap ;

    FT_Library library ;
    std::string fontDirectory ;
    FontMap fonts ;

    glFontManager(void) ;
    ~glFontManager(void) ;

    static glFontManager *singleton ;

  public:

    // Font URI examples:
    //    vera:sans-serif?size=12&bold&italic
    //    file:/Users/roussel/casa/lib/fonts/TrueType/verdanaz.ttf?size=22
    
    static glFont *getFont(const char *url) ;

    static glFont *getFont(std::string url) {
	 return getFont(url.c_str()) ;
    }

    static void releaseAllFonts(void) ;
    
  } ;

  // ------------------------------------------------------------------

}

#endif
