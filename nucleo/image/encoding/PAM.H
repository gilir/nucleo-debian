#ifndef n_PAM_H
#define n_PAM_H

/*
 *
 * nucleo/image/encoding/PAM.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

namespace nucleo {

  // RGB and L images are directly supported. Anything else is first
  // converted to RGB.
  bool pam_encode(Image *isrc, Image *idst) ;

  void pam_calcdims(Image *isrc) ;

  // RGB and L images are directly supported. Anything else is first
  // decoded to RGB.
  bool pam_decode(Image *isrc, Image *idst, Image::Encoding e, unsigned int quality) ;

}

#endif
