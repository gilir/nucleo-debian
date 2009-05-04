#ifndef n_CoreGraphics_H
#define n_CoreGraphics_H

/*
 *
 * nucleo/image/encoding/CoreGraphics.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

namespace nucleo {

  // ARGB is directly supported. Anything else is converted to ARGB.
  bool cg_encode(Image *src, Image *dst, Image::Encoding e, unsigned int quality) ;

  // Update the width, height and timestamp fields
  void cg_decode_meta(Image *isrc) ;

  // ARGB is directly supported. Anything else is converted to ARGB.
  bool cg_decode(Image *isrc, Image *idst, Image::Encoding e, unsigned int quality) ;

}

#endif
