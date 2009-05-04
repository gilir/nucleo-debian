#ifndef n_JPEG_H
#define n_JPEG_H

/*
 *
 * nucleo/image/encoding/JPEG.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

namespace nucleo {

  // RGB and L images are directly supported. Anything else is
  // converted to RGB.
  bool jpeg_encode(Image *isrc, Image *idst, unsigned int quality) ;

  void jpeg_calcdims(Image *isrc) ;

  // idst pivot encoding: RGB (L is directly supported)
  bool jpeg_decode(Image *isrc, Image *idst, Image::Encoding e, unsigned int quality) ;

}

#endif
