#ifndef n_XmlStructure_H
#define n_XmlStructure_H

/*
 *
 * nucleo/xml/XmlStructure.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <string>
#include <list>
#include <iostream>

#include <stdarg.h>

namespace nucleo {

  struct XmlStructure {

    XmlStructure *parent ;
    std::string name ;

    typedef std::pair<std::string,std::string> KeyValuePair ;
    std::list<KeyValuePair> attribs ;

    std::string cdata ;
    std::list<XmlStructure*> children ;

    XmlStructure(XmlStructure *p, const char *n, const char **attr) ;

    void serialize(std::ostream &out, int indent=0) ;

    // Extra args are (key,value) attribute pairs ending with 0. Use an
    // empty key for the cdata part:
    //    find("mechanisms","xmlns","urn:ietf:params:xml:ns:xmpp-sasl",0)
    //    find("mechanism","","DIGEST-MD5",0)
    XmlStructure* find(const char *tagname, ...) ;
    XmlStructure* find(std::string &tagname, std::list<KeyValuePair> &attribs) ;

    // Extra args are other (nth,childname) pairs ending with 0:
    //    walk(1,"mechanisms",2,"mechanism",0)
    XmlStructure* walk(int nth, const char *childname, ...) ;

    std::string getAttr(std::string key, std::string defval="") ;
    std::string operator [] (std::string key) { return getAttr(key) ; }

    void detach(void) ;

    ~XmlStructure(void) ;

  } ;

}

#endif
