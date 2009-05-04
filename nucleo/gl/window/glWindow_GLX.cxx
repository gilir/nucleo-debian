/*
 *
 * nucleo/gl/window/glWindow_GLX.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/gl/window/glWindow_GLX.H>

#if HAVE_X11_XM_MWMUTIL_H
#  include <X11/Xm/MwmUtil.h>
#elif HAVE_XM_MWMUTIL_H
#  include <Xm/MwmUtil.h>
#endif

#include <X11/cursorfont.h>
#include <X11/Xatom.h>

#ifdef HAVE_XINPUT
#include <X11/extensions/XInput.h>
#endif

#include <iostream>
#include <stdexcept>
#include <cstring>

#define DEBUG_LEVEL 0

namespace nucleo {

  // -----------------------------------------------------------------------

  glWindow
  *createGLXwindow(long options, long eventmask) {
    return new glWindow_GLX(options, eventmask) ;
  }

  // ------------------------------------------------------------------

  unsigned int
  glWindow_GLX::getScreenWidth() {
    return DisplayWidth(_xDisplay,DefaultScreen(_xDisplay)) ;
  }

  unsigned int
  glWindow_GLX::getScreenHeight() {
    return DisplayHeight(_xDisplay,DefaultScreen(_xDisplay)) ;
  }

  // ------------------------------------------------------------------

  glWindow_GLX::glWindow_GLX(Display *disp, Window parent,
					    long options, long eventmask) {
    debugEvents = false ;
    debugExtInput = false ;
    _origCorePointerName = 0;
    _corePointerHasChanged = false;

    _xDisplay = disp ;
    _xParent = parent ;
    setup(options, eventmask) ;

    // So that we can notify the observers after the constructor has
    // returned...
    selfNotify() ;
  }

  // ------------------------------------------------------------------

  glWindow_GLX::glWindow_GLX(long options, long eventmask) {
    debugEvents = false ;
    debugExtInput = false ;
    _xDisplay = XOpenDisplay(0) ;
    _xParent = 0 ;
    _origCorePointerName = 0;
    _corePointerHasChanged = false;

    if (!_xDisplay) throw std::runtime_error("glWindow_GLX: can't open display") ;
    setup(options, eventmask) ;

    // So that we can notify the observers after the constructor has
    // returned...
    selfNotify() ;
  }

  // ------------------------------------------------------------------

  void
  glWindow_GLX::setup(long options, long eventmask) {
    _mapped = false ;

    _fk = FileKeeper::create(ConnectionNumber(_xDisplay), FileKeeper::R);
    subscribeTo(_fk) ;

    XVisualInfo *vi = 0 ;
    XSetWindowAttributes swa ;
    int attributeList[] = {
	 GLX_RGBA,
	 GLX_RED_SIZE, 8,
	 GLX_GREEN_SIZE, 8,
	 GLX_BLUE_SIZE, 8,
	 (int)None,
	 (int)None,
	 (int)None,
	 (int)None,
	 (int)None,
	 (int)None,
	 (int)None,
	 (int)None,
	 (int)None,
	 (int)None,
	 (int)None,
	 (int)None,
	 (int)None,
	 (int)None
    } ;
    int attributeListSize = 7 ;

    if (options&glWindow::DOUBLE_BUFFER) attributeList[attributeListSize++] = GLX_DOUBLEBUFFER ;

    int istencil = 0;
    int sstencil = 0;
    if (options&glWindow::STENCIL) {
	 attributeList[attributeListSize++] = GLX_STENCIL_SIZE ;
	 istencil = attributeListSize;
	 sstencil = attributeList[attributeListSize++] = 8 ;
    }

    int idepth=0 ;
    int ddepth = 0;
    if (options&glWindow::DEPTH) {
	 attributeList[attributeListSize++] = GLX_DEPTH_SIZE ;
	 idepth = attributeListSize ;
	 // was 1 (changed 25 apr 2002)
	 // 1 is tried if glXChooseVisual fail (2004-02-22)
	 ddepth = attributeList[attributeListSize++] = 24 ;
    }

    int depth, adepth=0, sizeBeforeAlpha=attributeListSize ;

    if (options&glWindow::ALPHA) {
	 attributeListSize+=2 ;
	 adepth=8 ;
    }

    do {
      do {
	do {

	  if (options&glWindow::ALPHA) {
	    attributeList[sizeBeforeAlpha] = GLX_ALPHA_SIZE ;
	    adepth = attributeList[sizeBeforeAlpha+1] = adepth ;
	  }

	  for (depth=8; depth; depth--) {
	    attributeList[2] = attributeList[4] = attributeList[6] = depth ;
	    vi = glXChooseVisual(_xDisplay, DefaultScreen(_xDisplay), attributeList);
	    if (vi)
	      break ;
	  }
	  if (vi) break ;

	  adepth-- ;

	} while (adepth>=0) ;

	if (vi) break ;

	if ((options&glWindow::DEPTH) && ddepth == 24) {
	  attributeList[idepth] = 1 ;
	  ddepth = 1;
	} else { ddepth = 0; }

      } while (ddepth) ;

      if (vi) break ;

      if ((options&glWindow::STENCIL) && sstencil == 8) {
	  attributeList[istencil] = 0 ;
	  sstencil = 1;
	} else { sstencil = 0; }

    } while(sstencil);

    if (!vi) {
      std::cerr << "glWindow_GLX : No suitable visual" << std::endl ;
      exit(1) ;
    }

    if ((options&glWindow::STENCIL) && sstencil == 1) {
      std::cerr << "glWindow_GLX Warnning: No suitable visual with a STENCIL buffer"
		<< std::endl ;
    }

#if DEBUG_LEVEL>=2
    std::cerr << "glWindow_GLX: rgb-depth=" << depth  ;
    if (options&glWindow::DEPTH) std::cerr << " depth=" << attributeList[idepth] ;
    std::cerr << " alpha=" << adepth ;
    std::cerr << " visual=0x" << hex << vi->visualid << dec << " (compare with 'glxinfo -b')" << std::endl ;
#else
#  if DEBUG_LEVEL>=1
    if ((options&glWindow::ALPHA) && !adepth) std::cerr << "glWindow_GLX warning: no alpha plane available ! (depth=" << depth << ")" << std::endl ;
#  endif
#endif

    _glContext = glXCreateContext(_xDisplay, vi, 0, GL_TRUE);

    if (!_xParent) _xParent = RootWindow(_xDisplay, vi->screen) ;
    int x=0, y=0 ;
    unsigned int w=1, h=1 ;
    unsigned long valuemask = 0 ;

    // swa.override_redirect = False ;       valuemask |= CWOverrideRedirect ;
    swa.backing_store = NotUseful ;       valuemask |= CWBackingStore ;
    swa.background_pixel = 0xFFFFFFFF ;   valuemask |= CWBackPixel ;
    swa.save_under = False ;              valuemask |= CWSaveUnder ;
    swa.border_pixel = 0xFFFFFFFF ;       valuemask |= CWBorderPixel ;
    
    swa.colormap = XCreateColormap(_xDisplay, RootWindow(_xDisplay, vi->screen), vi->visual, AllocNone);
    valuemask |= CWColormap ;

    swa.event_mask = 0 ;
    if (eventmask & glWindow::event::configure) swa.event_mask |= StructureNotifyMask ;
    if (eventmask & glWindow::event::expose) swa.event_mask |= ExposureMask ;
    if (eventmask & glWindow::event::destroy) swa.event_mask |= StructureNotifyMask ;
    if (eventmask & glWindow::event::enter) swa.event_mask |= EnterWindowMask ;
    if (eventmask & glWindow::event::leave) swa.event_mask |= LeaveWindowMask ;
    if (eventmask & glWindow::event::keyPress) swa.event_mask |= KeyPressMask ;
    if (eventmask & glWindow::event::keyRelease) swa.event_mask |= KeyReleaseMask ;
    if (eventmask & glWindow::event::buttonPress) swa.event_mask |= ButtonPressMask ;
    if (eventmask & glWindow::event::buttonRelease) swa.event_mask |= ButtonReleaseMask ;
    if (eventmask & glWindow::event::pointerMotion) swa.event_mask |= PointerMotionMask ;
    if ((eventmask & glWindow::event::focusIn) || (eventmask & glWindow::event::focusOut)) swa.event_mask |= FocusChangeMask ;
    valuemask |= CWEventMask ;

    _xWindow = XCreateWindow(_xDisplay, _xParent,
					    x, y, w, h,
					    0, vi->depth, InputOutput, vi->visual,
					    valuemask, &swa);

    if (eventmask & glWindow::event::destroy) {
      static Atom wmDeleteWindow = XInternAtom(_xDisplay, "WM_DELETE_WINDOW", False);
      XSetWMProtocols(_xDisplay, _xWindow, &wmDeleteWindow, 1);
      // NB: to remove it: XSetWMProtocols(_xDisplay, _xWindow, &wmDeleteWindow, 0);
    }
    
    XFree(vi);
#ifdef HAVE_XINPUT
    _setupXInput();
#endif

    makeCurrent() ;
  }

  // ------------------------------------------------------------------

  glWindow_GLX::~glWindow_GLX(void) {
    // std::cerr << "glWindow_GLX::~glWindow_GLX" << std::endl ;
    unsubscribeFrom(_fk) ;
    delete _fk ;
    unmap() ;
    XSync(_xDisplay, False) ;
    glXDestroyContext(_xDisplay, _glContext) ;
    XSync(_xDisplay, False) ;
    XDestroyWindow(_xDisplay, _xWindow) ;
  }

  // ------------------------------------------------------------------

  void
  glWindow_GLX::map(void) {
    // std::cerr << "glWindow_GLX::map" << std::endl ;
    if (!_mapped) {
	 // std::cerr << "glWindow_GLX::map !!!!!!!" << std::endl ;
	 XMapWindow(_xDisplay, _xWindow) ;
	 XFlush(_xDisplay) ;
	 _mapped = true ;
    }
  }

  void
  glWindow_GLX::unmap(void) {
    // std::cerr << "glWindow_GLX::unmap" << std::endl ;
    if (_mapped) {
	 // std::cerr << "glWindow_GLX::unmap !!!!!!!" << std::endl ;
	 XUnmapWindow(_xDisplay, _xWindow) ;
	 _mapped = false ;
    }
  }

  // ------------------------------------------------------------------

  // use the EWMH spec (see http://www.freedesktop.org/Standards/wm-spec)
  // for certain opertaions. Most "modern" X wm implement this spec (e.g.,
  // FVWM-2.5.x, metacity, kwin, icewm, latest E16, swafish ...etc.)

  void
  *glWindow_GLX::_getAtom(Window id, Atom to_get, Atom type, unsigned int *size)
  {
    unsigned char *retval;
    Atom  type_ret;
    unsigned long  bytes_after, num_ret;
    long length;
    int  format_ret;
    void *data;
    int ok;

    retval = NULL;
    length = 0x7fffffff;
    ok = XGetWindowProperty(_xDisplay, id, to_get, 0, length, False, type,
					   &type_ret, &format_ret, &num_ret, &bytes_after, &retval);

    if ((ok == Success) && (retval) && (num_ret > 0) && (format_ret > 0)) {
      data = malloc(num_ret * (format_ret >> 3));
      if (data) {
	   memmove(data, retval, num_ret * (format_ret >> 3));
      }
      XFree(retval);
      *size = num_ret * (format_ret >> 3);
      return data;
    }
    if (retval)
      XFree(retval);
    return NULL;
  }

  // check if the current wm is EWMH compliant and support the netProprety
  // property
  bool
  glWindow_GLX::_checkNetSupported(Atom netProprety) {
    Atom wmWindowAtom;
    Atom netSupportedAtom;

    wmWindowAtom = XInternAtom(_xDisplay,"_NET_SUPPORTING_WM_CHECK", False);
    netSupportedAtom = XInternAtom(_xDisplay,"_NET_SUPPORTED", False);
     
    // first we check that the current window manager is EWMH compliant
    // Note that the wm may change during the life of the window
    unsigned int size;
    Window *wmWindow = (Window *)_getAtom(DefaultRootWindow(_xDisplay),
								  wmWindowAtom, XA_WINDOW, &size);
    if (!wmWindow)
      return false;

    // check that the window exist ...
    Window junkRoot;
    int junkX, junkY;
    unsigned int junkWidth, junkHeight, junkBW, junkDepth;
    if (!XGetGeometry(_xDisplay, wmWindow[0], &junkRoot, &junkX, &junkY,
				  &junkWidth, &junkHeight, &junkBW,  &junkDepth)) {
      return false;
    }

    // check that the window is the good corresponding window 
    Window *checkWindow = 0;
    checkWindow = (Window *)_getAtom(wmWindow[0], wmWindowAtom, XA_WINDOW,
							  &size);

    if (!checkWindow || checkWindow[0] != wmWindow[0]) {
      if (checkWindow)
	   free(checkWindow);
      free(wmWindow);
      return false;
    }

    free(wmWindow);
    free(checkWindow);

    // get what the wm support
    Atom *supported;
    supported = (Atom *)_getAtom(DefaultRootWindow(_xDisplay),
						   netSupportedAtom, XA_ATOM, &size);
    if (!supported)
      return false;

    unsigned int num = 0;
    bool found = false;
    while(num < size && !found) {
      if (supported[num] == netProprety) {
	   found = true;
      }
      num++;
    }
    free(supported);
    return found;
  }

  bool
  glWindow_GLX::_ewmhFullScreenMode(bool activate) {
    Atom fullScreenAtom = XInternAtom(_xDisplay,"_NET_WM_STATE_FULLSCREEN",0) ;

    if (!_checkNetSupported(fullScreenAtom))
      return false;
    Atom state = XInternAtom(_xDisplay,"_NET_WM_STATE",0);
    if (!_mapped && activate) {
      XChangeProperty(_xDisplay, _xWindow, state, XA_ATOM, 32, PropModeAppend,
				  (unsigned char*)&fullScreenAtom, 1);
      setGeometry(getScreenWidth(),getScreenHeight(),0,0) ;
    } else if (_mapped) {
      std::cerr << "Ewmh fullscreen " << activate << std::endl;
      XClientMessageEvent ev;
      ev.type = ClientMessage;
      ev.window = _xWindow;
      ev.message_type = state;
      ev.format = 32;
      ev.data.l[0] = (int)activate;
      ev.data.l[1] = fullScreenAtom;
      ev.data.l[2] = 0;
      XSendEvent(_xDisplay, DefaultRootWindow(_xDisplay), False,
			  (SubstructureNotifyMask|SubstructureRedirectMask),
			  (XEvent *) &ev);
      /* the wm move-resize for us. But in the case where we _start_
       * fullscreen and we ask to desactivate the wm cannot know the size
       * we want, so in this case the caller should resize the window
       * itself to the desired size. */
    }
    else {
      // !_mapped && !activate, nothing to do!?
    }
    return true;
  }

  //-------------------------------------------------------------------
  // xinput stuff
