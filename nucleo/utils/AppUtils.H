#ifndef n_AppUtils_H
#define n_AppUtils_H

/*
 *
 * nucleo/utils/AppUtils.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <vector>
#include <string>

#include <stdarg.h>

namespace nucleo {

  int parseCommandLine(int argc, char** argv,
				   const char* oformat=0, const char* pformat=0, ...) ;

  bool appIsBundled(void) ;

  class DocumentOpener {
  public:
    DocumentOpener(void) ;
    virtual ~DocumentOpener(void) {}
    virtual void openDocuments(std::vector<std::string> &documents) = 0 ;
  } ;

}

#endif
