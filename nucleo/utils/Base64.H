#ifndef n_Base64_H
#define n_Base64_H

/*
 *
 * nucleo/utils/Base64.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <string>

namespace nucleo {

  namespace Base64 {
    std::string encode(std::string input) ;
    std::string decode(std::string input) ;
  }

}

#endif
