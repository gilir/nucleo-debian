/*
 *
 * nucleo/core/carbon/cPoolTrick.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#import <Foundation/NSAutoreleasePool.h>

static NSAutoreleasePool *thePool = 0 ;

void doThePoolTrick(void) {
  if (!thePool) thePool = [[NSAutoreleasePool alloc] init] ;
  // FIXME: at some point later, we should do: [thePool release] ;
}
