#ifndef n_glImageSink_H
#define n_glImageSink_H

/*
 *
 * nucleo/image/sinks/glwindowImageSink.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/URI.H>

#include <nucleo/image/sink/ImageSink.H>

#include <nucleo/gl/window/glWindow.H>
#include <nucleo/gl/texture/glTexture.H>
#include <nucleo/image/processing/gamma/GammaCorrection.H>

namespace nucleo {

  // ----------------------------------------------------------------------------

  class glwindowImageSink : public ImageSink {

  private:

    URI _uri ;

    long *_display ; // An X Window Display*...
    glWindow *_window ;
    unsigned int _win_width, _win_height ;

    bool _active ;

    bool _fitImage ;
    bool _saveNextFrame ;
    bool _displayFrameRate ;

    GammaByteCorrection _gcorrect ;
    glTexture _texture ;

    void refresh(void) ;

  protected:

    void react(Observable*) ;

  public:

    glwindowImageSink(const URI &u) ;
    ~glwindowImageSink() ;

    ImageSink::state getState(void) {
	 return (_active?STARTED:STOPPED) ;
    }

    bool start(void) ;
    bool handle(Image *img) ;
    bool stop(void) ;

  } ;

  // ----------------------------------------------------------------------------

}

#endif
