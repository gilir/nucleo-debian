/*
 *
 * demos/misc/multitexture.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/nucleo.H>
#include <nucleo/utils/AppUtils.H>
#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/image/source/ImageSource.H>
#include <nucleo/image/processing/basic/Resize.H>
#include <nucleo/image/processing/basic/Paint.H>
#include <nucleo/gl/window/glWindow.H>
#include <nucleo/gl/glUtils.H>

#include <iostream>
#include <stdexcept>

#include <cstdlib> // exit
#include <unistd.h>

using namespace nucleo ;

// ---------------------------------------------------------------------------

int
main(int argc, char **argv) {

  try {
    char *MASK = 0 ;
    char *VIDEO = 0 ;
    char *GEOMETRY = "320x240" ;
    bool FAST = false ;

    if (parseCommandLine(argc, argv, "m:i:g:f", "sssb",
					&MASK, &VIDEO, &GEOMETRY, &FAST)<0) {
	 std::cerr << std::endl << argv[0] ;
	 std::cerr << " [-m mask] [-i source] [-g geometry] [-(use)s(ub-textures)] [-f(ast)]" << std::endl ;
	 std::cerr << std::endl << std::endl ;
	 exit(1) ;
    }

    if (!MASK) MASK = "file:/Users/roussel/casa/images/masks/chessboard-2.jpg" ;

    if (!VIDEO) VIDEO = getenv("NUCLEO_SRC1") ;

    // ------------------------------------------------------


    long options = glWindow::DOUBLE_BUFFER ;
    long eventmask = glWindow::event::configure
	 | glWindow::event::expose
	 | glWindow::event::keyRelease ;
    glWindow *win = glWindow::create(options, eventmask) ;

    win->setTitle("multitexture") ;
    win->setGeometry(GEOMETRY) ;
    win->makeCurrent() ;

#if 0
    // GL_ARB_multitexture was promoted to a core feature in OpenGL 1.3. 
    const char *exten = (const char *) glGetString(GL_EXTENSIONS);
    if (!strstr(exten, "GL_ARB_multitexture")) {
	 std::cerr << "Sorry, GL_ARB_multitexture is not supported..." << std::endl ;
	 exit(1);
    }
#endif

    glClearColor(1.0,1.0,1.0,1.0) ;

    GLuint texObj[2];
    glGenTextures(2, texObj);

    // ------------------------------------------------------

    Image iMask ;
    if (!ImageSource::getImage(MASK, &iMask, Image::L)) {
	 std::cerr << "Unable to get mask from " << MASK << std::endl ;
	 exit(1) ;
    }

    unsigned int width=iMask.getWidth(), height=iMask.getHeight() ;
    std::cerr << "mask: " << width << "x" << height << std::endl ;
    unsigned int newWidth, newHeight ;
    for (newWidth=2; newWidth<width; newWidth*=2) ;
    for (newHeight=2; newHeight<height; newHeight*=2) ;

    resizeImage(&iMask, newWidth, newHeight) ;

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D) ;
    glBindTexture(GL_TEXTURE_2D, texObj[0]);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D,
			  0,
			  GL_ALPHA,
			  iMask.getWidth(), iMask.getHeight(),
			  0,
			  GL_ALPHA, GL_UNSIGNED_BYTE, iMask.getData()) ;

    if (FAST) {
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) ;
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST) ;
    } else {
	 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

    // ---------

    ImageSource *sVideo = ImageSource::create(VIDEO, Image::RGB) ;
    if (!sVideo) {
	 std::cerr << "Unable to create source from " << VIDEO << std::endl ;
	 exit(1) ;
    }

    Image iVideo, tVideo(512,512,Image::RGB) ;

    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D) ;
    glBindTexture(GL_TEXTURE_2D, texObj[1]);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D,
			  0,
			  GL_RGB,
			  tVideo.getWidth(), tVideo.getHeight(),
			  0,
			  GL_RGB, GL_UNSIGNED_BYTE, tVideo.getData()) ;

    if (FAST) {
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) ;
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST) ;
    } else {
	 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

#if 1
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#else
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE) ;
#endif

    // ------------------------------------------------------

    double angle=0.0, aDelta=0.0 ;

    for (bool loop=true; loop; ) {

	 bool refresh = true ;

	 if (sVideo->getState()==ImageSource::STOPPED) sVideo->start() ;

	 ReactiveEngine::step(100) ;

	 glWindow::event e ;
	 while (win->getNextEvent(&e)) {
	   switch( e.type ) {
	   case glWindow::event::keyRelease:
		switch (e.keysym) {
		case XK_Escape: loop=false ; break ;
		case XK_Up: ++aDelta ; break ;
		case XK_Down: --aDelta ; break ;
		}
		break ;
	   case glWindow::event::configure:
		win->setAspectRatio(e.width,e.height) ;
		glViewport(0,0,e.width,e.height) ;
		refresh = true ;
		break ;
	   case glWindow::event::expose:
		refresh = true ;
		break ;
	   default:
		break ;
 	   }
	 }

	 if (sVideo->getNextImage(&iVideo)) {
	   glActiveTexture(GL_TEXTURE1);
	   glTexSubImage2D(GL_TEXTURE_2D,
				    0,
				    0, 0, 
				    iVideo.getWidth(), iVideo.getHeight(),
				    GL_RGB, GL_UNSIGNED_BYTE, iVideo.getData()) ;
	   refresh = true ;
	 }

	 if (refresh) {
	   glClear(GL_COLOR_BUFFER_BIT) ;

	   glMatrixMode(GL_MODELVIEW) ;
	   glPushMatrix() ;

	   glEnable(GL_TEXTURE_2D) ;
	   glEnable(GL_BLEND);
	   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	   glColor3f(1.0,1.0,1.0) ;
	   glScalef(2,2,2) ;
	   glRotated(angle, 0.0, 0.0, 1.0) ;
	   angle += aDelta ;

 	   const float tw2 = (float)iVideo.getWidth()/(float)tVideo.getWidth() ;
 	   const float th2 = (float)iVideo.getHeight()/(float)tVideo.getHeight() ;

	   glBegin(GL_QUADS);
   	   glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0);
	   glMultiTexCoord2f(GL_TEXTURE1, 0.0, th2);
	   glVertex2f(-0.5, -0.5);
	   glMultiTexCoord2f(GL_TEXTURE0, 1.0, 1.0);
	   glMultiTexCoord2f(GL_TEXTURE1, tw2, th2);
	   glVertex2f(0.5, -0.5);
	   glMultiTexCoord2f(GL_TEXTURE0, 1.0, 0.0);
	   glMultiTexCoord2f(GL_TEXTURE1, tw2, 0.0);
	   glVertex2f(0.5, 0.5);
	   glMultiTexCoord2f(GL_TEXTURE0, 0.0, 0.0);
	   glMultiTexCoord2f(GL_TEXTURE1, 0.0, 0.0);
	   glVertex2f(-0.5, 0.5);
	   glEnd();

	   glPopMatrix() ;

	   win->swapBuffers() ;
	 }

    }

    std::cerr << "video: " << sVideo->getMeanRate() << " img/sec" << std::endl ;

    sVideo->stop() ;
    delete sVideo ;

    return 0 ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (...) {
    std::cerr << "Unknown error" << std::endl ;
  }

  return -1 ;
}
