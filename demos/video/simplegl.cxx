/*
 *
 * demos/video/simplegl.cxx --
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
#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/gl/window/glWindow.H>
#include <nucleo/gl/texture/glTexture.H>
#include <nucleo/gl/text/glFontManager.H>
#include <nucleo/gl/text/glString.H>
#include <nucleo/gl/shading/glShader.H>

#include <iostream>
#include <stdexcept>

using namespace nucleo ;

class SimpleApp : public ReactiveObject {

protected:

  ImageSource *source ;
  Image image ;
  glWindow *window ;
  unsigned int window_width, window_height ;
  glTexture *texture ;
  glString *message ;
  glShader *shader ;
  std::string shaderstatus ;
  bool firstImage ;

  void react(Observable *obs) {
    bool redisplay=false ;

    glWindow::event e ;
    while (window->getNextEvent(&e)) {
	 switch (e.type) {
	 case glWindow::event::configure:
	   glViewport(0,0,e.width,e.height) ;
	   glMatrixMode(GL_PROJECTION) ;
	   glLoadIdentity() ;
	   glOrtho(0,e.width,0,e.height,-1.0,1.0) ;
	   glMatrixMode(GL_MODELVIEW) ;
	   window_width = e.width ;
	   window_height = e.height ;
	   redisplay = true ;
	   break ;
	 case glWindow::event::keyRelease:
	   switch (e.keysym) {
	   case XK_Escape:
		ReactiveEngine::stop() ;
		break ;
	   case XK_space:
		if (shader->isActive()) {
		  shader->deactivateAllShaders() ;
		  shaderstatus = "" ;
		} else {
		  shader->activate() ;
		  shaderstatus = shader->isActive() ? " (shader on)" : " (shader problem)" ;
		}
		break ;
	   case XK_F6:
		window->setFullScreen(true) ;
		break ;
	   case XK_F7:
		window->setFullScreen(false) ;
		break ;
	   }
	   break ;
	 case glWindow::event::destroy:
	   ReactiveEngine::stop() ;
	   break ;
	 default:
	   break ;
	 }
    }

    if (source->getNextImage(&image)) {
	 if (firstImage) {
	   unsigned int w = image.getWidth() ;
	   unsigned int h = image.getHeight() ;
	   window->setGeometry(w,h) ;
	   window->setAspectRatio(w,h) ;
	   window->map() ;
	   firstImage = false ;
	 }
	 texture->update(&image) ;
	 redisplay = true ;
    }

    if (redisplay) {
	 glClear(GL_COLOR_BUFFER_BIT) ;
	 glLoadIdentity() ;
	 // display the image
	 texture->display(0,0,window_width,window_height) ;
	 // display the message
	 *message << glString::CLEAR << (int)source->getMeanRate() << " fps" << shaderstatus ;
	 glTranslatef(10,10,0) ;
	 message->renderAsTexture() ;
	 // done!
	 window->swapBuffers() ;
    }
  }

public:

  SimpleApp(const char *uri, std::list<std::string> &shaders) {
    long options = glWindow::DOUBLE_BUFFER ;
    long eventmask = glWindow::event::destroy|glWindow::event::configure|glWindow::event::keyRelease ;
    window = glWindow::create(options, eventmask) ;
    subscribeTo(window) ;   

    std::string title = uri ;
    if (title.size()>20) title.replace(0,title.size()-20,"...") ;
    window->setTitle(title) ;

    // these two need a valid OpenGL context, which is why they
    // couldn't be created before
    texture = new glTexture ;
    message = new glString ;

    *message << glFontManager::getFont("vera:sans-serif") ;

    shader = new glShader ;
    for (std::list<std::string>::const_iterator s=shaders.begin();
	    s!=shaders.end(); ++s) {
	 URI uri(*s) ;
	 std::string path = (uri.opaque!="" ? uri.opaque : uri.path) ;
	 shader->attachFromFile(path,uri.scheme,path) ;
    }
    shader->link() ;

    source = ImageSource::create(uri, Image::PREFERRED) ;
    subscribeTo(source) ;
    source->start() ;

    firstImage = true ;
  }

  ~SimpleApp(void) {
    unsubscribeFrom(source) ;
    delete source ;
    delete texture ;
    delete message ;
    unsubscribeFrom(window) ;
    delete window ;
  }

} ;

int
main(int argc, char **argv) {
  try {
    char *SOURCE = "videoin:" ;
    int firstArg = parseCommandLine(argc, argv, "i:", "s", &SOURCE) ;
    if (firstArg<0) {
	 std::cerr << std::endl << argv[0] << " [-i source]" ;
	 std::cerr << std::endl ;
	 exit(1) ;
    }

    std::list<std::string> shaders ;
    for (int i=0; i<argc-firstArg; ++i)
	 shaders.push_back(argv[firstArg+i]) ;

    // glTextureTile::debugLevel = 10 ;
    SimpleApp tester(SOURCE, shaders) ;

    ReactiveEngine::run() ;

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }
}
