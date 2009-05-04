/*
 *
 * tests/test-AppleIRController.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveEngine.H>

#include <nucleo/helpers/AppleIRController.H>

#include <stdexcept>
#include <iostream>

using namespace nucleo ;

class Tester : public ReactiveObject {
protected:
  AppleIRController *rc ;
  void react(Observable *obs) {
    AppleIRController::event e ;
    while (rc->getNextEvent(&e)) {
	 std::cerr << "New event from rc #" << e.control_id << ": " 
			 << e.type << " (" << e.name << ")" << std::endl ;
    }
  }
public:
  Tester(AppleIRController *c) : rc(c) {
    subscribeTo(rc) ;
  }
  ~Tester(void) {
    unsubscribeFrom(rc) ;
  }
} ;

int
main(int argc, char **argv) {
  try {
    AppleIRController control ;
    Tester tester(&control) ;
    ReactiveEngine::run() ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }
  return 0;
}
