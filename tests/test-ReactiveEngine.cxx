/*
 *
 * tests/test-ReactiveEngine.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/FileKeeper.H>
#include <nucleo/core/TimeKeeper.H>

#include <iostream>
#include <cstdlib>

using namespace nucleo ;

void
testTimeKeeper(void) {
  std::cerr << "TimeKeeper" << std::endl ;

  TimeKeeper *timer = TimeKeeper::create() ;
  const char *stateNames[] = {"DISARMED","ARMED","TRIGGERED","TRIGGERED_AND_ARMED"} ;

  ReactiveEngine::step(1*TimeKeeper::second) ;

  timer->arm(2*TimeKeeper::second) ;
  std::cerr << "   timer but no timeout (timer should trigger): " << std::flush ;
  ReactiveEngine::step() ;
  std::cerr << stateNames[timer->getState()] << std::endl ;

  ReactiveEngine::step(1*TimeKeeper::second) ;

  timer->arm(2*TimeKeeper::second) ;
  std::cerr << "   timeout>timer (timer should trigger again): " << std::flush ;
  ReactiveEngine::step(4*TimeKeeper::second) ;
  std::cerr << stateNames[timer->getState()] << std::endl ;

  ReactiveEngine::step(1*TimeKeeper::second) ;

  timer->arm(4*TimeKeeper::second) ;
  std::cerr << "   timeout<timer: " << std::flush ;
  ReactiveEngine::step(2*TimeKeeper::second) ;
  std::cerr << stateNames[timer->getState()] << std::endl ;
}


void
testFileKeeper(void) {
  std::cerr << "FileKeeper" << std::endl ;

  FileKeeper *file = FileKeeper::create(0, FileKeeper::R) ;
  const char *stateNames[] = {"NOTHING","R","W","RW","E","RE","WE","RWE"} ;

  char tmp[1024] ;

  ReactiveEngine::step(1*TimeKeeper::second) ;

  std::cerr << "   no timeout: " << std::flush ;
  ReactiveEngine::step() ;
  std::cerr << "   " << stateNames[file->getState()] ;
  std::cin >> tmp ; 
  std::cerr << " --> '" << tmp << "'" << std::endl ;

  ReactiveEngine::step(1*TimeKeeper::second) ;

  std::cerr << "   4 seconds timeout: " << std::flush ;
  ReactiveEngine::step(4*TimeKeeper::second) ;
  if (file->getState()&FileKeeper::R) {
    std::cerr << "   " << stateNames[file->getState()] ;
    std::cin >> tmp ; 
    std::cerr << " --> '" << tmp << "'" << std::endl ;
  } else
    std::cerr << stateNames[file->getState()] << " timeout?" << std::endl ;

  delete file ;
}

int
main(int argc, char **argv) {
  char *ENGINE = getenv("NUCLEO_ENGINE") ;
  ReactiveEngine::setEngineType(ENGINE?ENGINE:"default") ;

  testFileKeeper() ;
  testTimeKeeper() ;

  return 0 ;
}
