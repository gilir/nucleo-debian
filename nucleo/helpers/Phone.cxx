/*
 *
 * nucleo/helpers/Phone.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/helpers/Phone.H>
#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/TimeKeeper.H>
#include <nucleo/core/FileKeeper.H>
#include <nucleo/utils/StringUtils.H>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <iostream>
#include <sstream>

using namespace nucleo ;

namespace nucleo {

  Phone::Phone(std::string d, bool dm) {
    debugMode = dm ;
    device = d ;

    if (debugMode) std::cerr << "Phone: opening " << device << std::endl ;

    fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK) ;
    if (fd==-1) {
	 std::cerr << "Phone: error opening " << device << " - " << strerror(errno) << " (" << errno << ")." << std::endl ;
	 return ;
    }

    if (debugMode) std::cerr << "Phone: configuring the device" << std::endl ;

    if (ioctl(fd, TIOCEXCL) == -1) {
	 std::cerr << "Phone: error setting TIOCEXCL on " << device << " - " << strerror(errno) << " (" << errno << ")." << std::endl ;
	 close(fd) ;
	 fd = -1 ;
	 return ;
    }

    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
	 std::cerr << "Phone: error reading TTY settings from " << device << " - " << strerror(errno) << " (" << errno << ")." << std::endl ;
	 close(fd) ;
	 fd = -1 ;
	 return ;
    }
    flags &= ~O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) {
	 std::cerr << "Phone: error setting TTY settings on " << device << " - " << strerror(errno) << " (" << errno << ")." << std::endl ;
	 close(fd) ;
	 fd = -1 ;
    }

    if (tcgetattr(fd, &originalTTYAttrs) == -1) {
	 std::cerr << "Phone: error getting tty attributes from " << device << " - " << strerror(errno) << " (" << errno << ")." << std::endl ;
	 close(fd) ;
	 fd = -1 ;
	 return ;
    }

    struct termios newtio ;
    tcgetattr(fd, &newtio);
    newtio.c_cflag |= 	B9600 | CS8 | CLOCAL | CREAD;
    newtio.c_iflag |= 	IGNPAR;
    newtio.c_oflag = 	0;
    newtio.c_lflag = 	0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 	0;
    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
	 std::cerr << "Phone: error setting tty attributes on " << device << " - " << strerror(errno) << " (" << errno << ")." << std::endl ;
	 close(fd) ;
	 fd = -1 ;
	 return ;
    }
  }

  void
  Phone::sendCommand(int fd, int milliseconds, const char *cmd) {
    if (fd==-1) return ;

    tcflush(fd, TCIOFLUSH) ;
    write(fd, cmd, strlen(cmd)) ;
    tcdrain(fd) ;
    ReactiveEngine::sleep(milliseconds*1000) ;
    return ;
  }

  void
  Phone::debugMessage(const char *buffer, int size) {
    std::cerr << "--> " ;
    for (int i=0; i<size; ++i)
	 if (buffer[i]=='\r') fprintf(stderr,"\\r") ;
	 else if (buffer[i]=='\n') fprintf(stderr,"\\n") ;
	 else fprintf(stderr,"%c",buffer[i]) ;
    std::cerr << std::endl ;
  }

  bool
  Phone::dial(std::string number) {
    if (fd==-1) return false ;

    if (!hangup()) {
	 std::cerr << "Phone: " << device << " is not responding" << std::endl ;
	 return false ;
    }
    
	if (debugMode) std::cerr << "Phone: dialing " << number << std::endl ;

	std::string command = "ATD"+number+";H\r" ;
	sendCommand(fd, 2, command.c_str()) ;

	char out[1024];
	TimeKeeper *timer = TimeKeeper::create(5*TimeKeeper::second) ;
	FileKeeper *reader = FileKeeper::create(fd, FileKeeper::R) ;
	bool result = false ;
	for (;;) {
	  ReactiveEngine::step() ;
	  if (reader->getState()&FileKeeper::R) {
	    bzero(out, sizeof(out));
	    int nbbytes = read(fd, out, sizeof(out)-1) ;
	    if (debugMode) {
		 std::string msg(out, nbbytes) ;
		 msg = findAndReplace(msg, "\r", "\\r") ;
		 msg = findAndReplace(msg, "\n", "\\n") ;
		 std::cerr << "Phone: received " << nbbytes << " bytes (" << msg << ")" << std::endl ;
	    }
	    if (strstr(out, "OK") != NULL) {
		 result = true ;
		 break ;
	    }
	  }
	  if (timer->getState()&TimeKeeper::TRIGGERED) {
	    if (debugMode) std::cerr << "Phone: timed out" << std::endl ;
	    break ;
	  }
	}									

	delete timer ;
	delete reader ;
	return result ;
  }

  bool
  Phone::hangup(void) {
    if (fd==-1) return false ;

    if (debugMode) std::cerr << "Phone: hanging up" << std::endl ;

    sendCommand(fd, 1, "ATH0\r");

    char 	out[1024];
    bzero(out, sizeof(out));
    read(fd, out, sizeof(out)-1);
    if (strstr(out, "OK") == NULL) return false ;
    return true ;
  }

  bool
  Phone::reset(void) {
    if (fd==-1) return false ;

    if (debugMode) std::cerr << "Phone: resetting" << std::endl ;

    sendCommand(fd, 1, "+++ATH0\r");
    sendCommand(fd, 1, "ATZ\r");

    char 	out[1024];
    bzero(out, sizeof(out));
    read(fd, out, sizeof(out)-1);
    if (strstr(out, "OK") == NULL) return false ;
    return true ;
  }

  Phone::~Phone(void) {
    if (fd==-1) return ;
    
    tcsetattr(fd, TCSANOW, &originalTTYAttrs);
    close(fd);
    fd = -1 ;
  }
 
}
