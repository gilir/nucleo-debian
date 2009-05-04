/*
 *
 * nucleo/xml/XmlParser.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/xml/XmlParser.H>
#include <nucleo/utils/StringUtils.H>

#include <stdexcept>
#include <iostream>
#include <sstream>

#include <assert.h>

// -----------------------------------------------------------------------

namespace nucleo {

  void
  XmlParser::tag_start(void *obj, const char *el, const char **attr) {
    XmlParser *parser = (XmlParser*)obj ;
    assert(parser) ;
    XmlStructure *newnode = new XmlStructure(parser->last, el, attr) ;
    if (parser->last)
	 parser->last->children.push_back(newnode) ;
    else
	 parser->root = newnode ;
    parser->last = newnode ;
  }

  void
  XmlParser::tag_end(void *obj, const char *el) {
    XmlParser *parser = (XmlParser*)obj ;
    assert(parser && parser->last) ;
    nucleo::trimString(parser->last->cdata) ;
    if (parser->inbox && parser->last->parent==parser->root)
	 parser->inbox->push_back(parser->last) ;
    parser->last = parser->last->parent ;
  }

  void
  XmlParser::cdata(void *obj, const XML_Char *s, int len) {
    XmlParser *parser = (XmlParser*)obj ;
    assert(parser && parser->last) ;
    std::string moredata(s, len) ;
    nucleo::trimString(moredata) ;
    if (moredata.length()) parser->last->cdata.append(moredata) ;
  }

  // -----------------------------------------------------------------------

  XmlParser::XmlParser(InBox *box) : root(0), last(0), inbox(box) {
    parser = XML_ParserCreate("UTF-8") ;
    // parser = XML_ParserCreateNS("UTF-8",'|') ;
    if (!parser)
	 throw std::runtime_error("XmlParser: couldn't create expat parser") ;
    setup() ;
  }

  void
  XmlParser::reset(void) {
    int offset, size ;
    const char *context = XML_GetInputContext(parser, &offset, &size) ;
    std::string start(context,offset,size-offset) ;
    XML_ParserReset(parser,"UTF-8") ;
    setup() ;
    parse(start.data(), start.size()) ;
  }

  void
  XmlParser::setup(void) {
    XML_SetUserData(parser, this) ;
    XML_SetElementHandler(parser, tag_start, tag_end) ;
    XML_SetCharacterDataHandler(parser, cdata) ;
    delete root ;
    if (inbox) inbox->clear() ;
    root = last = 0 ;
    state = XmlParser::PARSING ;
  }

  XmlParser::status
  XmlParser::parse(const char *morexml, unsigned int morebytes) {
    if (!morexml || !morebytes) 
	 return state ;

    if (XML_Parse(parser, morexml, morebytes, false) == XML_STATUS_OK)
	 return (state = XmlParser::PARSING) ;

    XML_Error code = XML_GetErrorCode(parser) ;
    if (code==XML_ERROR_MISPLACED_XML_PI
	   || code==XML_ERROR_JUNK_AFTER_DOC_ELEMENT
	   // || code==XML_ERROR_FINISHED
	   )
	 return (state = XmlParser::DONE) ;

    return (state = XmlParser::ERROR) ;
  }

  void
  XmlParser::debug(std::ostream &out) {
    char *statustext[] = {"PARSING", "DONE", "ERROR"} ;
    out << "[" << statustext[state] << ", root=" << root ;
    if (root) {
	 int s = root->children.size() ;
	 if (!s) out << ", no child" ;
	 else if (s==1) out << ", 1 child" ;
	 else out << ", " << s << " children" ;
    }
    out << "]" ;
  }

  std::string
  XmlParser::getErrorMessage(void) {
    std::stringstream result ;
    result << "XmlParser: " << XML_ErrorString(XML_GetErrorCode(parser)) 
		 << " (line " << XML_GetCurrentLineNumber(parser)
		 << ", column " << XML_GetCurrentColumnNumber(parser) ;
    int offset, size ;
    const char *context = XML_GetInputContext(parser, &offset, &size) ;
    if (context) {
	 int nbc = size-offset ;
	 if (nbc>10) nbc = 10 ;
	 std::string ctx = findAndReplace(std::string(context,offset,nbc),
							    "\n","\\n") ;
	 result << ", '" << ctx << "'" ;
    }
    result << ")" ;
    return  result.str() ;
  }

  bool
  XmlParser::serializeDocument(std::ostream &out, int indent) {
    if (!root) return false ;
    root->serialize(out, indent) ;
    return true ;
  }

  XmlParser::~XmlParser(void) {
    delete root ;
    XML_ParserFree(parser) ;
  }

}
