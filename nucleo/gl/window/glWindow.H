#ifndef n_glWindow_H
#define n_glWindow_H

/*
 *
 * nucleo/gl/window/glWindow.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveObject.H>
#include <nucleo/gl/glIncludes.H>
#include <nucleo/gl/window/keysym.H>

#include <string>
#include <iostream>

namespace nucleo {

  // ------------------------------------------------------------------

  class glWindow : public ReactiveObject {

  private:

  protected:

  public:

    bool debugEvents ;
    bool debugExtInput ;

    // -------------------------------------------------
    // Options that can be used for GL setup

    enum glOptions {
	 NONE = 0,
	 DOUBLE_BUFFER = 1,
	 STENCIL = 2,
	 DEPTH = 4,
	 ALPHA = 8,
	 NONRESIZABLE = 16,
	 FLOATING = 32,
    } ;

    // -------------------------------------------------
    // Screen-related utilities

    virtual unsigned int getScreenWidth(void) = 0 ;
    virtual unsigned int getScreenHeight(void) = 0 ;

    virtual void setFullScreen(bool activate) = 0 ;

    // -------------------------------------------------
    // Window-related utilities

    virtual void map(void) = 0 ;
    virtual void unmap(void) = 0 ;

    void setTitle(std::string title) { setTitle(title.c_str()) ; }
    virtual void setTitle(const char *title) = 0 ;

    virtual void getGeometry(unsigned int *width, unsigned int *height,
					    int *x=0, int *y=0) = 0 ;
    // x and y can be negative (relative to parent's width and height)
    virtual void setGeometry(unsigned int width, unsigned int height,
					    int x, int y) = 0 ;
    virtual void setGeometry(unsigned int width, unsigned int height) = 0;
    virtual void setGeometry(const char *geometry) ;

    virtual void raise(void) = 0 ;
    virtual void lower(void) = 0 ;

    virtual void move(int newx, int newy) ;
    virtual void moveRel(int dx, int dy) ;

    // -------------------------------------------------
    // GL context manipulation

    virtual void makeCurrent(void) = 0 ;
    virtual void swapBuffers(void) = 0 ;

    // -------------------------------------------------
    // Bonus

    virtual void setCursorVisible(bool activate) {
      std::cerr << "glWindow::setCursorVisible is not implemented on this platform..." << std::endl ;
    }
    virtual void warpCursor(int x, int y) {
	 // x and y are in window coordinates
      std::cerr << "glWindow::warpCursor is not implemented on this platform..." << std::endl ;
    }

    virtual void setKeyboardAutoRepeat(bool activate) {
      std::cerr << "glWindow::setKeyboardAutoRepeat is not implemented on this platform..." << std::endl ;
    }

    virtual void syncToVbl(int nb_vbls=1) {
      std::cerr << "glWindow::syncToVbl is not implemented on this platform..." << std::endl ;
    }

    virtual void setAspectRatio(int width, int height) {
      std::cerr << "glWindow::setAspectRatio is not implemented on this platform..." << std::endl ;
    }

    virtual void setMinMaxSize(int minwidth, int minheight, int maxwidth, int maxheight) {
      std::cerr << "glWindow::setMinMaxSize is not implemented on this platform..." << std::endl ;
    }

    virtual float getAlpha(void) {
	 return 1.0 ;
    }
    virtual void setAlpha(float alpha) {
      std::cerr << "glWindow::setAlpha is not implemented on this platform..." << std::endl ;
    }

    virtual void emulateThreeButtonMouse(bool activate) {
      std::cerr << "glWindow::emulateThreeButtonMouse is not implemented on this platform..." << std::endl ;
    }

    virtual bool getPtrAccel(int *accelNum, int *accelDenom, int *threshold)
    {
	    std::cerr << "glWindow::getPtrAccel is not implemented on this platform..." << std::endl ;
	    return false;
    }
    virtual bool setPtrAccel(int accelNum, int accelDenom, int threshold)
    {
	    std::cerr << "glWindow::setPtrAccel is not implemented on this platform..." << std::endl ;
	    return false;
    }
    
    // -------------------------------------------------
    // Event handling

    struct event {
	 static const char * const strings[] ;

	 enum {
#define EVENTDEF(eventname,eventnum) eventname = eventnum,
#include <nucleo/gl/window/_events.H>
#undef EVENTDEF
	 } type ;

	 // If you add more parameters here, change the copy constructor
	 // implementation accordingly...

	 // Window
	 int width, height ;

	 // Mouse (pointer, buttons, wheels)
	 int x, y ;
	 int button ;
	 int axis, delta ;

	 // Keyboard
	 unsigned long keysym ;
	 std::string keystr ;

	 // Extension events (glWindow_GLX only)
	 enum {
#define EVENTDEF(eventname,eventnum) eventname = eventnum,
#include <nucleo/gl/window/_extEvents.H>
#undef EVENTDEF
	 } extType ;
	 int device_id ;
#define N_MAX_AXES_COUNT 6
	 int axes_count, first_axis, axis_data[N_MAX_AXES_COUNT];

	 // WTF? This one seems to be used in glWindow_GLX only...
	 int time ; 

	 event(void) { keystr = "<?>" ; }
	 event& operator = (event& src) ;
	 void debug(std::ostream& out) const ;
	 ~event() ;
    } ;

    // -------------------------------------------------
    // extension device

    class extensionDevice {
    public:
	 typedef enum {
	   HAS_KEYS=(1<<0),
	   HAS_BUTTONS=(1<<1),
	   HAS_VALUATORS=(1<<2),
	   HAS_PROXIMITY=(1<<3)
	 } properties ;
    protected:
	 unsigned long int _id;
	 char *_name;
	 unsigned long _dev_type;
	 unsigned long _selectedEvents;
    public: 
	 extensionDevice(unsigned long id, unsigned long dev_type, char *name) ;
	 ~extensionDevice(void) ;

	 char *getName(void) ;
	 unsigned long getID(void) ;

	 bool hasKeys(void) ;
	 bool hasButtons(void) ;
	 bool hasValuators(void) ;
	 bool hasProximity(void) ;
    } ;

    typedef std::list<extensionDevice *> extDevicesList ;

    virtual extDevicesList *getExtensionDevices(void) {
       std::cerr << "glWindow::getExtensionDevices is not implemented on this platform..." << std::endl ;
       return 0;
    }
    virtual void selectExtensionEvent(extensionDevice *extDev, unsigned int events, bool onRootToo) {
    }
    virtual bool changeCorePointer(extensionDevice *extDev, int xaxis = 0, int yaxis = 1) {
	    return false;
    }
    virtual bool getExtensionPtrAccel(extensionDevice *extDev, int *accelNum, int *accelDenom, int *threshold) {
	    return false;
    }
    virtual bool setExtensionPtrAccel(extensionDevice *extDev, int accelNum, int accelDenom, int threshold) {
	    return false;
    }   
    virtual void restoreCorePointer(void) {
    }

    // -------------------------------------------------

    virtual bool getNextEvent(event *) = 0 ;

    // don't forget to delete the event when you're done with it...
    virtual event *getNextEvent(void) = 0 ;

    static glWindow *create(long options, long eventmask) ;

  } ;

  // ------------------------------------------------------------------

  bool
  parseGeometry(const char *geometry,
			 unsigned int *width, unsigned int *height,
			 int *x=0, int *y=0) ;

  // ------------------------------------------------------------------

}

#endif
