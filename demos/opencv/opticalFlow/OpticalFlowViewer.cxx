#include "OpticalFlowViewer.H"

#include <nucleo/core/ReactiveEngine.H>

#include <sstream>
#include <iomanip>

OpticalFlowViewer::OpticalFlowViewer(OpticalFlow *f, int tmi, int tma) {
  tmin = tmi ; tmax = tma ;
  flow = f ;
  subscribeTo(flow) ;

  long options = glWindow::DOUBLE_BUFFER ;
  long eventmask = glWindow::event::destroy|glWindow::event::configure|glWindow::event::keyRelease ;
  window = glWindow::create(options, eventmask) ;
  subscribeTo(window) ;   

  texture = new glTexture ;
  showImage = true ;
  message = new glString ;
  verb = V_FPS ;

  window->map() ;  
  std::cerr << "OpticalFlowViewer: tmin=" << tmin << " tmax=" << tmax << std::endl ;
}

void
OpticalFlowViewer::react(Observable *obs) {
  bool redisplay = false ;
  unsigned flow_width=flow->getWidth(), flow_height=flow->getHeight() ;

  if (obs==window) {
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
	   redisplay = (flow->getFrameNumber()>1) ;
	   break ;
	 case glWindow::event::keyRelease:
	   switch (e.keysym) {
	   case XK_F1: showImage = !showImage ; break ;
	   case XK_F2: verb = (verbosity)((verb+1)%V_MAX) ; break ;
	   case XK_Escape: ReactiveEngine::stop() ; return ;
	   }
	   break ;
	 case glWindow::event::destroy:
	   ReactiveEngine::stop() ;
	   return ;
	 default:
	   break ;
	 }
    }
  }
 
  if (obs==flow) {
    // std::cerr << "flow: " << flow_width << "x" << flow_height << " " << flow->getFrameNumber() << std::endl ;
    if (flow_width!=window_width || flow_height!=window_height) {
	 window->setGeometry(flow_width,flow_height) ;
	 window->setAspectRatio(flow_width,flow_height) ;
    } else
	 redisplay = true ;
  }

  if (redisplay) {
    glLoadIdentity() ;

    glClear(GL_COLOR_BUFFER_BIT) ;
    if (showImage) {
	 flow->updateTexture(texture) ;
	 glColor4f(0.5,0.5,0.5,1.0) ;
	 texture->display(0,0,window_width,window_height) ;
    }

    float x=0.0, y=0.0 ;

    glPushMatrix() ;
    glColor4f(1.0,1.0,1.0,1.0) ;
    glTranslatef(0,window_height,0) ;
    glScalef((float)window_width/flow_width,-(float)window_height/flow_height,1) ;
    unsigned int nbPairs = flow->getNbPairs() ;
    unsigned int nbPairsValid=0, nbPairsOk=0 ;
    for (unsigned int i=0; i<nbPairs; ++i) {
	 OpticalFlow::FeaturePair pair = flow->getPair(i) ;
	 if (!pair.isValid
		|| pair.x2<0 || pair.x2>=flow_width
		|| pair.y2<0 || pair.y2>=flow_height) continue ;
	 nbPairsValid++ ;
	 float dx = pair.x2 - pair.x1, dy = pair.y2 - pair.y1 ;
	 float d = dx*dx + dy*dy ;
	 if (d<tmin || d>tmax) continue ;
	 nbPairsOk++ ;

	 x += dx ; y += dy ;
	 glRectf(pair.x2-1.,pair.y2-1.,pair.x2+1.,pair.y2+1.) ;
	 glBegin(GL_LINES) ;
	 glVertex2f(pair.x1,pair.y1) ;
	 glVertex2f(pair.x2,pair.y2) ;
	 glEnd() ;
    }
    
    if (nbPairsOk) {
	 x = flow_width/2.0 + x/nbPairsOk ;
	 y = flow_height/2.0 + y/nbPairsOk ;
	 glColor4f(1.0,0.0,0.0,1.0) ;    
	 glRectf(x-1.,y-1.,x+1.,y+1.) ;
	 glBegin(GL_LINES) ;
	 glVertex2f(flow_width/2.0,flow_height/2.0) ;
	 glVertex2f(x,y) ;
	 glVertex2f(x,y) ;
	 glEnd() ;
    }

    glPopMatrix() ;

    if (verb!=V_NOTHING) {
	 *message << glString::CLEAR << (int)flow->getFrameRate() ;
	 if (verb==V_FPS_AND_STATS) {
	   std::stringstream msg ;
	   msg << " - " 
		  << std::setfill('0') << std::setw(3) << nbPairsValid
		  << "/" 
		  << std::setfill('0') << std::setw(3) << nbPairsOk  ;
	   *message << msg.str() ;
	 }
	 glTranslatef(10,10,0) ;
	 glColor4f(1.0,1.0,1.0,1.0) ;
	 message->renderAsTexture() ;
    }

    window->swapBuffers() ;
  }
}

OpticalFlowViewer::~OpticalFlowViewer(void) {
  unsubscribeFrom(flow) ;
  unsubscribeFrom(window) ;
  delete texture ;
  delete message ;
  delete window ;
}
