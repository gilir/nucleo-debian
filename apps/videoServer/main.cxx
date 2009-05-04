/*
 *
 * apps/videoServer/main.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/utils/AppUtils.H>

#include "VideoServer.H"

#include <iostream>
#include <cstdlib>

int
main(int argc, char **argv) {
  char *ENGINE = getenv("NUCLEO_ENGINE") ;
  ReactiveEngine::setEngineType(ENGINE?ENGINE:"default") ;

  char *CONFIG_FILE = "videoServer.conf" ;
  if (parseCommandLine(argc, argv, "c:", "s", &CONFIG_FILE)<0) {
    std::cerr << std::endl << argv[0] ;
    std::cerr << " [-c configfile]" << std::endl << std::endl ;
    exit(1) ;
  }

  VideoServer server(CONFIG_FILE) ;
  ReactiveEngine::run() ;

  return 0 ;
}
