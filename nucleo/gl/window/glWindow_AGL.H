#ifndef n_glWindow_AGL_H
#define n_glWindow_AGL_H

/*
 *
 * nucleo/gl/window/glWindow_AGL.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/gl/window/glWindow.H>

#include <AGL/agl.h>

#include <iostream>
#include <queue>

namespace nucleo {

  class glWindow_AGL : public glWindow {

  protected:

    long _options ;
    EventTypeSpec _macEvents[24] ;
    unsigned int _nbMacEvents ;

    EventHandlerRef _evHandler ;

    WindowPtr _window ;
    AGLContext _wContext, _fsContext, _context ;

    bool _mapped ; // whether the window is currently mapped on-screen
 
    double _aspect ;
    UInt32 _minWidth, _minHeight, _maxWidth, _maxHeight ;
    Rect _rect ; // used to translate mouse coordinate in window space
    UInt32 _modifiers ;
    bool _emulateThreeButtonMouse ;

    std::queue<event *> _equeue ;
    static OSStatus _windowEventHandler(EventHandlerCallRef, EventRef, void*) ;

    static bool wsConnected ;
    static void connectToWindowServer(void) ;

    AGLPixelFormat getPixelFormat(bool fullscreen) ;

    void react(Observable *) ;

  public:

    unsigned int getScreenWidth(void) ;
    unsigned int getScreenHeight(void) ;

    glWindow_AGL(long options, long eventmask) ;
    ~glWindow_AGL(void) ;

    void map(void) ;
    void unmap(void) ;

    void setTitle(const char *title) ;
    void getGeometry(unsigned int *width, unsigned int *height, int *x=0, int *y=0) ;
    void setGeometry(unsigned int width, unsigned int height, int x, int y) ;
    void setGeometry(unsigned int width, unsigned int height);
    void setFullScreen(bool activate) ;
    void setAspectRatio(int width, int height) ;
    void setMinMaxSize(int minwidth, int minheight, int maxwidth, int maxheight) ;

    void makeCurrent(void) ;
    void swapBuffers(void) ;

    void setCursorVisible(bool activate) ;
    void warpCursor(int x, int y) ;
    void syncToVbl(int nb_vbls=1) ;
    void raise(void) ;
    void lower(void) ;
    float getAlpha(void) ;
    void setAlpha(float alpha) ;
    void emulateThreeButtonMouse(bool activate) ;

    bool getNextEvent(event *) ;
    event *getNextEvent(void) ;
  } ;

}

#endif
