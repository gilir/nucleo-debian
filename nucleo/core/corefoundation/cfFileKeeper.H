#ifndef n_cfFileKeeper_H
#define n_cfFileKeeper_H

/*
 *
 * nucleo/core/corefoundation/cfFileKeeper.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/FileKeeper.H>

#include <nucleo/core/carbon/cReactiveEngine.H>

#include <CoreFoundation/CoreFoundation.h>

namespace nucleo {

  // ---------------------------------------------------------------

  class cfFileKeeper : public FileKeeper {

  protected:

    cReactiveEngine *cEngine ;

    CFRunLoopRef _runLoop ;
    CFSocketRef _socket ;
    CFRunLoopSourceRef _runLoopSource ;

    void _cleanup(void) ;
    static void _callback(CFSocketRef s,
					 CFSocketCallBackType type,
					 CFDataRef address, const void *data, void *info) ;

  public:

    cfFileKeeper(cReactiveEngine *cre=0) ;
    ~cfFileKeeper(void) ;

    void setup(int fd, eventMask mask) ;

  } ;

  // ---------------------------------------------------------------

}

#endif
