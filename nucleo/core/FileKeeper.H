#ifndef n_FileKeeper_H
#define n_FileKeeper_H

/*
 *
 * nucleo/core/FileKeeper.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveObject.H>

namespace nucleo {

  class FileKeeper : public Observable {

  public:

    typedef enum {NOTHING=0,
			   R=1,W=2,RW=3,E=4,RE=5,WE=6,RWE=7} eventMask ;

  protected:
    
    int _fd ;
    eventMask _mask ;

    FileKeeper(void) {
	 _fd = -1 ;
	 _mask = NOTHING ;
    }

  public:

    /*
	 The FileKeeper will notify its observers everytime one of the
	 specified conditions is observed on the file descriptor.

	 Note that the state of the file descriptor might have changed
	 since the notification was sent. ReactiveObjects should always
	 check the current state using the getState() method before
	 reacting.
    */

    virtual void setup(int fd, eventMask mask) {
	 _fd = fd ;
	 _mask = mask ;
    }

    int getFd(void) { return _fd ; }
    eventMask getEventMask(void) { return _mask ; }

    eventMask getState(void) ;

    static FileKeeper* create(void) ;
    static FileKeeper* create(int fd, eventMask mask) ;

  } ;

}

#endif
