/*
 *
 * nucleo/image/encoding/PNGenc.cxx --
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
#include <nucleo/image/encoding/PNGenc.H>

#include <png.h>

namespace nucleo {

  struct png_in_memory {
    unsigned char *data ;
#if 1
    png_size_t p ;
#else
    unsigned int p ;
#endif

    png_in_memory(unsigned char *d) : data(d), p(0) {}
  } ;

  // ------------------------------------------------------------------

  static void
  png_memory_write_data(png_structp png_ptr,
				    png_bytep data, png_size_t length) {
    png_in_memory *pim = (png_in_memory*)png_ptr->io_ptr ;
    
    void *pdst = pim->data+pim->p ;
    // std::cerr << "PNG: writing " << length << " bytes from " << (void *)data << " to " << pdst << std::endl ;
    memmove(pdst, data, length) ;

    pim->p += length ;
  }

  static void
  png_memory_flush_data(png_structp png_ptr) {
    // std::cerr << "PNG: flush" << std::endl ;
  }

  bool
  png_encode(Image *isrc, Image *idst, unsigned int quality) {
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
										NULL, NULL, NULL) ;
    if (!png_ptr) return false ;

    png_infop info_ptr = png_create_info_struct(png_ptr) ;
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL) ;
        return false ;
    }

    int bit_depth=8, color_type ;

    Image source ;
    source.linkDataFrom(*isrc) ;

    switch (source.getEncoding()) {
    case Image::L:
	 color_type = PNG_COLOR_TYPE_GRAY ;
	 break ;
    case Image::RGB:
	 color_type = PNG_COLOR_TYPE_RGB ;
	 break ;
    case Image::ARGB:
	 color_type = PNG_COLOR_TYPE_RGB_ALPHA ;
	 png_set_swap_alpha(png_ptr) ;
	 break ;
    case Image::RGBA:
	 color_type = PNG_COLOR_TYPE_RGB_ALPHA ;
	 break ;
    default:
	 convertImage(&source, Image::RGB) ;
	 color_type = PNG_COLOR_TYPE_RGB ;
	 break ;
    }

    // source.debug(std::cerr) ; std::cerr << std::endl ;

    unsigned int iwidth = source.getWidth() ;
    unsigned int iheight = source.getHeight() ;

    idst->setEncoding(Image::PNG) ;
    idst->setDims(iwidth, iheight) ;
    unsigned int size = (unsigned int)(source.getSize()*1.3) ; // FIXME: better estimation?
    unsigned char *data = Image::AllocMem(size) ;
    // std::cerr << "PNG: dst is " << size << " bytes starting at " << (void*)data << std::endl ;

    struct png_in_memory pim(data) ;
    png_set_write_fn(png_ptr,
				 (void*)&pim,
				 png_memory_write_data, png_memory_flush_data) ;

    png_set_IHDR(png_ptr, info_ptr, iwidth, iheight,
			  bit_depth, color_type, PNG_INTERLACE_NONE,
			  PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT) ;

    png_write_info(png_ptr, info_ptr) ;
    
    png_byte *row_pointer = source.getData() ;
    unsigned int rowSize = iwidth*source.getBytesPerPixel() ;
    for (unsigned int i=0; i<iheight; ++i) {
	 png_write_row(png_ptr, row_pointer) ;
	 row_pointer += rowSize ;
    }

    png_write_end(png_ptr, info_ptr) ;

    png_destroy_write_struct(&png_ptr, &info_ptr) ;

    // std::cerr << "PNG: p = " << pim.p << std::endl ;;
    idst->setData(data, pim.p, Image::FREEMEM) ;
    idst->setTimeStamp(isrc->getTimeStamp()) ;

    return true ;
  }

  // ------------------------------------------------------------------

  static void
  png_memory_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_in_memory *pim = (png_in_memory*)png_ptr->io_ptr ;
    memmove(data, pim->data+pim->p, length) ;
    pim->p+=length ;
  }

  bool
  png_decode(Image *isrc, Image *idst, Image::Encoding target_encoding, unsigned int quality) {
    unsigned char *data = isrc->getData() ;

    if (png_sig_cmp(data, (png_size_t)0, 4)) return false ;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
									    NULL, NULL, NULL) ;
    if (!png_ptr) return false ;

    png_infop info_ptr = png_create_info_struct(png_ptr) ;
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL) ;
        return false ;
    }

    png_infop end_info = png_create_info_struct(png_ptr) ;
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL) ;
        return false ;
    }

    struct png_in_memory pim(data) ;
    png_set_read_fn(png_ptr, (void *)&pim, png_memory_read_data) ;

    png_read_info(png_ptr, info_ptr) ;

    png_uint_32 width, height ;
    int bit_depth, color_type, interlace_type, compression_type, filter_type ;

    png_get_IHDR(png_ptr, info_ptr, &width, &height,
			  &bit_depth, &color_type, &interlace_type,
			  &compression_type, &filter_type) ;

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
	 png_set_gray_1_2_4_to_8(png_ptr);

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	 png_set_tRNS_to_alpha(png_ptr);

    if (bit_depth == 16)
	 png_set_strip_16(png_ptr);

    Image::Encoding encoding = Image::OPAQUE ;
    switch (color_type) {
    case PNG_COLOR_TYPE_PALETTE:
	 // std::cerr << "PNG_COLOR_TYPE_PALETTE" << std::endl ;
	 png_set_palette_to_rgb(png_ptr) ;
	 png_set_strip_alpha(png_ptr); // otherwise, image is semi-transparent (why?)
	 encoding = Image::RGB ;
	 break ;
    case PNG_COLOR_TYPE_GRAY:
	 // std::cerr << "PNG_COLOR_TYPE_GRAY" << std::endl ;
	 encoding = Image::L ;
	 break ;
    case PNG_COLOR_TYPE_RGB:
	 // std::cerr << "PNG_COLOR_TYPE_RGB" << std::endl ;
	 encoding = Image::RGB ;
	 break ;
    case PNG_COLOR_TYPE_RGB_ALPHA:
	 // std::cerr << "PNG_COLOR_TYPE_RGB_ALPHA" << std::endl ;
	 png_set_swap_alpha(png_ptr) ;
	 if (target_encoding == Image::RGBA) {
	   encoding = Image::RGBA ;
	 }
	 else {
	   png_set_swap_alpha(png_ptr) ;
	   encoding = Image::ARGB ;
	 }
	 break ;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
	 // std::cerr << "PNG_COLOR_TYPE_GRAY_ALPHA" << std::endl ;
	 png_set_gray_to_rgb(png_ptr) ;
	 png_set_swap_alpha(png_ptr) ;
	 encoding = Image::ARGB ;
	 break ;
    default:
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL) ;
        return false ;
    }

    unsigned int dec_row_size = (unsigned int)(width*idst->getBytesPerPixel(encoding)) ;
    unsigned int dec_size = height*dec_row_size ;
    unsigned char *decoded = Image::AllocMem(dec_size) ;

    unsigned char *ptr = decoded ;
    for (unsigned int i=0; i<height; ++i, ptr+=dec_row_size)
	 png_read_row(png_ptr, ptr, NULL);

    png_read_end(png_ptr, end_info) ;
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    idst->setDims(width, height) ;
    idst->setData(decoded, dec_size, Image::FREEMEM) ;
    idst->setEncoding(encoding) ;
    idst->setTimeStamp(isrc->getTimeStamp()) ;

    convertImage(idst, target_encoding, quality) ;

    return true ;
  }

  // ------------------------------------------------------------------

  void
  png_calcdims(Image *isrc) {
    unsigned char *data = isrc->getData() ;

    if (png_sig_cmp(data, (png_size_t)0, 4)) return ;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
									    NULL, NULL, NULL) ;
    if (!png_ptr) return ;

    png_infop info_ptr = png_create_info_struct(png_ptr) ;
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL) ;
        return ;
    }

    png_infop end_info = png_create_info_struct(png_ptr) ;
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL) ;
        return ;
    }

    struct png_in_memory pim(data) ;
    png_set_read_fn(png_ptr, (void *)&pim, png_memory_read_data) ;

    png_read_info(png_ptr, info_ptr) ;

    unsigned int width = png_get_image_width(png_ptr, info_ptr) ;
    unsigned int height = png_get_image_height(png_ptr, info_ptr) ;
    isrc->setDims(width, height) ;

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info) ;
  }

}
