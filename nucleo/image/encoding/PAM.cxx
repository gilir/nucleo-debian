/*
 *
 * nucleo/image/encoding/PAM.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/image/Image.H>
#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/image/encoding/PAM.H>

#include <sstream>
#include <cstring>
#include <cstdlib>

namespace nucleo {

  // ------------------------------------------------------------------

  bool
  pam_encode(Image *isrc, Image *idst) {
    Image source ;
    source.linkDataFrom(*isrc) ;

    unsigned int width = source.getWidth() ;
    unsigned int height = source.getHeight() ;
    Image::Encoding encoding = source.getEncoding() ;

    int depth = 3 ;
    std::string tupltype = "RGB" ;
    if (encoding==Image::L) {
	 depth = 1 ;
	 tupltype = "GRAYSCALE" ;
    } else if (encoding!=Image::RGB) {
	 convertImage(&source, Image::RGB) ;
    }

    std::stringstream header ;
    header << "P7" << std::endl
		 << "WIDTH " << width << std::endl
		 << "HEIGHT " << height << std::endl 
		 << "MAXVAL 255" << std::endl
		 << "DEPTH " << depth << std::endl
		 << "TUPLTYPE " << tupltype << std::endl
		 << "ENDHDR" << std::endl ;

    std::string sHeader = header.str() ;
    unsigned int headerSize = sHeader.size() ;
    unsigned int dataSize = source.getSize() ;
    unsigned int size = headerSize+dataSize ;
    unsigned char *data = Image::AllocMem(size) ;
    memmove(data, sHeader.c_str(), headerSize) ;
    memmove(data+headerSize, source.getData(), dataSize) ;

    idst->setEncoding(Image::PAM) ;
    idst->setDims(width, height) ;
    idst->setData(data, size, Image::FREEMEM) ;
    idst->setTimeStamp(isrc->getTimeStamp()) ;

    return true ;
  }

  // ------------------------------------------------------------------

  static bool
  pam_parse_header(char *data, unsigned int datasize,
			    unsigned int *width, unsigned int *height,
			    std::string *tupltype, unsigned int *depth, unsigned int *maxval,
			    unsigned int *offset) {
    unsigned int iFrom=0, iTo=0 ;

    if (strncmp(data,"P7\n",3)) return false ;

    while (iFrom<datasize) {
	 for (iTo=iFrom; iTo<datasize && data[iTo]!='\n'; ++iTo) ;
	 if (iTo==datasize) break ;
	 if (data[iFrom]!='#') {
	   if (!strncmp(data+iFrom,"WIDTH ",6)) *width = atoi(data+iFrom+6) ;
	   if (!strncmp(data+iFrom,"HEIGHT ",7)) *height = atoi(data+iFrom+7) ;
	   if (!strncmp(data+iFrom,"DEPTH ",6)) *depth = atoi(data+iFrom+6) ;
	   if (!strncmp(data+iFrom,"MAXVAL ",7)) *maxval = atoi(data+iFrom+7) ;
	   if (!strncmp(data+iFrom,"TUPLTYPE ",9)) tupltype->assign(data,iFrom+9,iTo-(iFrom+9)) ;
	   if (!strncmp(data+iFrom,"ENDHDR",6)) {
		*offset = iTo+1 ;
		return true ;
	   }
	 }
	 iFrom = iTo+1 ;
    }

    return false ;
  }

  void
  pam_calcdims(Image *img) {
    unsigned int width, height, depth, maxval, offset ;
    std::string tupltype ;
    if (pam_parse_header((char *)img->getData(), img->getSize(),
					&width, &height,
					&tupltype, &depth, &maxval,
					&offset))
	 img->setDims(width, height) ;
  }

  // ------------------------------------------------------------------

  bool
  pam_decode(Image *isrc, Image *idst, Image::Encoding target_encoding, unsigned int quality) {
    unsigned int width, height, depth, maxval, offset ;
    std::string tupltype ;
    if (!pam_parse_header((char *)isrc->getData(), isrc->getSize(),
					 &width, &height,
					 &tupltype, &depth, &maxval,
					 &offset)) {
	 // std::cerr << "pam_parse_header failed" << std::endl ;
	 return false ;
    }

#if 0
    std::cerr << "Image is " << tupltype << " (" << depth << " bpp), "
		    << width << "x" << height
		    << std::endl ;
#endif

    if (depth==1 && tupltype=="GRAYSCALE")
	 idst->setEncoding(Image::L) ;
    else if (depth==3 && tupltype=="RGB")
	 idst->setEncoding(Image::RGB) ;
    else
	 return false ;

    idst->setDims(width, height) ;
    idst->setData(isrc->getData()+offset, width*height*depth, Image::NONE) ;
    idst->setTimeStamp(isrc->getTimeStamp()) ;

    if (!convertImage(idst, target_encoding, quality)) return false ;
    idst->acquireData() ;
    return true ;
  }

  // ------------------------------------------------------------------

}
