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

#include <nucleo/gl/text/glFontManager.H>

#include <nucleo/config.H>
#include <nucleo/core/URI.H>
#include <nucleo/nucleo.H>

#include <stdexcept>

namespace nucleo {

  glFontManager *glFontManager::singleton = 0 ;

  // ------------------------------------------------------------------------

  glFontManager::glFontManager(void) {
    if (FT_Init_FreeType(&library))
	 throw std::runtime_error("FreeType2 error: FT_Init_FreeType failed") ;
    fontDirectory = getNucleoResourcesDirectory() + "/fonts" ;
  }

  glFont *
  glFontManager::getFont(const char *uri) {
    if (!singleton) singleton = new glFontManager() ;

    FontMap::iterator iFont = singleton->fonts.find(uri) ;
    if (iFont!=singleton->fonts.end()) {
	 // std::cerr << "Found " << uri << " in the cache" << std::endl ;
	 return (*iFont).second ;
    }

     URI uFont(uri) ;

    std::string scheme = uFont.scheme ;
    std::string query = uFont.query ;

    std::string filename ;

    unsigned int pixelSize = 12 ;
    URI::getQueryArg(query,"size",&pixelSize) ;

    if (scheme=="file")
	 filename = uFont.opaque!="" ? uFont.opaque : uFont.path ;
    else if (scheme=="vera") {
	 filename = singleton->fontDirectory ;
	 if (filename.size() && filename[filename.size()-1]!='/')
	   filename = filename+"/" ;
	 filename = filename + "Vera" ;
	 std::string type = uFont.opaque ;

	 bool bold = URI::getQueryArg(query,"bold") ;
	 bool italic = URI::getQueryArg(query,"italic") ;
	 // std::cerr << "type=" << type << " bold=" << bold << " italic=" << italic << std::endl ;
	 if (type=="serif") {
	   filename = filename + "Se" ;
	   if (bold) filename = filename + "Bd" ;
	 } else if (type=="sans-serif") {
	   if (bold && italic) filename = filename + "BI" ;
	   else if (bold) filename = filename + "Bd" ;
	   else if (italic) filename = filename + "It" ;
	 } else if (type=="monospace") {
	   filename = filename + "Mo" ;
	   if (bold && italic) filename = filename + "BI" ;
	   else if (bold) filename = filename + "Bd" ;
	   else if (italic) filename = filename + "It" ;
	   else filename = filename + "no" ;
	 }
	 filename = filename + ".ttf" ;
    }

    FT_Face face ;
    FT_Error error = FT_New_Face(singleton->library, filename.c_str(), 0, &face) ;
    if (error == FT_Err_Unknown_File_Format) {
	 // the font file could be opened and read, but it appears that
	 // its font format is unsupported
	 throw std::runtime_error("FreeType2 error: unsupported font format "+filename) ;
    } else if ( error ) {
	 // another error code means that the font file could not be
	 // opened or read, or simply that it is broken
	 throw std::runtime_error("FreeType2 error: incorrect font file "+filename) ;
    }

    glFont *result = new glFont(face, pixelSize) ;
    // std::cerr << "Adding " << uri << " to the cache" << std::endl ;
    singleton->fonts[uri] = result ;
    return result ;
  }

  glFontManager::~glFontManager(void) {
    for (FontMap::iterator i=fonts.begin(); i!=fonts.end(); ++i)
	 delete (*i).second ;
    fonts.clear() ;
    FT_Done_FreeType(library) ;
  }

  void
  glFontManager::releaseAllFonts(void) {
    delete singleton ;
    singleton = 0 ;
  }

  // ------------------------------------------------------------------------
 
}