#ifdef HAVE_XINPUT
  void glWindow_GLX::_setupXInput(void)
  {
    // std::cerr << "glWindow_GLX::_setupXInput" << std::endl ;
	  _extDevices = new extDevicesList_XInput;
  }

  void glWindow_GLX::_resetXInput(void)
  {
    // std::cerr << "glWindow_GLX::_resetXInput" << std::endl ;
	  std::list<glWindow_GLX::extensionDevice_XInput *>::iterator o;
	  for(o = _extDevices->begin(); o != _extDevices->end(); o++)
	  {
		  delete (*o);
	  }

	  delete _extDevices;
	  _setupXInput();
  }

  bool glWindow_GLX::_initXInput(void)
  {
    // std::cerr << "glWindow_GLX::_initXInput" << std::endl ;
    XExtensionVersion *ext;
    bool getExtension = false;

    ext = XGetExtensionVersion(_xDisplay, "XInput");
  
    if (!ext || (ext == (XExtensionVersion*) NoSuchExtension))
    {
	    getExtension = false;
    }
    else
    {
	    getExtension = true;
    }

    if (debugExtInput)
    {
	    if (!getExtension)
	    {
		    std::cerr << "No XInput extension " << std::endl;
	    }
	    else
	    {
		    std::cerr << "XInputExtension Version "
			      << ext->major_version << "."
			      << ext->minor_version
			      << std::endl;
	    }
    }

    if (ext)
    {
	    XFree(ext);
    }

    return getExtension;

  }

   glWindow::extDevicesList *glWindow_GLX::getExtensionDevices(void)
  {
    // std::cerr << "glWindow_GLX::getExtensionDevices" << std::endl ;
    XDeviceInfo *devices;
    int num_devices, loop, i;

    if (_extDevices->begin() != _extDevices->end())
    {
	    return (extDevicesList *)_extDevices;
    }

    if (!_initXInput())
    {
	    return 0;
    }

    devices = XListInputDevices(_xDisplay, &num_devices);
    glWindow_GLX::extensionDevice_XInput *ed;
    for(loop=0; loop<num_devices; loop++)
    {
	    switch(devices[loop].use)
	    {
	    case IsXPointer: // core pointer
		    if (_origCorePointerName == 0 && devices[loop].name != 0)
		    {
			    _origCorePointerName = strdup(devices[loop].name);
			    if (debugExtInput)
			    {
				    std::cerr << "Core Pointer name: "
					      << _origCorePointerName
					      << std::endl;
			    }
		    }
		    break;
	    case IsXKeyboard: // core keyboard
		    break;
		    
	    case IsXExtensionDevice:
	    {
	       if (debugExtInput)
	       {
		       std::cerr << "Get eid " << devices[loop].name
				 << " with id "
				 << devices[loop].id << std::endl;
	       }
	       unsigned int dev_type = 0;
	       for(i = 0; i < devices[loop].num_classes;
		   i++, devices[loop].inputclassinfo =
			   (XAnyClassPtr)(
				   (char *)devices[loop].inputclassinfo +
				   devices[loop].inputclassinfo->length))
	       {
		  XAnyClassPtr  ici;
		  ici = devices[loop].inputclassinfo;
		  switch(devices[loop].inputclassinfo->c_class)
		  {
		  case ButtonClass:
		  {
			  if (debugExtInput)
			  {
				  XButtonInfo *bi = (XButtonInfo *)ici;
				  std::cerr << "  ButtonClass num_buttons: "
					    << bi->num_buttons << std::endl;
			  }
			  dev_type |= glWindow::extensionDevice::HAS_BUTTONS;
			  break;
		  }
		  case KeyClass:
		  {
			  if (debugExtInput)
			  {
				  XKeyInfo *ki = (XKeyInfo *)ici;
				  std::cerr << "  KeyClass  min_keycode: "
					    << ki->min_keycode
					    << ", max_keycode: " 
					    << ki->max_keycode
					    << ", num_keys: " << ki->num_keys
					    << std::endl;
			  }
			  dev_type |= glWindow::extensionDevice::HAS_KEYS;
			  break;
		  }
		  case ValuatorClass:
		  {
			  if (debugExtInput)
			  {
				  XValuatorInfo *vi = (XValuatorInfo *)ici;
				  std::cerr << "  ValuatorClass num_axes: "
					    << (int)vi->num_axes
					    << ", mode: "
					    << (int)vi->mode 
					    << ", motion_buffer: "
					    << vi->motion_buffer
					    <<  std::endl;
				  int k;
				  for (k = 0; k < vi->num_axes; k++, vi->axes++)
				  {
					  std::cerr << "    Axe " << k
						    << " res: "
						    << vi->axes->resolution
						    << ", min: "
						    << vi->axes->min_value
						    << ", max: " 
						    << vi->axes->max_value
						    <<  std::endl;
				  }
			  }
			  dev_type |= glWindow::extensionDevice::HAS_VALUATORS;
			  break;
		  }
		  case ProximityClass:
		  {
			if (debugExtInput)
			{
				std::cerr << "  ProximityClass" << std::endl;
			}
			dev_type |= glWindow::extensionDevice::HAS_PROXIMITY;
			break;
		  }
		  default:
			  break;
		  }
	       } // for
	       ed = new glWindow_GLX::extensionDevice_XInput(
		       (int)(devices[loop].id), dev_type, devices[loop].name);
	       _extDevices->push_back(ed);
	    } // IsXExtensionDevice

	    default:
		    break;
	    }
    }

    XFreeDeviceList(devices);

    return (extDevicesList *)_extDevices;
  }


  glWindow_GLX::extensionDevice_XInput *glWindow_GLX::_findExtensionDevice(
	  unsigned long id, char *name)
  {
    // std::cerr << "glWindow_GLX::_findExtensionDevice" << std::endl ;
	  extensionDevice_XInput *extDevX = 0;
	  std::list<glWindow_GLX::extensionDevice_XInput *>::iterator o;
	  for(o = _extDevices->begin(); o != _extDevices->end(); o++)
	  {
		  if (name != 0)
		  {
			  if (strcmp((*o)->getName(), name) == 0)
			  {
				  extDevX = (*o);
				  break;
			  }
		  }
		  else if ((*o)->getID() == id)
		  {
			  extDevX = (*o);
			  break;
		  }
	  }

	  if (extDevX == 0)
	  {
		  return 0;
	  }

	  XDevice *dev = extDevX->getDevice();
	  if (dev == 0)
	  {
		  dev =  XOpenDevice(_xDisplay, extDevX->getID());
		  extDevX->setDevice(dev);
		  if (debugExtInput)
		  {
			  std::cerr << "Opened edi " << extDevX->getName()
				    << " (" << dev << ")" << std::endl;
		  }
		  if (dev == 0)
		  {
			  std::cerr << "Nucleo: fail to Open Extension device " 
				    << extDevX->getName() << std::endl;
		  }
	  }

	  return extDevX;
  }

  bool glWindow_GLX::getExtensionPtrAccel(
	  extensionDevice *extDev, int *accelNum, int *accelDenom,
	  int *threshold)
  {
    // std::cerr << "glWindow_GLX::getExtensionPtrAccel" << std::endl ;
	  extensionDevice_XInput *extDevX;

	  extDevX = _findExtensionDevice(extDev->getID());

	  if (extDevX == 0)
	  {
		  return false;
	  }

	  XDevice *dev = extDevX->getDevice();
	  if (dev == 0)
	  {
		  dev =  XOpenDevice(_xDisplay, extDevX->getID());
		  extDevX->setDevice(dev);
		  if (dev == 0)
		  {
			  std::cerr << "Nucleo: fail to Open Extension device " 
				    << extDevX->getName() << std::endl;
			  return false;
		  }
	  }

	  XFeedbackState *state;
	  int num_feedbacks;
	  int loop;
	  bool found = false;

	  state = XGetFeedbackControl(_xDisplay, dev, &num_feedbacks);
	  for(loop=0; loop < num_feedbacks; loop++)
	  {
		  if (state->c_class == PtrFeedbackClass)
		  {
			  XPtrFeedbackState *pfs = (XPtrFeedbackState*) state;
			  *accelNum = pfs->accelNum; 
			  *accelDenom = pfs->accelDenom;
			  *threshold = pfs->threshold;
			  found = true;
			  break;
		  }
		  else
		  {
			  state = (XFeedbackState*)
				  ((char*) state + state->length);
		  }
	  }
	  // FIXME: should free state ?
	  return found;	  
  }

  bool glWindow_GLX::setExtensionPtrAccel(
	  extensionDevice *extDev, int accelNum, int accelDenom, int threshold)
  {
    // std::cerr << "glWindow_GLX::setExtensionPtrAccel" << std::endl ;
	  extensionDevice_XInput *extDevX;
	  XPtrFeedbackControl feedback;

	  extDevX = _findExtensionDevice(extDev->getID());

	  if (extDevX == 0)
	  {
		  return false;
	  }

	  XDevice *dev = extDevX->getDevice();
	  if (dev == 0)
	  {
		  dev =  XOpenDevice(_xDisplay, extDevX->getID());
		  extDevX->setDevice(dev);
		  if (dev == 0)
		  {
			  std::cerr << "Nucleo: fail to Open Extension device " 
				    << extDevX->getName() << std::endl;
			  return false;
		  }
	  }

	  feedback.c_class = PtrFeedbackClass;
	  feedback.length = sizeof(XPtrFeedbackControl);
	  feedback.id = 0;
	  feedback.threshold = threshold;
	  feedback.accelNum = accelNum;
	  feedback.accelDenom = accelDenom;

	  XChangeFeedbackControl(
		  _xDisplay, dev, DvAccelNum|DvAccelDenom|DvThreshold,
		  (XFeedbackControl*) &feedback);

	  return true;
  }

  bool glWindow_GLX::changeCorePointer(
	  extensionDevice *extDev, int xaxis, int yaxis)
  {
    // std::cerr << "glWindow_GLX::changeCorePointer" << std::endl ;
	  extensionDevice_XInput *extDevX;

	  extDevX = _findExtensionDevice(extDev->getID());

	  if (extDevX == 0)
	  {
		  return false;
	  }

	  if (!(extDevX->hasValuators() && extDevX->hasButtons()))
	  {
		  return false;
	  }

	  XDevice *dev = extDevX->getDevice();
	  if (dev == 0)
	  {
		  return false;
	  }

	  XChangePointerDevice(_xDisplay, dev, xaxis, yaxis);
	  if (debugExtInput)
	  {
		  std::cerr << "New core pointer: "
			    << extDevX->getName() << std::endl;
	  }
	  _corePointerHasChanged = true;
	  XSync(_xDisplay, False);

	  _resetXInput();

	  return true;
  }

  void glWindow_GLX::restoreCorePointer(void)
  {
    // std::cerr << "glWindow_GLX::restoreCorePointer" << std::endl ;
	  if (!_corePointerHasChanged || _origCorePointerName == 0)
	  {
		  if (debugExtInput)
		  {
			  std::cerr << "restoreCorePointer: nothing to do"
				    << std::endl;
		  }
		  return;
	  }

	  getExtensionDevices();

	  extensionDevice_XInput *extDevX =
		  _findExtensionDevice(0, _origCorePointerName);

	  if (extDevX == 0)
	  {
		  if (debugExtInput)
		  {
			 std::cerr << "restoreCorePointer: core pointer not "
				   << "found" << std::endl; 
		  }
		  return;
	  }

	  XDevice *dev = extDevX->getDevice();
	  if (dev == 0)
	  {
		  return;
	  }

	  XChangePointerDevice(_xDisplay, dev, 0, 1);
	  _corePointerHasChanged = false;
	  XSync(_xDisplay, False);

	  _resetXInput();
  }

  void glWindow_GLX::selectExtensionEvent(
	  extensionDevice *extDev, unsigned int events, bool onRootToo)
  {
    // std::cerr << "glWindow_GLX::selectExtensionEvent" << std::endl ;
	  extensionDevice_XInput *extDevX =
		  _findExtensionDevice(extDev->getID());
	  
	  if (extDevX == 0)
	  {
		  return;
	  }

	  XDevice *dev = extDevX->getDevice();
	  if (dev == 0)
	  {
		  return;
	  }

	  int i;
	  XInputClassInfo *ip;
	  bool hasProximity = false;
	  bool hasFocus = false;
	  for (ip = dev->classes, i=0; i< dev->num_classes; ip++, i++)
	  {
		  switch (ip->input_class)
		  {
		  case KeyClass:
		  case ButtonClass:
		  case ValuatorClass:
			  break;
		  case ProximityClass:
			  if (debugExtInput)
			  {
				  std::cerr << extDevX->getName()
					    << " has a Proximity class"
					    << std::endl;
			  }
			  hasProximity = true;
			  break;
		  case FocusClass:
			  if (debugExtInput)
			  {
				  std::cerr << extDevX->getName()
					    << " has a Focus class" << std::endl;
			  }
			  hasFocus = true;
			  break;
		  case FeedbackClass:
			  if (debugExtInput)
			  {
				  std::cerr << extDevX->getName()
					    << " has a FeedbackClass class"
					    << std::endl;
			  }
			  break;
		  case OtherClass:
			  if (debugExtInput)
			  {
				  std::cerr << extDevX->getName()
					    << " has an OtherClass class"
					    << std::endl;
			  }
		  default:
			  if (debugExtInput)
			  {
				  std::cerr << extDevX->getName()
					    << " has an unkown class classes"
					    << std::endl;
			  }
			  break;
		  }
	  }

	  unsigned char event_type;
	  XEventClass event_list[32];
	  int num = 0;

	  if ((events & glWindow::event::extMotionNotify) &&
	      extDevX->hasValuators())
	  {
		  DeviceMotionNotify(dev, event_type, event_list[num]);
		  num++;
		  extDevX->setDeviceMotionNotify(event_type);
	  }
	  if ((events & glWindow::event::extButtonPress) &&
	      extDevX->hasButtons())
	  {
		  DeviceButtonPress(dev, event_type, event_list[num]);
		  num++;
		  extDevX->setDeviceButtonPress(event_type);
	  }
	  if ((events & glWindow::event::extButtonRelease) &&
	      extDevX->hasButtons())
	  {
		  DeviceButtonRelease(dev, event_type, event_list[num]);
		  num++;
		  extDevX->setDeviceButtonRelease(event_type);
	  }
	  if ((events & glWindow::event::extKeyPress) &&
	      extDevX->hasKeys())
	  {
		  DeviceKeyPress(dev, event_type, event_list[num]);
		  num++;
		  extDevX->setDeviceKeyPress(event_type);
	  }
	  if ((events & glWindow::event::extKeyRelease) &&
	      extDevX->hasKeys())
	  {
		  DeviceKeyRelease(dev, event_type, event_list[num]);
		  num++;
		  extDevX->setDeviceKeyRelease(event_type);
	  }
	  if ((events & glWindow::event::extProximityIn) &&
	      (extDevX->hasProximity() || hasProximity))
	  {
		  ProximityIn(dev, event_type, event_list[num]);
		  num++;
		  extDevX->setProximityIn(event_type);
	  }
	  if ((events & glWindow::event::extProximityOut) &&
	      (extDevX->hasProximity() || hasProximity))
	  {
		  ProximityOut(dev, event_type, event_list[num]);
		  num++;
		  extDevX->setProximityOut(event_type);
	  }
	  if ((events & glWindow::event::extFocusIn) &&
	      hasFocus)
	  {
		  DeviceFocusIn(dev, event_type, event_list[num]);
		  num++;
		  extDevX->setDeviceFocusIn(event_type);
	  }
	  if ((events & glWindow::event::extFocusOut) &&
	      hasFocus)
	  {
		  DeviceFocusOut(dev, event_type, event_list[num]);
		  num++;
		  extDevX->setDeviceFocusOut(event_type);
	  }
	  if (events & glWindow::event::extStateNotify)
	  {
		  DeviceStateNotify(dev, event_type, event_list[num]);
		  num++;
		  #if 0
		  extDevX->setDeviceStateNotify(event_type);
		  DeviceMappingNotify(dev, event_type, event_list[num]);
		  num++;
		  ChangeDeviceNotify(dev, event_type, event_list[num]);
		  num++;
		  #endif
	  }
	  XSelectExtensionEvent(
		  _xDisplay, _xWindow, event_list, num);
	  if (onRootToo)
	  {
		  XSelectExtensionEvent(
			  _xDisplay, DefaultRootWindow(_xDisplay),
			  event_list, num);
		  // should not do that ... the wm should do things like that?
		  #if 0
		  XSetDeviceFocus(
			  _xDisplay, dev, PointerRoot,
			  RevertToPointerRoot, CurrentTime);
		  #endif
	  }
	  else
	  {
		  // should not do that ... the wm should do things like that?
		  #if 0
		  XSetDeviceFocus(
			  _xDisplay, dev, FollowKeyboard,
			  RevertToPointerRoot, CurrentTime);
		  #endif
	  }
	  if (debugExtInput)
	  {
		  std::cerr << "XSelectExtensionEvent for edi "
			    << extDevX->getName() << std::endl;
	  }
  }

   bool glWindow_GLX::_handleExtensionEvents(XEvent *xe, event *e)
   {
	// std::cerr << "glWindow_GLX::_handleExtensionEvents" << std::endl ;
	   int j;

	   for(std::list<glWindow_GLX::extensionDevice_XInput *>::iterator o =
		       _extDevices->begin();
	       o != _extDevices->end(); 
	       ++o)
	   {
		   if ((*o)->getDevice() == 0)
		   {
			   continue;
		   }
		   if (xe->type == (*o)->getDeviceMotionNotify())
		   {
			   XDeviceMotionEvent *eev = (XDeviceMotionEvent *)xe;
			   e->device_id = eev->deviceid;
			   e->type = glWindow::event::extensionEvent;
			   e->extType = glWindow::event::extMotionNotify;
			   e->time = eev->time;
			   e->first_axis = (int)eev->first_axis;
			   e->axes_count = (int)eev->axes_count;
			   for(j = (int)eev->first_axis;
			       j < (int)eev->axes_count;
			       j++)
			   {
				   e->axis_data[j] = eev->axis_data[j];
			   }
			   return true;
		   }
		   else if (xe->type == (*o)->getDeviceButtonPress() ||
			    xe->type == (*o)->getDeviceButtonRelease())
		   {
			   XDeviceButtonEvent *eev = (XDeviceButtonEvent *)xe;
			   e->device_id = eev->deviceid;
			   e->type = glWindow::event::extensionEvent;
			   e->extType =
				   (xe->type == (*o)->getDeviceButtonPress())?
				   glWindow::event::extButtonPress :
				   glWindow::event::extButtonRelease;
			   e->time = eev->time;
			   e->button = eev->button;
			   e->x = eev->x;
			   e->y = eev->y;
			   e->first_axis = (int)eev->first_axis;
			   e->axes_count = (int)eev->axes_count;
			   for(j = 0;
			       j < (int)eev->axes_count;
			       j++)
			   {
				   e->axis_data[j] = eev->axis_data[j];
			   }
			   return true;
		   }
		   else if (xe->type == (*o)->getDeviceKeyPress() ||
			    xe->type == (*o)->getDeviceKeyRelease())
		   {
			   XDeviceKeyEvent *eev = (XDeviceKeyEvent *)xe;
			   e->device_id = eev->deviceid;
			   e->type = glWindow::event::extensionEvent;
			   e->extType =
				   (xe->type == (*o)->getDeviceKeyPress())?
				   glWindow::event::extKeyPress :
				   glWindow::event::extKeyRelease;
			   e->time = eev->time;
			   KeySym ks ;
			   char keyname[256] ;
			   XLookupString(
				   (XKeyEvent *)eev, keyname, 256, &ks, NULL) ;
			   if (IsModifierKey(ks)) {
				   ks = XKeycodeToKeysym(
					   _xDisplay, eev->keycode, 0);
				   XLookupString(
					   (XKeyEvent *)eev, keyname, 256, &ks,
					   NULL) ;
			   }
			   e->keysym = (unsigned long)ks ;
			   e->keystr = keyname ;
			   return true;
		   }
		   else if (xe->type == (*o)->getProximityIn() ||
			    xe->type == (*o)->getProximityOut())
		   {
			   XProximityNotifyEvent *eev =
				   (XProximityNotifyEvent *)xe;
			   e->device_id = eev->deviceid;
			   e->type = glWindow::event::extensionEvent;
			   e->extType = (xe->type == (*o)->getProximityIn())?
				   glWindow::event::extProximityIn :
				   glWindow::event::extProximityOut;
			   e->time = eev->time;
			   e->first_axis = (int)eev->first_axis;
			   e->axes_count = (int)eev->axes_count;
			   for(j = 0;
			       j < (int)eev->axes_count;
			       j++)
			   {
				   e->axis_data[j] = eev->axis_data[j];
			   }
			   return true;
		   }
		   else if (xe->type == (*o)->getDeviceFocusIn() ||
			    xe->type == (*o)->getDeviceFocusOut())
		   {
			   XDeviceFocusChangeEvent *eev =
				   (XDeviceFocusChangeEvent *)xe;
			   e->device_id = eev->deviceid;
			   e->type = glWindow::event::extensionEvent;
			   e->extType = (xe->type == (*o)->getDeviceFocusIn())?
				   glWindow::event::extFocusIn :
				   glWindow::event::extFocusOut;
			   e->time = eev->time;
			   return true;
		   }
		   else if (xe->type == (*o)->getDeviceStateNotify())
		   {
			   XDeviceStateNotifyEvent *eev =
				   (XDeviceStateNotifyEvent *)xe;   
			   e->device_id = eev->deviceid;
			   e->type = glWindow::event::extensionEvent;
			   e->extType = glWindow::event::extStateNotify;
			   e->time = eev->time;
			   return true;
		   }
		   else
		   {
			   std::cerr << "Should not happen" << std::endl;
		   }
	   }
	   return false;
   }
