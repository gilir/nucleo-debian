/*
 *
 * nucleo/xml/XmlStructure.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/xml/XmlStructure.H>

namespace nucleo {

  XmlStructure::XmlStructure(XmlStructure *p, const char *n, const char **attr) {
    parent = p ;
    name = n ;
    for (int i=0; attr[i]; i+=2) 
	 attribs.push_back(KeyValuePair(attr[i],attr[i+1])) ;
  }

  void
  XmlStructure::serialize(std::ostream &out, int indent) {
    std::string padding ;
    for (int i=0; i<indent; ++i) padding = padding + "  " ;
    out << padding << "<" << name ;
    // out << " this='" << this << "'" ;
    for (std::list<KeyValuePair>::const_iterator i=attribs.begin();
	    i!=attribs.end(); ++i)
	 out << " " << (*i).first << "='" << (*i).second << "'" ;

    bool empty = children.size()==0 && cdata.length()==0 ;

    if (empty) {
	 out << "/>" << std::endl ;
    } else {
	 out << ">" ;
	 if (children.size()) {
	   if (cdata.length()) out << std::endl << padding+"  " << cdata ;
	   out << std::endl ;
	   for (std::list<XmlStructure*>::const_iterator i=children.begin();  
		   i!=children.end(); ++i)
		(*i)->serialize(out, indent+1) ;
	   out << padding ;
	 } else if (cdata.length()) 
	   out << cdata ;
	 out << "</" << name << ">" << std::endl ;
    }
  }

  XmlStructure*
  XmlStructure::find(const char *q_ctagname, ...) {
    std::string q_tagname = q_ctagname ;
    std::list<KeyValuePair> q_attribs ;
    va_list ap ;
    va_start(ap,q_ctagname) ;
    for (char *key=va_arg(ap, char*); key; key=va_arg(ap, char*))
	 q_attribs.push_back(KeyValuePair(key, va_arg(ap, char*))) ;
    va_end(ap) ;
    return find(q_tagname, q_attribs) ;
  }

  XmlStructure*
  XmlStructure::find(std::string &q_tagname, std::list<KeyValuePair> &q_attribs) {
    if (name==q_tagname) {
	 bool found = true ;
	 for (std::list<KeyValuePair>::const_iterator i=q_attribs.begin();
		 found && i!=q_attribs.end(); ++i) {
	   std::string key = (*i).first ;
	   std::string thisone = key=="" ? cdata : getAttr(key) ;
	   if (thisone!=(*i).second) found = false ;
	 }
	 if (found) return this ;
    }

    for (std::list<XmlStructure*>::iterator i=children.begin();
	    i!=children.end(); ++i) {
	 XmlStructure *node = (*i)->find(q_tagname, q_attribs) ;
	 if (node) return node ;
    }

    return 0 ;
  }

  XmlStructure*
  XmlStructure::walk(int nth, const char *cchildname, ...) {
    XmlStructure *current = this ;
    std::string childname = cchildname ;
    va_list ap ;
    va_start(ap,cchildname) ;
    while (nth) {
	 XmlStructure *child = 0 ;
	 for (std::list<XmlStructure*>::iterator i=current->children.begin();
		 i!=current->children.end(); ++i) {
	   if ((*i)->name==childname) nth-- ;
	   if (!nth) { child = *i ; break ; }
	 }
	 if (!child) { va_end(ap) ; return 0 ; }
	 current = child ;
	 nth = va_arg(ap, int) ;
	 if (nth) childname = va_arg(ap, const char*) ;
    }
    va_end(ap) ;
    return current ;
  }

  std::string
  XmlStructure::getAttr(std::string key, std::string defval) {
    for (std::list<KeyValuePair>::iterator i=attribs.begin();
	    i!=attribs.end(); ++i)
	 if ((*i).first==key) return (*i).second ;
    return defval ;
  }

  void
  XmlStructure::detach(void) {
    if (parent) {
	 parent->children.remove(this) ;
	 parent = 0 ;
    }
  }

  XmlStructure::~XmlStructure(void) {
    if (parent) parent->children.remove(this) ;
    while (! children.empty()) {
	 XmlStructure *child = children.front() ;
	 children.pop_front() ;
	 delete child ;
    }
  }

}
