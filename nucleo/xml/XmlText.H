#ifndef n_XmlText_H
#define n_XmlText_H

/*
 *
 * nucleo/xml/XmlText.H --
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

  namespace XmlText {

    std::string escape(std::string) ;
    std::string unescape(std::string) ;

  }

}

#endif
