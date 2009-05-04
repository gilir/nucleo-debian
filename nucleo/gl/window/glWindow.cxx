/*
 *
 * nucleo/gl/window/glWindow.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/gl/window/glWindow.H>

#if HAVE_AGL
#include <nucleo/gl/window/glWindow_AGL.H>
#endif

#if HAVE_GLX
#include <nucleo/gl/window/glWindow_GLX.H>
#endif

#include <cstring>

namespace nucleo {

  const char * const glWindow::event::strings[] = {
#define EVENTDEF(eventname,eventnum) #eventname,
#include <nucleo/gl/window/_events.H>
#undef EVENTDEF
  } ;

  glWindow::event&
  glWindow::event::operator = (glWindow::event& src) {
    if (&src!=this) {
	 type = src.type ;
	 width = src.width ;
	 height = src.height ;
	 x = src.x ;
	 y = src.y ;
	 button = src.button ;
	 axis = src.axis ;
	 delta = src.delta ;
	 time = src.time;
	 keysym = src.keysym ;
	 keystr = src.keystr ;

	 extType = src.extType;
	 device_id = src.device_id;
	 axes_count = src.axes_count;
	 first_axis = src.first_axis;
	 for(int i = 0; i < N_MAX_AXES_COUNT; i++)
	   axis_data[i] = src.axis_data[i];
    }
    return *this ;
  }

  void
  glWindow::event::debug(std::ostream& out) const {
    out << "[" ;
    switch (type) {
#define EVENTDEF(eventname,eventnum) case eventnum: out << #eventname ; break ;
#include <nucleo/gl/window/_events.H>
#undef EVENTDEF
    }
    switch (type) {
    case configure:
	 out << " x=" << x << " y=" << y << " w=" << width << " h=" << height ;
	 break ;
    case expose:
    case destroy:
    case enter:
    case leave:
	 break ;
    case keyPress:
    case keyRelease:
	 out << " sym=" << keysym << " str=" << keystr ;
	 break ;
    case buttonPress: 
    case buttonRelease:
	 out << " x=" << x << " y=" << y << " b=" << button ;
	 break ;
    case pointerMotion:
	 out << " x=" << x << " y=" << y ;
	 break ;
    case wheelMotion:
	 out << " a=" << axis << " d=" << delta ;
	 break ;
    case focusIn:
    case focusOut:
      break;
    case extensionEvent:
	    switch (extType)
	    {
#define EVENTDEF(eventname,eventnum) case eventnum: out << #eventname ; break ;
#include <nucleo/gl/window/_extEvents.H>
#undef EVENTDEF
	    }
	    switch(extType)
	    {
	    case extMotionNotify:
		    out << " axes_count=" << axes_count << " first_axis="
			<< first_axis;
		    for(int i = 0;
			i < axes_count && i < N_MAX_AXES_COUNT; i++)
		    {
			    out << " a" << (i+first_axis) << "=" << axis_data[i];
		    }
		    break;
	    case extKeyPress:
	    case extKeyRelease:
	      out << " sym=" << keysym << " str=" << keystr ;
	      break;
	    case extButtonPress: 
	    case extButtonRelease:
	      out << " b=" << button ;
	    case extStateNotify:
	    case extProximityIn:
	    case extProximityOut:
		    break;
	    default:
		    out << " Should not happen";
		    break;
	    }
    }
    out << "]" << std::flush ;
  }

  glWindow::event::~event() {
  }

  // ------------------------------------------------------------------

  void
  glWindow::setGeometry(const char *geometry) {
#if 1
    unsigned int width, height ;
    int x, y ;
    getGeometry(&width, &height, &x, &y) ;
    parseGeometry(geometry, &width, &height, &x, &y) ;
    setGeometry(width, height, x, y) ;
#else
    // std::cerr << "glWindow::setGeometry: " << geometry << std::endl ;

    // [=][<width>{xX}<height>][{+-}<xoffset>{+-}<yoffset>]

    char *ptr = (char *)geometry ;
    std::string number ;
    unsigned int width=0, height=0 ;
    int x=0, y=0, sign=1 ;
    bool hasPosition = false;

    getGeometry(&width, &height, &x, &y) ;

    if (*ptr=='=') ptr++ ;
    if (*ptr=='+' || *ptr=='-') goto position ;

    while (*ptr!='x' && *ptr!='X') {
	 if (*ptr=='\0') goto abort ;
	 number = number + (*ptr++) ;
    }
    width = (unsigned int) atoi(number.c_str()) ;

    number = "" ; ptr++ ; // skip the x (or X)

    while (*ptr!='\0' && *ptr!='+' && *ptr!='-') 
	 number = number + (*ptr++) ;
    height = (unsigned int) atoi(number.c_str()) ;
    if (*ptr=='\0') goto end ;

  position:

    sign = (*ptr=='-') ? -1 : 1 ;
    number = "" ; ptr++ ; // skip the sign

    while (*ptr!='+' && *ptr!='-'
		 ) {
	 if (*ptr=='\0') goto abort ;
	 number = number + (*ptr++) ;
    }
    x = sign * atoi(number.c_str()) ;
    
    sign = (*ptr=='-') ? -1 : 1 ;
    number = "" ; ptr++ ; // skip the sign
    while (*ptr!='\0')
	 number = number + (*ptr++) ;
    y = sign * atoi(number.c_str()) ;
    hasPosition = true;

  end:
    // std::cerr << "glWindow::setGeometry: " << width << "x" << height << " " << x << "," << y << std::endl ;
    if (hasPosition)
      setGeometry(width, height, x, y) ;
    else
      setGeometry(width, height);

  abort:
    return ;
#endif
  }

#if 0
  void
  glWindow::resize(unsigned int newwidth, unsigned int newheight) {
    unsigned int width, height ;
    int x, y ;
    getGeometry(&width, &height, &x, &y) ;
    // std::cerr << "x=" << x << " y=" << y << " w=" << width << " h=" << height << std::endl ;
    setGeometry(newwidth, newheight, x, y) ;
  }
#endif

  void
  glWindow::move(int newx, int newy) {
    unsigned int width, height ;
    int x, y ;
    getGeometry(&width, &height, &x, &y) ;
    setGeometry(width, height, newx, newy) ;
  }

  void
  glWindow::moveRel(int dx, int dy) {
    unsigned int width, height ;
    int x, y ;
    getGeometry(&width, &height, &x, &y) ;
    setGeometry(width, height, x+dx, y+dy) ;
  }

  // ------------------------------------------------------------------

  glWindow *
  glWindow::create(long options, long eventmask) {
#if HAVE_AGL
    return new glWindow_AGL(options,eventmask) ;
#elif HAVE_GLX
    return new glWindow_GLX(options, eventmask) ;
#endif
    return (glWindow*)0 ;
  }

  // ------------------------------------------------------------------

  glWindow::extensionDevice::extensionDevice(unsigned long id, unsigned long dev_type, char *name) {
    _id = id;
    _dev_type = dev_type;
    _selectedEvents = 0;
    if (name)
	 _name = strdup(name) ;
    else
	 _name = 0 ;
  }

  glWindow::extensionDevice::~extensionDevice(void) {
    if (_name) free(_name) ;
    _name = 0 ;
  }

  char *
  glWindow::extensionDevice::getName(void) {
    return _name ;
  }

  unsigned long
  glWindow::extensionDevice::getID(void) {
    return _id ;
  }

  bool
  glWindow::extensionDevice::hasKeys(void) { 
    return (HAS_KEYS & _dev_type)? true:false;
  }

  bool
  glWindow::extensionDevice::hasButtons(void) { 
    return (HAS_BUTTONS & _dev_type)? true:false;
  }

  bool
  glWindow::extensionDevice::hasValuators(void) { 
    return (HAS_VALUATORS & _dev_type)? true:false;
  }

  bool
  glWindow::extensionDevice::hasProximity(void) { 
    return (HAS_PROXIMITY & _dev_type)? true:false;
  }

  // ------------------------------------------------------------------

  bool
  parseGeometry(const char *geometry,
			 unsigned int *width, unsigned int *height,
			 int *x, int *y) {
    // [=][<width>{xX}<height>][{+-}<xoffset>{+-}<yoffset>]

    char *ptr = (char *)geometry ;
    std::string number ;

    if (*ptr=='=') ptr++ ;

    if (*ptr!='+' && *ptr!='-') {
	 while (*ptr!='x' && *ptr!='X') {
	   if (*ptr=='\0') return false ;
	   number = number + (*ptr++) ;
	 }
	 if (width) *width = (unsigned int) atoi(number.c_str()) ;

	 number = "" ; ptr++ ; // skip the x (or X)

	 while (*ptr!='\0' && *ptr!='+' && *ptr!='-') 
	   number = number + (*ptr++) ;
	 if (height) *height = (unsigned int) atoi(number.c_str()) ;
	 if (*ptr=='\0') return true ;
    }

    int sign = (*ptr=='-') ? -1 : 1 ;
    number = "" ; ptr++ ; // skip the sign

    while (*ptr!='+' && *ptr!='-') {
	 if (*ptr=='\0') return false ;
	 number = number + (*ptr++) ;
    }
    if (x) *x = sign * atoi(number.c_str()) ;
    
    sign = (*ptr=='-') ? -1 : 1 ;
    number = "" ; ptr++ ; // skip the sign
    while (*ptr!='\0')
	 number = number + (*ptr++) ;
    if (y) *y = sign * atoi(number.c_str()) ;

    return true ;
  }

  // ------------------------------------------------------------------

}
