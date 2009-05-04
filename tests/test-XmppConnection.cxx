/*
 *
 * tests/test-XmppConnection.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/AppUtils.H>
#include <nucleo/utils/FileUtils.H>
#include <nucleo/network/xmpp/XmppConnection.H>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <libgen.h>
#include <fcntl.h>

using namespace nucleo ;

int
main(int argc, char **argv) {
  try {
    std::string JID ;
    int firstArg = parseCommandLine(argc, argv, "j:", "S", &JID) ;
    if (firstArg<0) {
	 std::cerr << "Usage: " << basename(argv[0]) << " [-j jid] jid1 ... jidn" << std::endl ;
	 exit(-1) ;
    }

    XmppConnection c ;
    c.connect(JID) ;

    while (!std::cin.eof()) {
	 std::string msg ;
	 std::cin >> msg ;
	 if (msg.empty()) break ;
	 for (int i=firstArg; i<argc; ++i) c.sendMessage(argv[i], msg) ;
    }

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }

  return 0 ;
}
