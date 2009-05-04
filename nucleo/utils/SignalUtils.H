#ifndef n_SignalUtils_H
#define n_SignalUtils_H

/*
 *
 * nucleo/utils/SignalUtils.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

namespace nucleo {

  typedef void (*signalHandler) (int) ;
  void trapAllSignals(signalHandler handler) ;

}

#endif
