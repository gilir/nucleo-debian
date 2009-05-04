#ifndef n_YpCbCr420_H
#define n_YpCbCr420_H

/*
 *
 * nucleo/image/encoding/YpCbCr420.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

namespace nucleo {

  // -------------------------------------------------------
  // xRGB is RGB or ARGB...

  void xRGB2YpCbCr420(Image *img) ;

  // Image dimensions are cropped to a multiple of 16
  void YpCbCr4202xRGB(Image *img, Image::Encoding dst_encoding) ;

  // -------------------------------------------------------

}

#endif
