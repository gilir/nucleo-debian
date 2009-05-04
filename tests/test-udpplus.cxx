/*
 *
 * tests/test-udpplus.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/utils/FileUtils.H>

#include <nucleo/network/udp/UdpPlusSender.H>
#include <nucleo/network/udp/UdpPlusReceiver.H>

#include <unistd.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

using namespace nucleo ;

// -----------------------------------------------------------------

int
main(int argc, char **argv) {
  try {

    int PORT = 0 ;
    int firstArg = parseCommandLine(argc, argv, "p:", "i", &PORT) ;

    if (firstArg<0) {
	 std::cerr << std::endl << argv[0] ;
	 std::cerr << " receive filename" << std::endl ;
	 std::cerr << " send filename uri udpplus://host:port" << std::endl ;
	 std::cerr << std::endl << std::endl ;
	 exit(1) ;
    }

    // ------------------------------------------------------

    if (!strcmp(argv[firstArg],"send")) {
	 const char *filename = argv[firstArg+1] ;
	 const char *receiver = argv[firstArg+2] ;

	 unsigned int size = getFileSize(filename) ;
	 char *data = new char [size] ;
	 readFromFile(filename, (unsigned char *)data, size) ;
	 URI uri(receiver) ;
	 UdpPlusSender sender(uri.host.c_str(), uri.port) ;
	 std::cerr << "Sending " << size << " bytes" << std::endl ;
	 sender.send(data, size) ;
	 delete [] data ;
    } else if (!strcmp(argv[firstArg],"receive")) {
	 const char *filename = argv[firstArg+1] ;

	 UdpPlusReceiver receiver(PORT) ;
	 std::cerr << "Listening on port " << receiver.getPortNumber() << std::endl ;
	 unsigned char *data ;
	 unsigned int size ;
	 do ReactiveEngine::step() ; while (!receiver.receive(&data, &size)) ;
	 std::cerr << "Received " << size << " bytes" << std::endl ;
	 int fd = createFile(filename) ;
	 write(fd, data, size) ;
	 close(fd) ;
	 delete [] data ;
    }

    return 0 ;
  } catch (std::runtime_error e) {
    std::cerr << "runtime error: " << e.what() << std::endl ;
  } catch (...) {
    std::cerr << "Unknown exception..." << std::endl ;
  }
  return 1 ;
}
