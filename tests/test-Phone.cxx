/*
 *
 * tests/test-Phone.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/AppUtils.H>
#include <nucleo/helpers/Phone.H>

#include <stdexcept>
#include <iostream>
#include <cstdlib>

using namespace nucleo ;

int
main(int argc, char **argv) {
  try {
    std::string DEVICE = "/dev/cu.T610NR-SerialPort1-1" ;
    bool DEBUGMODE = false ;
    int firstArg = parseCommandLine(argc, argv, "d:D", "Sb", &DEVICE, &DEBUGMODE) ;
    if (firstArg<argc-1) {
	 std::cerr << std::endl << argv[0] << " [-d device] [-D(ebug)] number" << std::endl ;
	 exit(1) ;
    }

    Phone phone(DEVICE,DEBUGMODE) ;
    if (phone.dial(argv[firstArg]))
	 std::cerr << "Succeeded!" << std::endl ;
    else
	 std::cerr << "Failed!" << std::endl ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }
  return 0;
}
