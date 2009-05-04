/*
 *
 * tests/test-XmlParser.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/AppUtils.H>
#include <nucleo/utils/FileUtils.H>
#include <nucleo/xml/XmlParser.H>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstring>

#include <libgen.h>
#include <fcntl.h>

using namespace nucleo ;

int
main(int argc, char **argv) {
  try {
    int firstArg = parseCommandLine(argc, argv, "", "") ;
    if (firstArg<0) {
	 std::cerr << "Usage: " << basename(argv[0]) << " xml-file" << std::endl ;
	 exit(-1) ;
    }

    char *defaultxmldata = "<?xml version='1.0'?>\n<document a1=\"v1\">Preamble<section title=\"title 1\">Paragraph 1</section><section title=\"title 2\">Paragraph 2</section></document>\n<?xml version='1.0'?>\n<html>second</html>\n<?xml version='1.0'?>\n<html>third, incomplete" ;

    char *xmldata = 0 ;
    uint64_t xmlsize=0, xmloffset=0 ;
    if (firstArg!=argc) {
	   xmlsize = getFileSize(argv[firstArg]) ;
	   xmldata = new char [xmlsize] ;
	   readFromFile(argv[firstArg], (unsigned char *)xmldata, xmlsize) ;
    } else {
	 xmldata = defaultxmldata ;
	 xmlsize = strlen(xmldata) ;
    }

    XmlParser parser ;
    for (;;) {
	 if (xmloffset>=xmlsize) {
	   std::cerr << "EOF" << std::endl ;
	   break ;
	 }

	 uint64_t s = xmlsize-xmloffset ;
	 if (s>64) s = 64 ;
	 std::cerr << "Feeding the parser with " << s << " bytes (" << xmloffset << "/" << xmlsize << ")" << std::endl ;
	 XmlParser::status status = parser.parse(xmldata+xmloffset, s) ;
	 xmloffset += s ;
	 
	 // parser.debug(std::cerr) ; std::cerr << std::endl ;
	 if (status==XmlParser::ERROR) {
	   std::cerr << parser.getErrorMessage() << std::endl ;
	   break ;
	 } else if (status==XmlParser::DONE) {
	   if (!parser.serializeDocument(std::cout))
		std::cerr << "<!-- No document -->" << std::endl ;
	   parser.reset() ;
	   // parser.debug(std::cerr) ; std::cerr << std::endl ;
	 }
    }

    if (!parser.serializeDocument(std::cout))
	 std::cerr << "<!-- No document -->" << std::endl ;

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }

  return 0 ;
}
