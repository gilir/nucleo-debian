/*
 *
 * nucleo/utils/SignalUtils.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>
#include <nucleo/utils/SignalUtils.H>

#include <signal.h>

namespace nucleo {

  void
  trapAllSignals(signalHandler handler) {
    signal(SIGHUP, handler) ;
    signal(SIGINT, handler) ;
    signal(SIGKILL, handler) ;
    signal(SIGALRM, handler) ;
    signal(SIGTERM, handler) ;
    signal(SIGUSR1, handler) ;
    signal(SIGUSR2, handler) ;
    signal(SIGPIPE, SIG_IGN) ;
  }

}
