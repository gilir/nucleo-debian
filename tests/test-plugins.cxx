/*
 *
 * tests/test-plugins.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/AppUtils.H>
#include <nucleo/core/PluginManager.H>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

using namespace nucleo ;

int
main(int argc, char **argv) {
  std::cerr << "Warning: make sure you did a \"make install\" before you run this test program..." << std::endl << std::endl ;

  try {
    char *SERVICE = "ImageSource::create" ;
    char *TAG = "*" ;
    if (parseCommandLine(argc, argv, "s:t:", "ss", &SERVICE,&TAG)<0) {
	 std::cerr << std::endl << argv[0] << " [-s service] [-t tag]" << std::endl ;
	 exit(1) ;
    }

    std::cerr << "service='" << SERVICE << "' tag='" << TAG << "'" << std::endl ;
    
    void *ptr = PluginManager::getSymbol(SERVICE, TAG) ;
    std::cerr << "ptr = " << ptr << std::endl ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (...) {
    std::cerr << "Unknown exception..." << std::endl ;
  }
}
