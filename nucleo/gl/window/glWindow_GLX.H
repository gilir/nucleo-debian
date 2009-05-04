#ifndef n_glWindow_GLX_H
#define n_glWindow_GLX_H

/*
 *
 * nucleo/gl/window/glWindow_GLX.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/FileUtils.H>
#include <nucleo/gl/window/glWindow.H>
#include <nucleo/core/FileKeeper.H>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#ifdef HAVE_XINPUT
#include <X11/extensions/XInput.h>
#endif

#include <cstdlib>

namespace nucleo {

  // ------------------------------------------------------------------

  class glWindow_GLX : public glWindow {

  private:
    void *_getAtom(Window id, Atom to_get, Atom type, unsigned int *size);
    bool _checkNetSupported(Atom toBeSupported);
    bool _ewmhFullScreenMode(bool activate);

  protected:

    Display *_xDisplay ;
    FileKeeper *_fk ;
    Window _xParent, _xWindow ;
    GLXContext _glContext ;
    bool _mapped ; // whether the window is currently mapped on-screen

    void setup(long options, long eventmask) ;
    
    void react(Observable *) ;

#ifdef HAVE_XINPUT
    class extensionDevice_XInput : public extensionDevice
    {
    private: 
      XDevice *_dev;
      unsigned char _DeviceMotionNotify;
      unsigned char _DeviceButtonPress;
      unsigned char _DeviceButtonRelease;
      unsigned char _DeviceKeyPress;
      unsigned char _DeviceKeyRelease;
      unsigned char _DeviceStateNotify;
      unsigned char _ProximityIn;
      unsigned char _ProximityOut;
      unsigned char _DeviceFocusIn;
      unsigned char _DeviceFocusOut;

    public:
      extensionDevice_XInput(int id, int dev_type, char *name) : 
	glWindow::extensionDevice(id, dev_type, name)
      {
	_dev = 0;
      }
      ~extensionDevice_XInput() { if (_name) free(_name); _name = 0; }
      void setDevice(XDevice *dev) { _dev = dev; }
      XDevice *getDevice(void) { return _dev; }
      void setDeviceMotionNotify(unsigned char event_type)
      {
	_DeviceMotionNotify = event_type;
      }
      unsigned char getDeviceMotionNotify(void)
      {
	return _DeviceMotionNotify;
      }
      void setDeviceButtonPress(unsigned char event_type)
      {
	_DeviceButtonPress = event_type;
      }
      unsigned char getDeviceButtonPress(void)
      {
	return _DeviceButtonPress;
      }
      void setDeviceButtonRelease(unsigned char event_type)
      {
	_DeviceButtonRelease = event_type;
      }
      unsigned char getDeviceButtonRelease(void)
      {
	return _DeviceButtonRelease;
      }
      void setDeviceKeyPress(unsigned char event_type)
      {
	_DeviceKeyPress = event_type;
      }
      unsigned char getDeviceKeyPress(void)
      {
	return _DeviceKeyPress;
      }
      void setDeviceKeyRelease(unsigned char event_type)
      {
	_DeviceKeyRelease = event_type;
      }
      unsigned char getDeviceKeyRelease(void)
      {
	return _DeviceKeyRelease;
      }
      void setDeviceStateNotify(unsigned char event_type)
      {
	_DeviceStateNotify = event_type;
      }
      unsigned char getDeviceStateNotify(void)
      {
	return _DeviceStateNotify;
      }
      void setProximityIn(unsigned char event_type)
      {
	_ProximityIn = event_type;
      }
      unsigned char getProximityIn(void)
      {
	return _ProximityIn;
      }
      void setProximityOut(unsigned char event_type)
      {
	_ProximityOut = event_type;
      }
      unsigned char getProximityOut(void)
      {
	return _ProximityOut;
      }
      void setDeviceFocusOut(unsigned char event_type)
      {
	_DeviceFocusOut = event_type;
      }
      unsigned char getDeviceFocusOut(void)
      {
	return _DeviceFocusOut;
      }
      void setDeviceFocusIn(unsigned char event_type)
      {
	_DeviceFocusIn = event_type;
      }
      unsigned char getDeviceFocusIn(void)
      {
	return _DeviceFocusIn;
      }
    };

    typedef std::list<extensionDevice_XInput *> extDevicesList_XInput ;
    extDevicesList_XInput *_extDevices;

    void _setupXInput(void);
    void _resetXInput(void);
    bool _initXInput(void);
    extensionDevice_XInput *_findExtensionDevice(unsigned long id, char *name=0);
    bool _handleExtensionEvents(XEvent *xe, event *e);
#endif

    char *_origCorePointerName;
    bool _corePointerHasChanged;

  public:

    unsigned int getScreenWidth(void) ;
    unsigned int getScreenHeight(void) ;

    glWindow_GLX(Display *_xDisplay, Window _xParent, long options, long eventmask) ;
    glWindow_GLX(long options, long eventmask) ;
    ~glWindow_GLX(void) ;

    void map(void) ;
    void unmap(void) ;

    void setTitle(const char *title) ;
    void getGeometry(unsigned int *width, unsigned int *height,
				 int *x=0, int *y=0) ;
    void setGeometry(unsigned int width, unsigned int height,
				 int x, int y) ;
    void setGeometry(unsigned int width, unsigned int height);
    void setGeometry(char *geom) { glWindow::setGeometry(geom) ; }
    void setAspectRatio(int width, int height) ;
    void setMinMaxSize(int minwidth, int minheight, int maxwidth, int maxheight) ;
    void setFullScreen(bool activate) ;
    void setCursorVisible(bool activate) ;
    void warpCursor(int x, int y) ;
    void setKeyboardAutoRepeat(bool activate) ;
    bool getPtrAccel(int *accelNum, int *accelDenom, int *threshold);
    bool setPtrAccel(int accelNum,int accelDenom,int threshold);
    void raise(void) ;
    void lower(void) ;

    void makeCurrent(void) ;
    void swapBuffers(void) ;

    bool getNextEvent(event *e) ;
    event *getNextEvent(void) ;

    // -------------------------------------------

    Display *getDisplay(void);
    Window getWindowID(void);

    // -------------------------------------------

#ifdef HAVE_XINPUT
    extDevicesList *getExtensionDevices(void);
    void selectExtensionEvent(
	    extensionDevice *extDev, unsigned int events, bool onRootToo);
    bool changeCorePointer(
	    extensionDevice *extDev, int xaxis, int yaxis);
    void restoreCorePointer(void);
    bool getExtensionPtrAccel(
	    extensionDevice *extDev, int *accelNum, int *accelDenom,
	    int *threshold);
    bool setExtensionPtrAccel(
	    extensionDevice *extDev, int accelNum, int accelDenom,
	    int threshold);
#endif

  } ;

  // ------------------------------------------------------------------

}

#endif
