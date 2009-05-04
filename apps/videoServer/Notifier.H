#ifndef Notifier_H
#define Notifier_H

/*
 *
 * apps/videoServer/Notifier.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <sys/types.h>

#include <string>

class Notifier {

protected:

  int nIn[2], nOut[2] ;
  pid_t pid ;
  std::string buffer ;

public:

  Notifier(std::string path) ;
  ~Notifier(void) ;

  bool send(const std::string& s) ;
  std::string receive(char *pattern) ;
  
} ;

#endif
