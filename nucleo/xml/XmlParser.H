#ifndef n_XmlParser_H
#define n_XmlParser_H

/*
 *
 * nucleo/xml/XmlParser.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/xml/XmlStructure.H>

#include <expat.h>

#include <deque>

namespace nucleo {

  class XmlParser {

  public:

    // Nodes removed from the inbox should be detached and properly
    // deleted
    typedef std::deque<XmlStructure*> InBox ;

    typedef enum {PARSING, DONE, ERROR} status ;

  protected:

    XML_Parser parser ;
    status state ;
    XmlStructure *root, *last ;
    InBox *inbox ;

    static void tag_start(void *obj, const char *el, const char **attr) ;
    static void tag_end(void *obj, const char *el) ;
    static void cdata(void *obj, const XML_Char *s, int len) ;

    void setup(void) ;

  public:

    XmlParser(InBox *box=0) ;

    status getState(void) { return state ; }

    status parse(const char *morexml=0, unsigned int morebytes=0) ;

    std::string getErrorMessage(void) ;

    // Warning: never delete the returned node
    XmlStructure *getDocumentStructure(void) { return root ; }

    bool serializeDocument(std::ostream &out, int indent=0) ;

    // Warning: deletes the structure returned by getDocumentStructure
    void reset(void) ;

    void debug(std::ostream &out) ;

    ~XmlParser(void) ;

  } ;

}

#endif
