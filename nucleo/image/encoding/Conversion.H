#ifndef n_Conversion_H
#define n_Conversion_H

/*
 *
 * nucleo/image/encoding/Conversion.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/Image.H>

namespace nucleo {

  bool convertImage(Image *src, Image *dst,
				Image::Encoding e, unsigned int quality=100) ;

  bool convertImage(Image *img,
				Image::Encoding e, unsigned int quality=100) ;

}

#endif
