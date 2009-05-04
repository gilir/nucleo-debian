/*
 *
 * tests/test-TimeKeeper.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/TimeKeeper.H>

#include <iostream>
#include <cstdlib>

using namespace nucleo ;

class Reactor : public ReactiveObject {

protected:

  std::string _name ;
  int _cpt ;
  
  void react(Observable*) {
    std::cerr << _name << std::flush ;
    if (++_cpt == 4)
	 ReactiveEngine::stop() ;
    else
	 notifyObservers() ;
  }

public:

  Reactor(std::string name, Observable *other) {
    _name = name ;
    _cpt = 0 ;
    subscribeTo(other) ;
  }

} ;

int
main(int argc, char **argv) {
  char *ENGINE = getenv("NUCLEO_ENGINE") ;
  ReactiveEngine::setEngineType(ENGINE?ENGINE:"default") ;

  TimeKeeper *tk = TimeKeeper::create(2*TimeKeeper::second, true) ;

  Reactor A("A", tk) ;
  Reactor B("B", &A) ;
  Reactor C("C", &A) ;
  Reactor D("D", &B) ;
  Reactor E("E", &B) ;
  Reactor F("F", &C) ;
  Reactor G("G", &C) ;

  ReactiveEngine::run() ;

  std::cerr << std::endl << std::endl ;

  return 0 ;
}
