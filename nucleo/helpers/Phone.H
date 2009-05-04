#ifndef n_Phone_H
#define n_Phone_H

/*
 *
 * nucleo/helpers/Phone.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <termios.h>

#include <string>

namespace nucleo {

  class Phone {

  protected:

    std::string device ;
    int fd ;
    struct termios originalTTYAttrs ;

    void debugMessage(const char *buffer, int size) ;

    void sendCommand(int fd, int timewait, const char *cmd) ;

  public:

    bool debugMode ;
    
    Phone(std::string device, bool debugMode=false) ;
    ~Phone(void) ;
    
    bool dial(std::string number) ;
    bool hangup(void) ;
    bool reset(void) ;
    
    static bool dial(std::string number, std::string device, bool debugMode=false) {
	 Phone phone(device, debugMode) ;
	 return phone.dial(number) ;
    }

  } ;

}

#endif
