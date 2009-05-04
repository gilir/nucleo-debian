/*
 *
 * apps/videoServer/Notifier.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "Notifier.H"

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/FileKeeper.H>
#include <nucleo/core/TimeKeeper.H>
#include <nucleo/utils/FileUtils.H>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

using namespace nucleo ;

// static void ignoreSignal(int) {}

static const int PIPE_READ=0 ;
static const int PIPE_WRITE=1 ;

Notifier::Notifier(std::string path) {   
  pipe(nOut) ;
  pipe(nIn) ;
  pid = fork() ;
  if (!pid) {
    // the notifier will not use these
    close(nOut[PIPE_READ]) ;
    close(nIn[PIPE_WRITE]) ;
    // make stdin read from nIn[PIPE_READ]
    close(0) ; // stdin
    dup(nIn[PIPE_READ]) ; 
    close(nIn[PIPE_READ]) ;
    // make stdout write to nOut[PIPE_WRITE]
    close(1) ; // stdout
    dup(nOut[PIPE_WRITE]) ; 
    close(nOut[PIPE_WRITE]) ;
    // execute the notifier program
    char *const argv[] = {(char *)path.c_str(),0} ;
    execve(path.c_str(),argv,0) ; // this should never return
    std::cerr << "Unable to start the notifier. Check whether " 
		    << path << " exists and is executable" << std::endl ;
    exit(EXIT_FAILURE) ;
  } 

  // signal(SIGPIPE, ignoreSignal) ;
  close(nOut[PIPE_WRITE]) ;
  close(nIn[PIPE_READ]) ;
}

Notifier::~Notifier(void) {
  close(nOut[PIPE_READ]) ;
  close(nIn[PIPE_WRITE]) ;

  // needed to avoid zombie processes...
  kill(pid, SIGINT) ; waitpid(pid, 0, 0) ;
}

// -------------------------------------------------------------

bool
Notifier::send(const std::string &s) {
  char *ptr = (char *)s.c_str() ;
  unsigned int bytesToWrite=s.length() ;
  while (bytesToWrite) {
    int n = write(nIn[PIPE_WRITE], ptr, bytesToWrite) ;
    if (n==-1) return false ;
    bytesToWrite -= n ;
    ptr += n ;
  }
  return true ;
}

// -------------------------------------------------------------

std::string
Notifier::receive(char *pattern) {
  FileKeeper *fkp = FileKeeper::create(nOut[PIPE_READ], FileKeeper::R) ;
  TimeKeeper *tkp = TimeKeeper::create(5*TimeKeeper::second) ;
  WatchDog rintintin(fkp), rantanplan(tkp) ;

  for (;;) {
    std::string::size_type p = buffer.find(pattern) ;
    if (p!=std::string::npos) {
	 std::string result ;
	 result.assign(buffer, 0, p) ;
	 buffer.erase(0,p+1) ;
	 delete fkp ;
	 delete tkp ;
	 return result ;
    }

    ReactiveEngine::step() ;

    if (fkp->getState()&FileKeeper::R) {
	 char data[512] ;
	 int bytesread = read(nOut[PIPE_READ], data, 512) ;
	 // std::cerr << bytesread << " bytes read from the notifier" << std::endl ;
	 if (bytesread<1) {
	   delete fkp ;
	   delete tkp ;
	   throw std::runtime_error("Notifier: read failed") ;
	 }
	 buffer.append(data, bytesread) ;
    }

    if (rantanplan.sawSomething()) {
	 delete fkp ;
	 delete tkp ;
	 throw std::runtime_error("Notifier: timed out") ;
    }
  }
}
