/*
 *
 * nucleo/gl/window/glWindow_AGL.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>
#include <nucleo/nucleo.H>
#include <nucleo/gl/window/glWindow_AGL.H>
#include <nucleo/core/ReactiveEngine.H>

#include <OpenGL/OpenGL.h>

// for EnterMovies...
#include <QuickTime/QuickTime.h> 
// For CG*
#include <ApplicationServices/ApplicationServices.h>

#include <stdexcept>

// Window server private stuff
// extern "C" OSErr CPSSetProcessName (ProcessSerialNumber *psn, char *processname) ;
extern "C" OSErr CPSEnableForegroundOperation(ProcessSerialNumber *psn) ;
// extern "C" OSErr CPSEnableForegroundOperation(ProcessSerialNumber *psn, UInt32 _arg2, UInt32 _arg3, UInt32 _arg4, UInt32 _arg5) ;

#include <AvailabilityMacros.h>

#define DEBUG_LEVEL 0
// #define LEOPARD_SUCKS 1

namespace nucleo {

  bool glWindow_AGL::wsConnected = false ;

  static inline void
  debugModifiers(std::string msg, UInt32 modifiers) {
    std::cerr << "kEventClassKeyboard: " << msg << " " << modifiers << std::endl ;
    std::cerr << "   cmdKey: " << (modifiers & cmdKey) << std::endl ;
    std::cerr << "   shiftKey: " << (modifiers & shiftKey) << std::endl ;
    std::cerr << "   alphaLock: " << (modifiers & alphaLock) << std::endl ;
    std::cerr << "   optionKey: " << (modifiers & optionKey) << std::endl ;
    std::cerr << "   controlKey: " << (modifiers & controlKey) << std::endl ;
    std::cerr << "   rightShiftKey: " << (modifiers & rightShiftKey) << std::endl ;
    std::cerr << "   rightOptionKey: " << (modifiers & rightOptionKey) << std::endl ;
    std::cerr << "   rightControlKey: " << (modifiers & rightControlKey) << std::endl ;
    
    std::cerr << "   NumLock: " << (modifiers & kEventKeyModifierNumLockMask) << std::endl ;
    std::cerr << "   Fn: " << (modifiers & kEventKeyModifierFnMask) << std::endl ;
  }

  // -----------------------------------------------------------------------

  glWindow
  *createAGLwindow(long options, long eventmask) {
    return new glWindow_AGL(options, eventmask) ;
  }

  // -----------------------------------------------------------------------

  OSStatus
  glWindow_AGL::_windowEventHandler(EventHandlerCallRef nextHandler,
							 EventRef theEvent,
							 void* userData) {
    // std::cerr << "glWindow_AGL::_windowEventHandler: begin" << std::endl ;

    glWindow_AGL *obj = (glWindow_AGL*) userData ;

    // -- Standard handler ---

    // If we're fullscreen, standard handlers need not be called...
    if (! (obj->_fsContext)) {
	 OSStatus result = CallNextEventHandler(nextHandler, theEvent) ;
	 if (result!=eventNotHandledErr && result!=noErr) {
	   std::cerr << "glWindow_AGL: next event handler returned " << result << std::endl ;
	   return result ;
	 }
    }

    // -----------------------

    UInt32 eclass=GetEventClass(theEvent),
	 ekind=GetEventKind(theEvent) ;

    bool eventHandled = true ;

    switch(eclass) {

    case kEventClassMouse :
	 switch(ekind){

	 case kEventMouseDown :
	 case kEventMouseUp : {
	   Point point ;
	   GetEventParameter(theEvent, kEventParamMouseLocation, typeQDPoint, NULL,
					 sizeof(Point), NULL, &point) ;
	   if (obj->_fsContext || (FindWindow(point, &(obj->_window)) == inContent)) {
		glWindow::event *e = new glWindow::event ;
		e->type = (ekind==kEventMouseDown) ? glWindow::event::buttonPress : glWindow::event::buttonRelease;
		e->x = point.h ;
		e->y = point.v ;
		if (! obj->_fsContext) {
		  e->x -= obj->_rect.left ;
		  e->y -= obj->_rect.top ;
		}
		EventMouseButton mouseButton ;
		GetEventParameter(theEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &mouseButton) ;
		if (mouseButton!=1 || !obj->_emulateThreeButtonMouse)
		  e->button = mouseButton ;
		else {
		  if (obj->_modifiers&cmdKey) e->button = 3 ;
		  else if (obj->_modifiers&optionKey) e->button = 2 ;
		  else e->button = 1 ;
		}
		obj->_equeue.push(e) ;
	 }
#if DEBUG_LEVEL>0
	   else
		std::cerr << "kEventMouseDown|kEventMouseUp: " << point.h << "," << point.v << std::endl ;
#endif
	 } break;          

	 case kEventMouseMoved :
	 case kEventMouseDragged : {
	   Point point;
	   GetEventParameter(theEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &point);
	   if (obj->_fsContext || (FindWindow(point, &(obj->_window)) == inContent)) {
		glWindow::event *e = new glWindow::event ;
		e->type = glWindow::event::pointerMotion ;
		e->x = point.h ;
		e->y = point.v ;
		if (! obj->_fsContext) {
		  e->x -= obj->_rect.left ;
		  e->y -= obj->_rect.top ;
		}
		obj->_equeue.push(e) ;
	   }
#if DEBUG_LEVEL>0
	   else
		std::cerr << "kEventMouseMoved|kEventMouseDragged: " << point.h << "," << point.v << std::endl ;
#endif
	 } break;

	 case kEventMouseWheelMoved : {
	   EventMouseWheelAxis axis ;
	   GetEventParameter(theEvent, kEventParamMouseWheelAxis, typeMouseWheelAxis, NULL, sizeof(axis), NULL, &axis ) ;

	   SInt32 delta ;
	   GetEventParameter(theEvent, kEventParamMouseWheelDelta, typeSInt32, NULL, sizeof(delta), NULL, &delta ) ;

	   glWindow::event *e = new glWindow::event ;
	   e->type = glWindow::event::wheelMotion ;
	   e->axis = (int)axis ;
	   e->delta = (int)delta ;
	   obj->_equeue.push(e) ;
	 } break ;

	 default :
	   eventHandled = false ;
	   break ;
	 }
	 break;

    case kEventClassKeyboard :
	 switch(ekind){
	 case kEventRawKeyDown :
	 case kEventRawKeyRepeat:
	 case kEventRawKeyUp : {
	   glWindow::event *e = new glWindow::event ;

	   if (ekind==kEventRawKeyDown)
		e->type = glWindow::event::keyPress ;
	   else // Up or Repeat
		e->type = glWindow::event::keyRelease ;

	   char charCode ;
	   GetEventParameter(theEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char), NULL, &charCode);
	   UInt32 keyCode ;
	   GetEventParameter(theEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &keyCode);

	   if (obj->_modifiers & controlKey) {
		UInt32 dummy_state = 0;
		Handle transData = (Handle) GetScriptManagerVariable(smKCHRCache);
		UInt32 result = KeyTranslate(transData,
							    keyCode | (obj->_modifiers & ~controlKey),
							    &dummy_state) ;
		UInt32 lowChar =  result &  0x000000FF ;
		UInt32 highChar = (result & 0x00FF0000) >> 16 ;
		if (lowChar != 0) charCode = (char)lowChar;
		else if (highChar != 0) charCode = (char)highChar;
	   }

#if DEBUG_LEVEL>9
		std::cerr << "DEBUG: charCode=" << (int)charCode << " keyCode=" << (unsigned int)keyCode
				<< std::endl ;
#endif

	   bool unknown_key = false ;

	   switch(charCode) {
	   case 16:
		switch(keyCode) {
		case 122: e->keysym = XK_F1 ; e->keystr = "<F1>" ; break ;
		case 120: e->keysym = XK_F2 ; e->keystr = "<F2>" ; break ;
		case 99: e->keysym = XK_F3 ; e->keystr = "<F3>" ; break ;
		case 118: e->keysym = XK_F4 ; e->keystr = "<F4>" ; break ;
		case 96: e->keysym = XK_F5 ; e->keystr = "<F5>" ; break ;
		case 97: e->keysym = XK_F6 ; e->keystr = "<F6>" ; break ;
		case 98: e->keysym = XK_F7 ; e->keystr = "<F7>" ; break ;
		case 100: e->keysym = XK_F8 ; e->keystr = "<F8>" ; break ;
		case 101: e->keysym = XK_F9 ; e->keystr = "<F9>" ; break ;
		case 109: e->keysym = XK_F10 ; e->keystr = "<F10>" ; break ;
		case 103: e->keysym = XK_F11 ; e->keystr = "<F11>" ; break ;
		case 111: e->keysym = XK_F12 ; e->keystr = "<F12>" ; break ;
		default: unknown_key = true ; break ;
		}
		break ;
#define KEYTRANS(cc,sym,str) case cc: e->keysym=sym; e->keystr=str ; break ;
#include <nucleo/gl/window/_macos2keysym.H>
#undef KEYTRANS
	   default:
		unknown_key = true ;
		break;
	   }

	   if (unknown_key) {
		std::cerr << "glWindow_AGL: unknown MacOS key (c=" << (int)charCode << ",k=" << keyCode << ")" << std::endl;
	   } else {
		obj->_equeue.push(e) ;
		if (ekind==kEventRawKeyRepeat) {
		  glWindow::event *e2 = new glWindow::event ;
		  e2->type = glWindow::event::keyPress ;
		  e2->keysym = e->keysym ;
		  e2->keystr = e->keystr ;
		  obj->_equeue.push(e2) ;
		}
	   }
	   
	 } break;

	 case kEventRawKeyModifiersChanged: {
	   UInt32 m ;
	   GetEventParameter(theEvent, kEventParamKeyModifiers,typeUInt32,NULL,sizeof(UInt32),NULL,&m) ;

	   UInt32 mdelta = m^obj->_modifiers ;
#if DEBUG_LEVEL>10
	   debugModifiers("modifiers = ",m) ;
#endif
	   obj->_modifiers = m ;

	   /*
	    * Apple's X11 server uses this binding:
	    *    Right command (Apple) -> Meta_L
	    *    Left command (Apple) -> Meta_L
	    *    alt -> Mode_switch
	    *    ctrl -> Control_L
	    *    fn -> 
	    */

	   if (mdelta & cmdKey) {
		glWindow::event *e = new glWindow::event ;
		e->type = (m&cmdKey) ? event::keyPress : event::keyRelease ;
#if 0
		e->keysym = XK_Meta_L ;
		e->keystr = "<LeftMeta>" ;
#else
		e->keysym = XK_Super_L ;
		e->keystr = "<LeftSuper>" ;
#endif
		obj->_equeue.push(e) ;
	   } else if (mdelta & shiftKey) {
		glWindow::event *e = new glWindow::event ;
		e->type = (m&shiftKey) ? event::keyPress : event::keyRelease ;
		e->keysym = XK_Shift_L ;
		e->keystr = "<LeftShift>" ;
		obj->_equeue.push(e) ;
	   } else if (mdelta & rightShiftKey) {
		glWindow::event *e = new glWindow::event ;
		e->type = (m&rightShiftKey) ? event::keyPress : event::keyRelease ;
		e->keysym = XK_Shift_R ;
		e->keystr = "<RightShift>" ;
		obj->_equeue.push(e) ;
	   } else if (mdelta & controlKey) {
		glWindow::event *e = new glWindow::event ;
		e->type = (m&controlKey) ? event::keyPress : event::keyRelease ;
		e->keysym = XK_Control_L ;
		e->keystr = "<LeftControl>" ;
		obj->_equeue.push(e) ;
	   } else if (mdelta & rightControlKey) {
		glWindow::event *e = new glWindow::event ;
		e->type = (m&rightControlKey) ? event::keyPress : event::keyRelease ;
		e->keysym = XK_Control_R ;
		e->keystr = "<RightControl>" ;
		obj->_equeue.push(e) ;
	   } else if (mdelta & optionKey) {
		glWindow::event *e = new glWindow::event ;
		e->type = (m&optionKey) ? event::keyPress : event::keyRelease ;
		e->keysym = XK_Alt_L ;
		e->keystr = "<LeftAlt>" ;
		obj->_equeue.push(e) ;
	   } else if (mdelta & rightOptionKey) {
		glWindow::event *e = new glWindow::event ;
		e->type = (m&rightOptionKey) ? event::keyPress : event::keyRelease ;
		e->keysym = XK_Alt_R ;
		e->keystr = "<RightAlt>" ;
		obj->_equeue.push(e) ;
	   } else if (mdelta & kEventKeyModifierFnMask) {
		glWindow::event *e = new glWindow::event ;
		e->type = (m&kEventKeyModifierFnMask) ? event::keyPress : event::keyRelease ;
		e->keysym = XK_Hyper_L ;
		e->keystr = "<LeftHyper>" ;
		obj->_equeue.push(e) ;
	   } else if (mdelta & alphaLock) {
		glWindow::event *e = new glWindow::event ;
		e->type = (m&alphaLock) ? event::keyPress : event::keyRelease ;
		e->keysym = XK_Caps_Lock ;
		e->keystr = "<CapsLock>" ;
		obj->_equeue.push(e) ;
	   } else if (mdelta & kEventKeyModifierNumLockMask) {
		glWindow::event *e = new glWindow::event ;
		e->type = (m&kEventKeyModifierNumLockMask) ? event::keyPress : event::keyRelease ;
		e->keysym = XK_Num_Lock ;
		e->keystr = "<NumLock>" ;
		obj->_equeue.push(e) ;
	   }
	 } break ;
	 default :
	   eventHandled = false ;
	   break;
	 }
	 break ;

    case kEventClassWindow :
	 switch(ekind){
	 case kEventWindowInit: std::cerr << "kEventWindowInit" << std::endl ; break ;
	 case kEventWindowClosed : {
	   glWindow::event *e = new glWindow::event ;
	   e->type = glWindow::event::destroy;
	   obj->_equeue.push(e) ;
	 } break ;
	 case kEventWindowDrawContent : {
	   std::cerr << "kEventWindowDrawContent" << std::endl ;
	   glWindow::event *e = new glWindow::event ;
	   e->type = glWindow::event::expose ;
	   obj->_equeue.push(e) ;
	 } break;          
	 case kEventWindowBoundsChanging :
	   if (obj->_aspect 
		  || obj->_minWidth!=0 || obj->_minHeight!=0 
		  || obj->_maxWidth!=0 || obj->_maxHeight!=0) {
		Rect rect ;
		GetEventParameter(theEvent, kEventParamCurrentBounds,
					   typeQDRectangle, NULL, sizeof(Rect), NULL, &rect);
		UInt32 width = rect.right - rect.left;
		UInt32 height = rect.bottom - rect.top;
		bool corrected = false ;

		if (obj->_aspect) {
		  UInt32 reqWidth = (unsigned int)(height*obj->_aspect) ;
		  if (width != reqWidth) {
		    corrected = true ;
		    rect.right = rect.left + reqWidth ;
		  }
		} else {
		  if (obj->_minWidth!=0 && width<obj->_minWidth) {
		    rect.right = rect.left+obj->_minWidth ;
		    corrected=true ;
		  }
		  if (obj->_minHeight!=0 && height<obj->_minHeight) {
		    rect.bottom = rect.top+obj->_minHeight ;
		    corrected=true ;
		  }
		  if (obj->_maxWidth!=0 && width>obj->_maxWidth) {
		    rect.right = rect.left+obj->_maxWidth ;
		    corrected=true ;
		  }
		  if (obj->_maxHeight!=0 && height>obj->_maxHeight) {
		    rect.bottom = rect.top+obj->_maxHeight ;
		    corrected=true ;
		  }
		}

		if (corrected)
		  SetEventParameter(theEvent, kEventParamCurrentBounds,
						typeQDRectangle, sizeof(Rect), &rect) ;
	   }
	   break ;
	 case kEventWindowBoundsChanged :
	   if (!obj->_fsContext) {
		GetWindowBounds(obj->_window, kWindowContentRgn, &obj->_rect) ;
		aglSetCurrentContext(obj->_context) ;
		aglUpdateContext(obj->_context) ;
		glWindow::event *e = new glWindow::event ;
		e->type = glWindow::event::configure ;
		e->x = obj->_rect.left ;
		e->y = obj->_rect.top ;
		e->width = obj->_rect.right - obj->_rect.left ;
		e->height = obj->_rect.bottom - obj->_rect.top ;
		obj->_equeue.push(e) ;
	   }
	   break;
	 default:
	   eventHandled = false ;
	   break ;
	 }
	 break;	 

    default :
	 eventHandled = false ;
	 break ;
    }

    // std::cerr << "glWindow_AGL::_windowEventHandler: end" << std::endl ;

    if (eventHandled) {
	 obj->notifyObservers() ;
	 return noErr ;
    }

    char *ptr = (char *)&eclass ;
    std::cerr << "unexpected event: " ;
    std::cerr << "'" << ptr[0] << ptr[1] << ptr[2] << ptr[3] << "' (" << eclass << ")" ;
    std::cerr << " / " << ekind << std::endl ;
    return eventNotHandledErr ;
  }

  AGLPixelFormat
  glWindow_AGL::getPixelFormat(bool fullscreen) {
#if defined(MAC_OS_X_VERSION_10_5) && !defined(LEOPARD_SUCKS)
    CGDirectDisplayID displayID = CGMainDisplayID() ;
    CGOpenGLDisplayMask openGLDisplayMask = CGDisplayIDToOpenGLDisplayMask(displayID) ;
    int attributeListSize = 3+1 ;
    GLint attributeList[] = { AGL_RGBA,
						AGL_DISPLAY_MASK, openGLDisplayMask,
#else
    int attributeListSize = 1+1 ;
    GLint attributeList[] = { AGL_RGBA,
#endif
						AGL_NO_RECOVERY, // No software fallback
						// -----
						AGL_NONE, // placeholder for AGL_FULLSCREEN
						AGL_NONE, // placeholder for AGL_DOUBLEBUFFER
						AGL_NONE, // placeholders for AGL_DEPTH_SIZE
						AGL_NONE,
						AGL_NONE, // placeholders for AGL_STENCIL_SIZE
						AGL_NONE,
						AGL_NONE, // placeholders for AGL_ALPHA_SIZE
						AGL_NONE,
						AGL_NONE,
						AGL_NONE,
						AGL_NONE,
						AGL_NONE,
						AGL_NONE,
						AGL_NONE,
						AGL_NONE,
						AGL_NONE,
    };

    if (fullscreen)
	 attributeList[attributeListSize++] = AGL_FULLSCREEN ;

    if (_options&glWindow::DOUBLE_BUFFER)
	 attributeList[attributeListSize++] = AGL_DOUBLEBUFFER ;

    if (_options&glWindow::DEPTH) {
	 attributeList[attributeListSize++] = AGL_DEPTH_SIZE ;
	 attributeList[attributeListSize++] = 32 ; // was 24
    }

    if (_options&glWindow::STENCIL) {
	 attributeList[attributeListSize++] = AGL_STENCIL_SIZE ;
	 attributeList[attributeListSize++] = 8 ;
    }

    if (_options&glWindow::ALPHA) {
	 attributeList[attributeListSize++] = AGL_ALPHA_SIZE ;
	 attributeList[attributeListSize++] = 8 ;
    }

#if defined(MAC_OS_X_VERSION_10_5) && !defined(LEOPARD_SUCKS)
    AGLPixelFormat apf = aglCreatePixelFormat(attributeList) ;
#else
    GDHandle device = GetMainDevice();
    AGLPixelFormat apf = aglChoosePixelFormat(&device, 1, attributeList) ;
#endif

    if (!apf)
	 throw std::runtime_error("glWindow_AGL: no suitable AGL pixel format") ;
      
#if DEBUG_LEVEL>=2
    {
	 GLint id, red, green, blue, depth, alpha, stencil, fullscreen ;
	 aglDescribePixelFormat(apf, AGL_RENDERER_ID, &id) ;
	 aglDescribePixelFormat(apf, AGL_RED_SIZE, &red) ;
	 aglDescribePixelFormat(apf, AGL_GREEN_SIZE, &green) ;
	 aglDescribePixelFormat(apf, AGL_BLUE_SIZE, &blue) ;
	 aglDescribePixelFormat(apf, AGL_ALPHA_SIZE, &alpha) ;
	 aglDescribePixelFormat(apf, AGL_DEPTH_SIZE, &depth) ;
	 aglDescribePixelFormat(apf, AGL_STENCIL_SIZE, &stencil) ;
	 aglDescribePixelFormat(apf, AGL_FULLSCREEN, &fullscreen) ;
	 std::cerr << "glWindow_AGL:"
			 << " id=" << id
			 << " r=" << red << " g=" << green << " b=" << blue
			 << " a=" << alpha
			 << " depth=" << depth
			 << " stencil=" << stencil
			 << " fullscreen=" << fullscreen
			 << std::endl ;
    }
#endif

    return apf ;
  }

  // -----------------------------------------------------------------------

  // http://developer.apple.com/documentation/Carbon/Reference/Pasteboard_Reference/index.html
  // http://developer.apple.com/documentation/Carbon/Conceptual/Pasteboard_Prog_Guide/index.html
  // file:///System/Library/Frameworks/ApplicationServices.framework/Versions/Current/Frameworks/LaunchServices.framework/Headers/UTType.h

#if 0
  static OSErr
  __dragReceive(WindowRef inWindow, void *inUserData, DragRef inDrag) {
    std::cerr << "__dragReceive, line " << __LINE__ << std::endl ;

    PasteboardRef pasteboard ;
    OSStatus err = GetDragPasteboard(inDrag, &pasteboard) ;
    if (err!=noErr) return err ;

    PasteboardSynchronize(pasteboard) ;

    ItemCount itemCount ;
    err = PasteboardGetItemCount(pasteboard, &itemCount) ;
    if (err!=noErr) return err ;

    std::cerr << "PasteboardRef: " << (int)pasteboard 
		    << " has " << (int)itemCount << " item(s)" << std::endl ;

    for (UInt32 itemIndex = 1; itemIndex <= itemCount; itemIndex++) {
	 PasteboardItemID itemID ;
	 err = PasteboardGetItemIdentifier(pasteboard, itemIndex, &itemID) ;

	 CFArrayRef flavorTypeArray ;
	 err = PasteboardCopyItemFlavors(pasteboard, itemID, &flavorTypeArray) ;

	 CFIndex flavorCount = CFArrayGetCount(flavorTypeArray) ;
	 for (CFIndex flavorIndex = 0; flavorIndex<flavorCount; flavorIndex++) {
	   char tmp[128] ;
	   CFStringRef flavorType = (CFStringRef)CFArrayGetValueAtIndex(flavorTypeArray, flavorIndex) ;
	   CFStringGetCString(flavorType, tmp, 128, kCFStringEncodingMacRoman) ;
	   std::cerr << "    " << tmp << std::endl ;

	   CFStringRef tmpStr ;

	   tmpStr = UTTypeCopyPreferredTagWithClass(flavorType, kUTTagClassOSType) ;
	   if (tmpStr) {
		CFStringGetCString(tmpStr, tmp, 128, kCFStringEncodingMacRoman) ;
		std::cerr << "         kUTTagClassOSType: " << tmp << std::endl ;
	   }

	 }

#if 1
	 CFDataRef flavorData ;
	 err = PasteboardCopyItemFlavorData(pasteboard, itemID, CFSTR("public.url"), &flavorData) ;
	 
	 if (err==noErr)
	   std::cerr << "YOUPEE" << std::endl ;
	 else
	   std::cerr << "MERDE" << std::endl ;
#endif

	 CFRelease(flavorTypeArray) ;
    }

    HideDragHilite(inDrag) ; 

    return err ;
  }
#endif

  void
  glWindow_AGL::connectToWindowServer(void) {
    if (wsConnected) return ;

    wsConnected = true ;

    ProcessSerialNumber psn ;
    if (GetCurrentProcess(&psn) == noErr) {
	 CPSEnableForegroundOperation(&psn) ; // , 0x03, 0x3C, 0x2C, 0x1103) ;
	 SetFrontProcess(&psn) ;
	 std::string path = getNucleoResourcesDirectory()+"/nucleo.pdf" ;
	 CFURLRef icon = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
												  (const UInt8*)path.c_str(), path.size(), FALSE);
	 CGPDFDocumentRef document = CGPDFDocumentCreateWithURL(icon) ;
	 if (document) {
	   CGContextRef qctx = BeginCGContextForApplicationDockTile() ;
	   CGRect drawingarea = CGRectMake(0, 0, 128, 128) ;
	   CGContextClearRect(qctx, drawingarea) ;
	   CGContextDrawPDFPage(qctx, CGPDFDocumentGetPage(document, 1)) ;
	   CGContextFlush(qctx) ;
	   EndCGContextForApplicationDockTile(qctx) ;
	 }
	 CFRelease(icon) ;
	 // CFRelease(document) ; Causes a "Bus Error". Why?
    }
  }

  glWindow_AGL::glWindow_AGL(long options, long eventmask) {
#if defined(MAC_OS_X_VERSION_10_5) && !defined(LEOPARD_SUCKS) && DEBUG_LEVEL>0
    std::cerr << "glWindow_AGL: OS X 10.5 API enabled" << std::endl ;
#endif
    connectToWindowServer() ;

    debugEvents = false ;
    _aspect = 0.0 ;
    _minWidth = _minHeight = _maxWidth = _maxHeight = 0 ;
    _modifiers = 0 ;
    _mapped = false ;
    _emulateThreeButtonMouse = false ;
    _options = options ;

    // --------------------

    SetRect(&_rect, 50, 50, 320, 240) ; // SIF, might want to FIXME...
    WindowClass windowClass = kDocumentWindowClass ;
    WindowAttributes attrs = kWindowAsyncDragAttribute | kWindowCloseBoxAttribute | kWindowCollapseBoxAttribute ;
    // kWindowLiveResizeAttribute and kWindowIgnoreClicksAttribute
    // might also be useful
    if (_options&FLOATING) {
	 // std::cerr << "FLOATING" << std::endl ;
	 windowClass = kFloatingWindowClass ;
	 attrs = attrs | kWindowSideTitlebarAttribute ;
    } else if (_options&NONRESIZABLE) {
	 // std::cerr << "NONRESIZABLE" << std::endl ;
    } else {
	 // std::cerr << "NORMAL" << std::endl ;
	 attrs = attrs | kWindowFullZoomAttribute | kWindowResizableAttribute ;
    }
    CreateNewWindow(windowClass, attrs, &_rect, &_window) ;			

    // --------------------

    AGLPixelFormat apf = getPixelFormat(false) ;
    _context = _wContext = aglCreateContext(apf, NULL) ;
    _fsContext = 0 ;
    aglDestroyPixelFormat(apf) ;
#if defined(MAC_OS_X_VERSION_10_5) && !defined(LEOPARD_SUCKS)
    aglSetWindowRef(_context, _window) ;
#else
    aglSetDrawable(_context, GetWindowPort(_window)) ;
#endif
    aglSetCurrentContext(_context) ;

    // --------------------

    unsigned int lastEventType = 0;

    // so that we always know the geometry of the window
    _macEvents[lastEventType].eventClass = kEventClassWindow ;
    _macEvents[lastEventType++].eventKind = kEventWindowBoundsChanging ;
    _macEvents[lastEventType].eventClass = kEventClassWindow ;
    _macEvents[lastEventType++].eventKind = kEventWindowBoundsChanged ;

    _macEvents[lastEventType].eventClass = kEventClassWindow ;
    _macEvents[lastEventType++].eventKind = kEventWindowInit ;

    if (eventmask & glWindow::event::pointerMotion){
	 _macEvents[lastEventType].eventClass = kEventClassMouse ;
	 _macEvents[lastEventType++].eventKind = kEventMouseMoved ;
	 _macEvents[lastEventType].eventClass = kEventClassMouse ;
	 _macEvents[lastEventType++].eventKind = kEventMouseDragged ;
    }
    if (eventmask & glWindow::event::wheelMotion){
	 _macEvents[lastEventType].eventClass = kEventClassMouse ;
	 _macEvents[lastEventType++].eventKind = kEventMouseWheelMoved ;
    }
    if (eventmask & glWindow::event::buttonPress){
	 _macEvents[lastEventType].eventClass = kEventClassMouse ;
	 _macEvents[lastEventType++].eventKind = kEventMouseDown ;
    }
    if (eventmask & glWindow::event::buttonRelease){
	 _macEvents[lastEventType].eventClass = kEventClassMouse ;
	 _macEvents[lastEventType++].eventKind = kEventMouseUp ;
    }

    if (eventmask & glWindow::event::keyPress){
	 _macEvents[lastEventType].eventClass = kEventClassKeyboard ;
	 _macEvents[lastEventType++].eventKind = kEventRawKeyDown ;
    }
    if (eventmask & glWindow::event::keyRelease){
	 _macEvents[lastEventType].eventClass = kEventClassKeyboard ;
	 _macEvents[lastEventType++].eventKind = kEventRawKeyUp ;
    }
    if (eventmask & (glWindow::event::keyPress|glWindow::event::keyRelease)){
	 _macEvents[lastEventType].eventClass = kEventClassKeyboard ;
	 _macEvents[lastEventType++].eventKind = kEventRawKeyModifiersChanged ;
	 _macEvents[lastEventType].eventClass = kEventClassKeyboard ;
	 _macEvents[lastEventType++].eventKind = kEventRawKeyRepeat ;
    }

    if (eventmask & glWindow::event::expose){
#if 0
	 _macEvents[lastEventType].eventClass = kEventClassWindow ;
	 _macEvents[lastEventType++].eventKind = kEventWindowDrawContent ;
#else
	 // std::cerr << "glWindow_AGL: 'expose' events are not supported..." << std::endl ;
#endif
    }

    if (eventmask & glWindow::event::destroy) {
	 _macEvents[lastEventType].eventClass = kEventClassWindow ;
	 _macEvents[lastEventType++].eventKind = kEventWindowClosed ;
    }

    if (eventmask & glWindow::event::enter)
	 std::cerr << "glWindow_AGL: 'enter' events are not supported..." << std::endl ;
    if (eventmask & glWindow::event::leave)
	 std::cerr << "glWindow_AGL: 'leave' events are not supported..." << std::endl ;

    _nbMacEvents = lastEventType;

    EventTargetRef winTarget = GetWindowEventTarget(_window) ;
    InstallStandardEventHandler(winTarget) ;
    InstallEventHandler(winTarget,
				    NewEventHandlerUPP(glWindow_AGL::_windowEventHandler),
				    _nbMacEvents, _macEvents,
				    (void *)this,
				    &_evHandler) ;
    // InstallReceiveHandler(__dragReceive, _window, this) ;

    // So that objects that subscribe to us can get a "configure"
    // event (see the "react" method below)
    selfNotify() ;
  }
     
  glWindow_AGL::~glWindow_AGL(void) {
    RemoveEventHandler(_evHandler) ;
    aglSetCurrentContext(NULL) ;
    if (_fsContext) {
#if defined(MAC_OS_X_VERSION_10_5) && !defined(LEOPARD_SUCKS)
	 aglSetWindowRef(_fsContext, NULL) ;
#else
	 aglSetDrawable(_fsContext, NULL) ;
#endif
	 aglDestroyContext(_fsContext) ;
    }
#if defined(MAC_OS_X_VERSION_10_5) && !defined(LEOPARD_SUCKS)
    aglSetWindowRef(_wContext, NULL) ;
#else
    aglSetDrawable(_wContext, NULL) ;
#endif
    aglDestroyContext(_wContext) ;
#if defined(MAC_OS_X_VERSION_10_5) && !defined(LEOPARD_SUCKS)
    CFRelease(_window) ;
#else
    ReleaseWindow(_window) ;
#endif
  }

  // -----------------------------------------------------------------------

  void
  glWindow_AGL::setCursorVisible(bool activate) {
#if 1
    EnterMovies() ;
    if (activate) ShowCursor() ; else HideCursor() ;
#else
    CGRect rect ;
    rect.origin.x = _rect.left ;
    rect.origin.y = _rect.top ;
    rect.size.width = _rect.right - _rect.left ;
    rect.size.height = _rect.bottom - _rect.top ;
    CGDirectDisplayID dspys[10] ;
    CGDisplayCount count = 0 ;

    CGDisplayErr err = CGGetDisplaysWithRect(rect, 10, dspys, &count) ;
    if (err==CGDisplayNoErr) {
	 for ()
	 if (activate) CGDisplayShowCursor(dspy) ;
	 else CGDisplayHideCursor(dspy) ;
    } else {
	 std::cerr << "glWindow_AGL::setCursorVisible: CGGetDisplaysWithRect failed" << std::endl ;
    }
#endif
  }

  void
  glWindow_AGL::warpCursor(int x, int y) {
    CGPoint newCursorPosition ;
    newCursorPosition.x = _rect.left + x ;
    newCursorPosition.y = _rect.top + y ;
    CGWarpMouseCursorPosition(newCursorPosition) ;
    // Use CGDisplayMoveCursorToPoint(CGDirectDisplayID display, CGPoint point) instead ?
  }

  void
  glWindow_AGL::syncToVbl(int nb_vbls) {
    GLint n = nb_vbls ;
    aglSetInteger(_context, AGL_SWAP_INTERVAL, &n) ;
  }

  void
  glWindow_AGL::raise(void) {
    SelectWindow(_window) ;
  }

  void
  glWindow_AGL::lower(void) {
    SendBehind(_window, NULL) ;
  }

  float
  glWindow_AGL::getAlpha(void) {
    float alpha=1.0 ;
    GetWindowAlpha(_window, &alpha) ;
    return alpha ;
  }

  void
  glWindow_AGL::setAlpha(float alpha) {
    SetWindowAlpha(_window, alpha) ;
  }

  void
  glWindow_AGL::emulateThreeButtonMouse(bool activate) {
    _emulateThreeButtonMouse = activate ;
  }

  // -----------------------------------------------------------------------

  unsigned int
  glWindow_AGL::getScreenWidth(void) {
#if defined(MAC_OS_X_VERSION_10_5) && !defined(LEOPARD_SUCKS)
    CGDirectDisplayID displayID = CGMainDisplayID() ;
    return CGDisplayPixelsWide(displayID) ;
#else
    return (**GetMainDevice()).gdRect.right ;
#endif
  }
  
  unsigned int
  glWindow_AGL::getScreenHeight(void) {
#if defined(MAC_OS_X_VERSION_10_5) && !defined(LEOPARD_SUCKS)
    CGDirectDisplayID displayID = CGMainDisplayID() ;
    return CGDisplayPixelsHigh(displayID) ;
#else
    return (**GetMainDevice()).gdRect.bottom ;
#endif
  }

  void
  glWindow_AGL::map(void) {
    ShowWindow(_window) ;
    _mapped = true ;
  }
    
  void
  glWindow_AGL::unmap(void) {
    HideWindow(_window) ;
    _mapped = false ;
  }

  void
  glWindow_AGL::makeCurrent(void) {
    aglSetCurrentContext(_context) ;      
  }

  void
  glWindow_AGL::swapBuffers(void) {
    aglSwapBuffers(_context) ;
  }

  void
  glWindow_AGL::setTitle(const char *title) {
#if 0
    Str255 sTitle;
    CopyCStringToPascal(title, sTitle);
    SetWTitle(_window, sTitle);
#else
    CFStringRef newTitle = CFStringCreateWithCString (kCFAllocatorDefault, title,
										    kCFStringEncodingISOLatin1);
    SetWindowTitleWithCFString (_window, newTitle);
    CFRelease (newTitle); 
#endif
  }

  void
  glWindow_AGL::getGeometry(unsigned int *width, unsigned int *height, int *x, int *y) {
    if (x) *x = _rect.left ;
    if (y) *y = _rect.top ;
    if (width) *width = _rect.right - _rect.left ;
    if (height) *height = _rect.bottom - _rect.top ;
  }
    
  void 
  glWindow_AGL::setGeometry(unsigned int width, unsigned int height, int x, int y) {
    if (!_mapped) map() ;
    SizeWindow(_window, width, height, false);
    MoveWindow(_window, x, y, false);
  }

  void 
  glWindow_AGL::setGeometry(unsigned int width, unsigned int height) {
    if (!_mapped) map() ;
    SizeWindow(_window, width, height, false);
  }

  void
  glWindow_AGL::setFullScreen(bool activate) {
    // std::cerr << "setFullScreen(" << activate << ")" << std::endl ;
    if (!activate && !_fsContext) return ;
    if (activate && _fsContext) return ;

    glWindow::event *e = new glWindow::event ;
    e->type = glWindow::event::configure ;

    if (activate) {
	 SetUserFocusWindow(_window) ;
	 CGCaptureAllDisplays() ;

	 AGLPixelFormat apf = getPixelFormat(true) ;
	 _fsContext = aglCreateContext(apf, _wContext) ;
	 aglDestroyPixelFormat(apf) ;
	 if (!_fsContext) {
	   std::cerr << "glWindow_AGL::setFullScreen: aglCreateContext failed ("
			   << aglErrorString(aglGetError())
			   << ")" << std::endl ;
	   CGReleaseAllDisplays() ;
	   return ;
	 }

	 if (!aglCopyContext(_wContext, _fsContext, GL_ALL_ATTRIB_BITS))
	   std::cerr << "glWindow_AGL::setFullScreen: aglCopyContext failed" << std::endl ;
	 GLint swap_interval = 0 ;
	 aglGetInteger(_wContext, AGL_SWAP_INTERVAL, &swap_interval) ;
	 aglSetInteger(_fsContext, AGL_SWAP_INTERVAL, &swap_interval) ;
	 aglSetCurrentContext(_fsContext) ;
	 aglUpdateContext(_fsContext) ;
	 aglSetFullScreen(_fsContext, 0, 0, 0, 0) ;
	 _context = _fsContext ;

	 RemoveEventHandler(_evHandler) ;
	 InstallApplicationEventHandler(NewEventHandlerUPP(glWindow_AGL::_windowEventHandler),
							  _nbMacEvents, _macEvents,
							  (void *)this,
							  &_evHandler) ;

	 GLint params[3] ;
	 aglGetInteger(_fsContext, AGL_FULLSCREEN, params) ;
	 e->width = params[0] ; e->height = params[1] ;
	 e->x = e->y = 0 ;
	 _rect.left = 0 ; _rect.right = params[0] ;
	 _rect.top = 0 ; _rect.bottom = params[1] ;
	 HideMenuBar() ;
    } else {
	 aglCopyContext(_fsContext, _wContext, GL_ALL_ATTRIB_BITS) ;
#if defined(MAC_OS_X_VERSION_10_5) && !defined(LEOPARD_SUCKS)
	 // aglSetWindowRef(_fsContext, NULL) ; causes a segfault...
#else
	 aglSetDrawable(_fsContext, NULL) ;
#endif
	 aglDestroyContext(_fsContext) ;
	 aglSetCurrentContext(_wContext) ;
	 _context = _wContext ;
	 _fsContext = 0 ;

	 CGReleaseAllDisplays() ;
	 ShowWindow(_window) ;
	 _mapped = true ;

	 RemoveEventHandler(_evHandler) ;
	 InstallEventHandler(GetWindowEventTarget(_window),
					 NewEventHandlerUPP(glWindow_AGL::_windowEventHandler),
					 _nbMacEvents, _macEvents,
					 (void *)this,
					 &_evHandler) ;

	 GetWindowBounds(_window, kWindowContentRgn, &_rect) ;
	 e->width = _rect.right - _rect.left ; e->height = _rect.bottom - _rect.top ;
	 e->x = _rect.left ; e->y = _rect.top ;
	 ShowMenuBar() ;
    }

    _equeue.push(e) ;
    notifyObservers() ;
  }

  void
  glWindow_AGL::setAspectRatio(int width, int height) {
    _aspect = (double)width/(double)height ;
  }

  void
  glWindow_AGL::setMinMaxSize(int minwidth, int minheight, int maxwidth, int maxheight) {
    _minWidth = minwidth ;
    _minHeight = minheight ;
    _maxWidth = maxwidth ;
    _maxHeight = maxheight ;
  }

  // -----------------------------------------------------------------------

  glWindow::event *
  glWindow_AGL::getNextEvent(void) {
    if (_equeue.empty()) return 0 ;

    event *e = _equeue.front() ;
    _equeue.pop() ;
    if (debugEvents) {
	 std::cerr << "glWindow_AGL event: " ;
	 e->debug(std::cerr) ;
	 std::cerr << std::endl ;
    }

    return e ;
  }

  bool 
  glWindow_AGL::getNextEvent(event *e) {
    event *ev = getNextEvent() ;
    if (!ev) return false ;
    *e = *ev ;
    return true ;
  }

  // -----------------------------------------------------------------------

  void 
  glWindow_AGL::react(Observable *obs) {
    if (obs==this) {
	 glWindow::event *e = new glWindow::event ;
	 e->type = glWindow::event::configure ;
	 e->width = _rect.right - _rect.left ; e->height = _rect.bottom - _rect.top ;
	 e->x = _rect.left ; e->y = _rect.top ;
	 _equeue.push(e) ;
	 notifyObservers() ;
    }
  }

}
