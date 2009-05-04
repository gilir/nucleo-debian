#ifndef n_PerlinNoise_H
#define n_PerlinNoise_H

/*
 *
 * nucleo/image/processing/noise/PerlinNoise.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

namespace nucleo {

  class ImprovedPerlinNoise {
    
  private:
    
    static bool initialized ;
    static int p[512] ;
    double biasx, biasy, biasz ;
    
  public:
    
    ImprovedPerlinNoise(bool randomize=true) ;
    
    double noise(double x) {
	 return noise(x,0.13,0.95) ;
    }
    double noise(double x, double y) {
	 return noise(x,y,0.37) ;
    }
    double noise(double x, double y, double z) ;

  } ;

}

#endif
