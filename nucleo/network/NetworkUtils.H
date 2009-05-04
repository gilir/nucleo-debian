#ifndef n_NetworkUtils_H
#define n_NetworkUtils_H

/*
 *
 * nucleo/network/NetworkUtils.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <sys/types.h>
#include <netinet/in.h>

#include <string>

namespace nucleo {

  in_addr_t resolveAddress(const char *addressOrHostname) ;

  std::string getHostName(void) ;

}

#endif
