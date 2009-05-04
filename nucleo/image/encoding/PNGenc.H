#ifndef n_PNGenc_H
#define n_PNGenc_H

/*
 *
 * nucleo/image/encoding/PNGenc.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

namespace nucleo {

  // RGB, L, RGBA and ARGB images are directly supported. ABGR data is
  // converted to RGBA. Anything else is converted to RGB.
  bool png_encode(Image *isrc, Image *idst, unsigned int quality) ;

  void png_calcdims(Image *isrc) ;

  // RGB, L and RGBA images are directly supported (choice depends on
  // what is actually stored in PNG data). Anything else is converted
  // from one of these encodings.
  bool png_decode(Image *isrc, Image *idst, Image::Encoding e, unsigned int quality) ;

}

#endif
