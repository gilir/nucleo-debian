/*
 *
 * nucleo/gl/window/_extEvents.H --
 * 
 * Copyright (C) Olivier Chapuis
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

// This file is for nucleo internal use only

EVENTDEF(extFocusIn,(1 << 8))
EVENTDEF(extFocusOut,(1 << 9))
EVENTDEF(extKeyPress,(1 << 0))
EVENTDEF(extKeyRelease,(1 << 1))

EVENTDEF(extMotionNotify,(1 << 4))
EVENTDEF(extButtonPress,(1 << 2))
EVENTDEF(extButtonRelease,(1 << 3))

EVENTDEF(extStateNotify,(1 << 5))
EVENTDEF(extProximityIn,(1 << 6))
EVENTDEF(extProximityOut,(1 << 7))