#endif

  // ------------------------------------------------------------------

  void 
  glWindow_GLX::setTitle(const char *title) {
    XStoreName(_xDisplay, _xWindow, title) ;
  }
  
  void
  glWindow_GLX::getGeometry(unsigned int *width, unsigned int *height,
					   int *x, int *y) {
    int rx, ry ;
    unsigned int rw, rh ;
    Window wDummy ;
    unsigned int uiDummy ;
    XGetGeometry(_xDisplay, _xWindow, &wDummy,
			  &rx, &ry, &rw, &rh,
			  &uiDummy, &uiDummy) ;
    if (x) *x = rx ; if (y) *y = ry ;
    if (width) *width = rw ; if (height) *height = rh ;
  }

  void
  glWindow_GLX::setGeometry(unsigned int width, unsigned int height,
					   int x, int y) {
    if (x<0 || y<0) {
	 Window wDummy ;
	 unsigned int uiDummy, pWidth, pHeight ;
	 int iDummy ;
	 XGetGeometry(_xDisplay, _xParent, &wDummy,
			    &iDummy, &iDummy, &pWidth, &pHeight, &uiDummy, &uiDummy) ;
	 if (x<0) x += pWidth ;
	 if (y<0) y += pHeight ;
    }
    XMoveResizeWindow(_xDisplay, _xWindow, x, y, width, height) ;
    map() ;
  }

  void
  glWindow_GLX::setGeometry(unsigned int width, unsigned int height) {
    XResizeWindow(_xDisplay, _xWindow, width, height) ;
    map() ;
  }
  
  void
  glWindow_GLX::setAspectRatio(int width, int height) {
    XSizeHints *h = XAllocSizeHints() ;
    h->flags = PAspect ;
    h->min_aspect.x = h->max_aspect.x = width ;
    h->min_aspect.y = h->max_aspect.y = height ;
    XSetWMNormalHints(_xDisplay, _xWindow, h) ;
    XFree(h) ;
  }

  void 
  glWindow_GLX::setMinMaxSize(int minwidth, int minheight, int maxwidth, int maxheight) {
    XSizeHints *h = XAllocSizeHints() ;
    h->flags = 0 ;
    if (minwidth>-1 && minheight>-1) {
	 h->flags = h->flags | PMinSize ;
	 h->min_width = minwidth ;
	 h->min_height = minheight ;
    }
    if (maxwidth>-1 && maxheight>-1) {
	 h->flags = h->flags | PMaxSize ;
	 h->max_width = maxwidth ;
	 h->max_height = maxheight ;
    }
    XSetWMNormalHints(_xDisplay, _xWindow, h) ;
    XFree(h) ;
  }

  void
  glWindow_GLX::setFullScreen(bool activate) {
    if (!_ewmhFullScreenMode(activate)) {
      // EWMH method fails
#ifdef MWM_HINTS_DECORATIONS
      XGrabServer(_xDisplay) ;
      if (_mapped) unmap() ;
      Atom MotifHints = XInternAtom(_xDisplay,"_MOTIF_WM_HINTS",0) ;
      MotifWmHints hints ;
      hints.flags = MWM_HINTS_DECORATIONS ;
      hints.decorations = activate ? 0 : MWM_DECOR_ALL ;
      XChangeProperty(_xDisplay, _xWindow,
				  MotifHints, MotifHints, 32,
				  PropModeReplace, (unsigned char *) &hints, 4) ;
      XUngrabServer(_xDisplay) ;
#endif
      if (activate)
	   setGeometry(getScreenWidth(),getScreenHeight(),0,0) ;
      else
	   {
		// I do not think this is correct; either we should restore the
		// size and position before fullscreen state or we have been
		// started fullscreen and the caller should resize itself.
		// (at least this is the ewmh logic).
		setGeometry(getScreenWidth()/2,getScreenHeight()/2,50,50) ;
	   }
    }
    map() ;
  }

  void
  glWindow_GLX::setCursorVisible(bool activate) {
    Cursor cursor = 0 ;
    if (!activate) {
	 XColor xcf, xcb ;
	 static char m [] = { 0x0 } ;
	 Pixmap pixmap = XCreateBitmapFromData (_xDisplay, _xWindow, m, 1, 1) ;
	 cursor = XCreatePixmapCursor (_xDisplay, pixmap, pixmap, &xcf, &xcb, 0, 0) ;
    } else {
	 cursor = XCreateFontCursor(_xDisplay, XC_top_left_arrow) ; // not the right arrow...
    }
    XDefineCursor (_xDisplay, _xWindow, cursor) ;
  }

  void
  glWindow_GLX::warpCursor(int x, int y) {
    XWarpPointer(_xDisplay, _xWindow, _xWindow, 0, 0, 0, 0, x, y);
    XSync(_xDisplay, False);
  }

  void
  glWindow_GLX::setKeyboardAutoRepeat(bool activate) {
    if (activate)
	 XAutoRepeatOn(_xDisplay) ;
    else
	 XAutoRepeatOff(_xDisplay) ;
  }

  bool glWindow_GLX::getPtrAccel(int *accelNum, int *accelDenom, int *threshold)
  {
	  XGetPointerControl(_xDisplay, accelNum, accelDenom, threshold);
	  return true;
  }

  bool glWindow_GLX::setPtrAccel(int accelNum, int accelDenom, int threshold)
  {
	  XChangePointerControl(
		  _xDisplay, True, True, accelNum, accelDenom, threshold);
	  return true;
  }

  void
  glWindow_GLX::raise(void) {
    XRaiseWindow(_xDisplay, _xWindow) ;
  }

  void
  glWindow_GLX::lower(void) {
    XLowerWindow(_xDisplay, _xWindow) ;
  }

  // ------------------------------------------------------------------

  void
  glWindow_GLX::makeCurrent(void) {
    // std::cerr << "glWindow_GLX::makeCurrent " << std::endl ;
    glXMakeCurrent(_xDisplay, _xWindow, _glContext) ;
  }

  // ------------------------------------------------------------------

  void
  glWindow_GLX::swapBuffers(void) {
    // std::cerr << "glWindow_GLX::swapBuffers " << std::endl ;
    glXSwapBuffers(_xDisplay, _xWindow) ; // _xWindow was glXGetCurrentDrawable() before
  }

  // ------------------------------------------------------------------

  void
  glWindow_GLX::react(Observable*) {
    // if (_fk->getState()&FileKeeper::R)
    notifyObservers() ;
  }

  glWindow::event*
  glWindow_GLX::getNextEvent(void) {
    event *e = new event ;
    if (getNextEvent(e)) return e ;
    delete e ;
    return 0 ;
  }

  bool
  glWindow_GLX::getNextEvent(event *e) {
    static Atom wmDeleteWindow = XInternAtom(_xDisplay, "WM_DELETE_WINDOW", False);
    bool eventHandled = false;

    while(!eventHandled) {
      if (!XPending(_xDisplay)) return false ;
      eventHandled = true;
      XEvent xe ;
      XNextEvent(_xDisplay, &xe) ;
      switch (xe.type) {
      case ConfigureNotify:
	   e->type = glWindow::event::configure ;
	   e->width = xe.xconfigure.width ;
	   e->height = xe.xconfigure.height ;
	   e->x = xe.xconfigure.x ;
	   e->y = xe.xconfigure.y ;
	   e->time = CurrentTime;
	   break ;
      case Expose:
	   e->type = glWindow::event::expose ;
	   e->time = CurrentTime;
	   break ;
      case DestroyNotify:
	   e->type = glWindow::event::destroy;
	   e->time = CurrentTime;
	   break;
      case ClientMessage:
	   if (xe.xclient.format == 32 &&
	       (unsigned)xe.xclient.data.l[0] == wmDeleteWindow) {
		e->type = glWindow::event::destroy;
	   }
	   e->time = CurrentTime;
	   break;
	   // Ignore Enter/Leave events generated by grabbing or ungrabbing the
	   // pointer if this is needed a new member may be added to e 
      case EnterNotify:
	   if (xe.xcrossing.mode != NotifyNormal)
		eventHandled = false;  // pointer ungrab
	   else
		e->type = glWindow::event::enter ;
	   e->time = xe.xcrossing.time;
	   break ;
      case LeaveNotify:
	   if (xe.xcrossing.mode != NotifyNormal)
		eventHandled = false; // pointer grab
	   else
		e->type = glWindow::event::leave ;
	   break ;
      case ButtonPress:
      case ButtonRelease:
	   e->type = (xe.type==ButtonPress) ? glWindow::event::buttonPress : glWindow::event::buttonRelease ;
	   e->x = xe.xbutton.x ;
	   e->y = xe.xbutton.y ;
	   e->button = xe.xbutton.button ;
	   e->time = xe.xbutton.time;
	   break ;
      case MotionNotify:
	   e->type = glWindow::event::pointerMotion ;
	   while (XCheckTypedWindowEvent(_xDisplay, xe.xmotion.window, MotionNotify, &xe)) ;
	   e->x = xe.xmotion.x ;
	   e->y = xe.xmotion.y ;
	   e->time = xe.xmotion.time;
	   break ;
      case KeyPress:
      case KeyRelease: {
	   KeySym ks ;
	   char keyname[256] ;
	   XLookupString(&xe.xkey, keyname, 256, &ks, NULL) ;
	   if (IsModifierKey(ks)) {
		ks = XKeycodeToKeysym(_xDisplay, xe.xkey.keycode, 0);
		XLookupString(&xe.xkey, keyname, 256, &ks, NULL) ;
	   }
	   e->type = (xe.type==KeyPress) ? glWindow::event::keyPress : glWindow::event::keyRelease ;
	   e->keysym = (unsigned long)ks ;
	   e->keystr = keyname ;
	   e->time = xe.xkey.time;
      } break ;
      case FocusIn:
	      if (xe.xfocus.mode != NotifyNormal)
		      eventHandled = false; // pointer grab
	      else
		      e->type = glWindow::event::focusIn;
	      break;
      case FocusOut:
	      if (xe.xfocus.mode != NotifyNormal)
		      eventHandled = false; // pointer ungrab
	      else
		      e->type = glWindow::event::focusOut;
	      break;

	   // not handled events selected by StructureNotifyMask
      case CirculateNotify:
      case GravityNotify:
      case MapNotify:
      case ReparentNotify:
      case UnmapNotify:
	   eventHandled = false;
	   break ;

	   // not handled events always selected
      case MappingNotify:
      case SelectionClear:
      case SelectionNotify:
      case SelectionRequest:
	   eventHandled = false;
	   break ;

      default:
#ifdef HAVE_XINPUT
	      if (!_handleExtensionEvents(&xe, e))
#endif
	      {
		      // should not happen
		      std::cerr << "glWindow_GLX: X event type " << xe.type
				<< std::endl ;
		      eventHandled = false;
	      }
	      break;
      }

      if (debugEvents) {
	   std::cerr << "glWindow_GLX event: " ;
	   e->debug(std::cerr) ;
	   std::cerr << std::endl ;
      }
    } /* while */

    return true ;
  }

  // ------------------------------------------------------------------

  Window  glWindow_GLX::getWindowID(void) {
    return _xWindow ;
  }

  Display *glWindow_GLX::getDisplay(void) {
    return _xDisplay ;
  }

}
