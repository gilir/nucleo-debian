/*
 *
 * nucleo/image/source/noiseImageSource.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/source/noiseImageSource.H>
#include <nucleo/image/encoding/Conversion.H>

#include <cstdlib>

namespace nucleo {

  // -----------------------------------------------------

  noiseImageSource::noiseImageSource(const URI &uri, Image::Encoding encoding) {
    target_encoding = (encoding==Image::PREFERRED) ? Image::L : encoding ;

    std::string query = uri.query ;
    std::string arg ;

    width = URI::getQueryArg(query, "w", &arg) ? atoi(arg.c_str()) : 320 ;
    height = URI::getQueryArg(query, "h", &arg) ? atoi(arg.c_str()) : 320 ;
    harmonics = URI::getQueryArg(query, "H", &arg) ? atoi(arg.c_str()) : 1 ;
    scale = URI::getQueryArg(query, "s", &arg) ? atof(arg.c_str()) : 1.0 ;

    min_val = URI::getQueryArg(query, "m", &arg) ? (unsigned char)atoi(arg.c_str()) : 0 ;
    max_val = URI::getQueryArg(query, "M", &arg) ? (unsigned char)atoi(arg.c_str()) : 255 ;

    _deltat = URI::getQueryArg(query, "f", &arg) ? (unsigned long)(1000.0 / atof(arg.c_str())) : 0 ;

    offset_x = offset_y = 0.01 ;
    _tk = 0 ;
  }

  // -----------------------------------------------------

  bool
  noiseImageSource::start(void) {
    _tk = TimeKeeper::create(_deltat, true) ;
    subscribeTo(_tk) ;
    frameCount = 0 ; previousImageTime = TimeStamp::undef ; sampler.start() ;
    return true ;
  }

  void
  noiseImageSource::react(Observable *obs) {
    if (obs!=_tk) return ;

    lastImage.prepareFor(width, height, Image::L) ;
    unsigned char *ptr = lastImage.getData() ;  

    double v_max=0 ;
    unsigned int s=1 ;
    for (unsigned int i=0; i<harmonics; ++i) {
	 v_max += 1.0/s ;
	 s <<= 1 ;
    }
    //    std::cerr << "harmonics=" << harmonics << " v_max=" << v_max << std::endl ;

    double v_o = min_val+(max_val-min_val)/2.0 ;
    double v_s = (max_val-min_val)/(2.0*v_max) ;
    //    std::cerr << "v_o=" << v_o << " v_s=" << v_s << std::endl ;

    double sw = scale/width ;
    double sh = scale/height ;
    for (unsigned int row=0; row<height; ++row)
	 for (unsigned int col=0; col<width; ++col) {
	   float x = offset_x + (double)col*sw ;
	   float y = offset_y + (double)row*sh ;

	   unsigned int s = 1 ;
	   double n = 0.0 ;
	   for (unsigned int i=0; i<harmonics; ++i) {   
		n += noisemaker.noise(x*s, y*s)/s ;
		s <<= 1 ;
	   }
	   
	   *ptr++ = (unsigned char)(v_o + n*v_s) ;
	 }

    offset_x += dx.noise(offset_y)/10.0 + 0.004 ;
    offset_y += dy.noise(offset_y)/10.0 + 0.003 ;

    lastImage.setTimeStamp() ;
    frameCount++ ; sampler.tick() ;
    if (!_pendingNotifications) notifyObservers() ;
  }

  bool
  noiseImageSource::getNextImage(Image *img, TimeStamp::inttype reftime) {
    if (!frameCount || reftime>=lastImage.getTimeStamp()) return false ;
    previousImageTime = lastImage.getTimeStamp() ;
    bool ok = convertImage(&lastImage, target_encoding) ;
    if (ok) img->linkDataFrom(lastImage) ;
    return ok ;
  }

  bool
  noiseImageSource::stop(void) {
    unsubscribeFrom(_tk) ;
    delete _tk ;
    _tk = 0 ;
    sampler.stop() ;
    return true ;
  }

  // -----------------------------------------------------

}
