/*
 *
 * nucleo/gl/window/_events.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

// This file is for nucleo internal use only

// window
EVENTDEF(configure,(1 << 0))
EVENTDEF(expose,(1 << 1))
EVENTDEF(destroy,(1 << 2))

// mouse
EVENTDEF(pointerMotion,(1 << 3))
EVENTDEF(enter,(1 << 4))
EVENTDEF(leave,(1 << 5))
EVENTDEF(buttonPress,(1 << 6))
EVENTDEF(buttonRelease,(1 << 7))
EVENTDEF(wheelMotion,(1 << 8))

// keyboard
EVENTDEF(focusIn,(1 << 9))
EVENTDEF(focusOut,(1 << 10))
EVENTDEF(keyPress,(1 << 11))
EVENTDEF(keyRelease,(1 << 12))

// other input devices
EVENTDEF(extensionEvent,(1 << 13))
