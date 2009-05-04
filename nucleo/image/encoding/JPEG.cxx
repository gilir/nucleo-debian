/*
 *
 * nucleo/image/encoding/JPEG.cxx --
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
#include <nucleo/image/encoding/JPEG.H>
#include <nucleo/image/processing/basic/Transform.H>

#if HAVE_LIBEXIF
#include <libexif/exif-data.h>
#include <libexif/exif-utils.h>
#endif

extern "C" {
#undef HAVE_STDLIB_H
#include <jpeglib.h>
}

#include <cstdio>
#include <setjmp.h>

#include <stdexcept>

namespace nucleo {

  static const int ESTIMATED_JPEG_HEADER_LENGTH=1024 ;

  struct jpeg_error_manager {
    struct jpeg_error_mgr pub ;    /* "public" fields */
    jmp_buf setjmp_buffer;        /* for return to caller */
  } ;

  static void
  jpeg_error_exit (j_common_ptr cinfo) {
    // (*cinfo->err->output_message) (cinfo) ;
    jpeg_error_manager *mgr = (jpeg_error_manager*)cinfo->err ;  
    longjmp(mgr->setjmp_buffer, 1) ;
  }

  // ------------------------------------------------------------------

  typedef struct {
    struct jpeg_destination_mgr pub ;
    JOCTET *buffer ;
    int size ;
  } jpeg_sink_manager ;

  METHODDEF(void)
    init_destination (j_compress_ptr cinfo) {
    jpeg_sink_manager *dest = (jpeg_sink_manager*)cinfo->dest ;
    dest->pub.next_output_byte = dest->buffer ;
    dest->pub.free_in_buffer = dest->size ;
  }

  METHODDEF(boolean)
    empty_output_buffer (j_compress_ptr) {
    return FALSE ;
  }

  METHODDEF(void) term_destination (j_compress_ptr) {}

  bool
  jpeg_encode(Image *isrc, Image *idst, unsigned int quality) {
    Image source ;
    source.linkDataFrom(*isrc) ;

    struct jpeg_compress_struct cinfo ;
    struct jpeg_error_mgr jerr ;
    cinfo.err = jpeg_std_error(&jerr) ; 
    jpeg_create_compress(&cinfo) ;

    int bpp=0 ;
    switch (source.getEncoding()) {
#if 0
    case Image::YpCbCr422:
	 bpp = 2 ;
	 cinfo.in_color_space = JCS_YCbCr ;
	 cinfo.input_components = 3 ;
	 break ;
#endif
    case Image::L:
	 bpp = 1 ;
	 cinfo.in_color_space = JCS_GRAYSCALE ;
	 cinfo.input_components = 1 ;
	 break ;
    default: 
	 convertImage(&source, Image::RGB, quality) ;
	 bpp = 3 ;
	 cinfo.in_color_space = JCS_RGB ;
	 cinfo.input_components = 3 ;
	 break ;
    }

    cinfo.dest = (jpeg_destination_mgr *) new jpeg_sink_manager ; 

    jpeg_set_defaults(&cinfo) ;
    cinfo.dct_method = JDCT_FLOAT ;
    cinfo.image_width = source.getWidth() ;
    cinfo.image_height = source.getHeight() ;

    if (cinfo.in_color_space == JCS_YCbCr) {
	 cinfo.comp_info[0].h_samp_factor = 2 ;
	 cinfo.comp_info[0].v_samp_factor = 2 ;
	 cinfo.comp_info[1].h_samp_factor = 1 ;
	 cinfo.comp_info[1].v_samp_factor = 1 ;
	 cinfo.comp_info[2].h_samp_factor = 1 ;
	 cinfo.comp_info[2].v_samp_factor = 1 ;
	 cinfo.raw_data_in = TRUE ;
    }

    jpeg_sink_manager *dest = (jpeg_sink_manager*)cinfo.dest ;
    dest->size = ESTIMATED_JPEG_HEADER_LENGTH + cinfo.image_width*cinfo.image_height*bpp ;
    dest->pub.init_destination = init_destination ;
    dest->pub.empty_output_buffer = empty_output_buffer ;
    dest->pub.term_destination = term_destination ;
    dest->pub.next_output_byte = 0 ;
    dest->pub.free_in_buffer = 0 ;
    dest->buffer = (JOCTET *)Image::AllocMem(dest->size) ;

    jpeg_set_quality(&cinfo, quality, TRUE) ;
    jpeg_start_compress(&cinfo, TRUE) ;

    if (cinfo.in_color_space != JCS_YCbCr) {
	 int row_stride = cinfo.image_width * bpp ;
	 JSAMPROW row_pointer[cinfo.image_height] ;
	 JOCTET *ptr = (JOCTET*)source.getData() ;
	 for (unsigned int i=0; i<cinfo.image_height; ++i) {
	   row_pointer[i] = ptr ;
	   ptr += row_stride ;
	 }
	 for (unsigned int i=cinfo.image_height; i>0; )
	   i -= jpeg_write_scanlines(&cinfo, & row_pointer[cinfo.image_height-i], i) ;
    } else {
	 JSAMPROW y[16],cb[16],cr[16]; // y[2][5] = color sample of row 2 and pixel column 5; (one plane)
	 JSAMPARRAY data[3]; 
	 data[0] = y;
	 data[1] = cb;
	 data[2] = cr;
	 JOCTET *ptr = (JOCTET*)source.getData() ;
	 for (unsigned int j=0;j<cinfo.image_height;j+=16) {
	   for (int i=0;i<16;i++) {
		y[i] = ptr + cinfo.image_width*(i+j);
		if (i%2 == 0) {
		  cb[i/2] = ptr + cinfo.image_width*cinfo.image_height + cinfo.image_width/2*((i+j)/2);
		  cr[i/2] = ptr + cinfo.image_width*cinfo.image_height + cinfo.image_width*cinfo.image_height/4 + cinfo.image_width/2*((i+j)/2);
		}
	   }
	   jpeg_write_raw_data (&cinfo, data, 16);
	 }
    }

    jpeg_finish_compress(&cinfo) ;

    idst->setEncoding(Image::JPEG) ;
    idst->setData(dest->buffer, dest->size - dest->pub.free_in_buffer, Image::FREEMEM) ;
    idst->setTimeStamp(isrc->getTimeStamp()) ;

    delete cinfo.dest ;
    jpeg_destroy_compress(&cinfo) ;

    return true ;
  }

  // ------------------------------------------------------------------

  typedef struct {
    struct jpeg_source_mgr pub ;
    JOCTET firstBuffer[ESTIMATED_JPEG_HEADER_LENGTH] ;
    JOCTET* input ;
    int input_size ;
  } jpeg_source_manager ;

  METHODDEF(void) init_source (j_decompress_ptr cinfo) {
    jpeg_source_manager *src = (jpeg_source_manager*)cinfo->src ;
    src->pub.next_input_byte = src->input ;
    src->pub.bytes_in_buffer = src->input_size ;
  }

  METHODDEF(boolean)
    fill_input_buffer (j_decompress_ptr) {
    return FALSE ;
  }

  METHODDEF(void)
    skip_input_data (j_decompress_ptr cinfo, long num_bytes) {
    jpeg_source_manager *src = (jpeg_source_manager*)cinfo->src ;
    if (num_bytes > 0) {
	 src->pub.next_input_byte += (size_t) num_bytes ;
	 src->pub.bytes_in_buffer -= (size_t) num_bytes ;
    }
  }

  METHODDEF(void) term_source (j_decompress_ptr) {}

  bool
  jpeg_decode(Image *isrc, Image *idst,
		    Image::Encoding target_encoding, unsigned int quality) {
    short orientation = 0 ;

#if HAVE_LIBEXIF
    ExifData *eData = exif_data_new_from_data(isrc->getData(), isrc->getSize()) ;
    for (unsigned int i=0; i<EXIF_IFD_COUNT; ++i) {
	 ExifEntry *entry = exif_content_get_entry(eData->ifd[i], EXIF_TAG_ORIENTATION) ;
	 if (!entry) continue ;
	 if (entry->format==EXIF_FORMAT_SHORT) {
	   orientation = exif_get_short(entry->data, exif_data_get_byte_order(eData)) ;
	   break ;
	 }
    }
    exif_data_unref(eData) ;
#endif

    J_COLOR_SPACE colorspace = JCS_RGB ;
    Image::Encoding tmpenc = Image::RGB ;
    
    if (target_encoding==Image::L) {
	 tmpenc = Image::L ;
	 colorspace = JCS_GRAYSCALE ;
    } else if (target_encoding==Image::YpCbCr422) {
	 tmpenc = Image::YpCbCr422 ;
	 colorspace = JCS_YCbCr ;
    }

    struct jpeg_decompress_struct cinfo ;
    struct jpeg_error_manager jerr;
    cinfo.err = jpeg_std_error(&jerr.pub) ; 
    jerr.pub.error_exit = jpeg_error_exit ;

    jpeg_create_decompress(&cinfo) ;

    jpeg_source_manager *src = new jpeg_source_manager ;    
    cinfo.src = (jpeg_source_mgr *)src ;
    src->pub.init_source = init_source ;
    src->pub.fill_input_buffer = fill_input_buffer ;
    src->pub.skip_input_data = skip_input_data ;
    src->pub.resync_to_restart = jpeg_resync_to_restart ; /* use default method */
    src->pub.term_source = term_source ;
    src->pub.bytes_in_buffer = 0 ; /* forces fill_input_buffer on first read */
    src->pub.next_input_byte = NULL ; /* until buffer loaded */
    src->input = (JOCTET *)isrc->getData() ;
    src->input_size = isrc->getSize() ;

    if (setjmp(jerr.setjmp_buffer)) {
	 delete cinfo.src ;
	 jpeg_destroy_decompress(&cinfo) ;
	 return false ;
    }
    jpeg_read_header(&cinfo, TRUE) ;

    cinfo.out_color_space = colorspace ;

    jpeg_calc_output_dimensions(&cinfo) ;
    const int row_stride = cinfo.image_width * cinfo.output_components ;
    int size = row_stride * cinfo.image_height ;
    JOCTET *buffer = (JOCTET *)Image::AllocMem(size) ;

    JSAMPROW row_pointer[cinfo.image_height] ;
    JOCTET *ptr = buffer ;
    for (unsigned int i=0; i<cinfo.image_height; ++i) {
	 row_pointer[i] = ptr ;
	 ptr += row_stride ;
    }

    jpeg_start_decompress(&cinfo) ;

    for (unsigned int i=cinfo.image_height; i>0; ) {
      int read=jpeg_read_scanlines(&cinfo, & row_pointer[cinfo.image_height-i], i) ;
      if(!read) {
	   delete cinfo.src ;
	   jpeg_destroy_decompress(&cinfo) ;
	   return false ;	
      }
      i -= read;
    }
    jpeg_finish_decompress(&cinfo) ;

    idst->setEncoding(tmpenc) ;
    idst->setData(buffer, size, Image::FREEMEM) ;
    idst->setDims(cinfo.image_width, cinfo.image_height) ;
    idst->setTimeStamp(isrc->getTimeStamp()) ;

    switch (orientation) {
    case 0: // unknown
    case 1: // "standard"
	 break ;
    case 6:
	 rotateImage(idst, true) ;
	 break ;
    default:
	 std::cerr << "jpeg_decode: orientation of the image is " << orientation << std::endl ;
	 break ;
    }

    convertImage(idst, target_encoding, quality) ;

    delete cinfo.src ;
    jpeg_destroy_decompress(&cinfo) ;

    return true ;
  }

  // ------------------------------------------------------------------

  void
  jpeg_calcdims(Image *isrc) {
    struct jpeg_decompress_struct cinfo ;
    struct jpeg_error_manager jerr;

    cinfo.err = jpeg_std_error(&jerr.pub) ; 
    jerr.pub.error_exit = jpeg_error_exit ;

    jpeg_create_decompress(&cinfo) ;

    cinfo.src = (jpeg_source_mgr *) new jpeg_source_manager ;    
    jpeg_source_manager *src = (jpeg_source_manager*)cinfo.src ;
    src->pub.init_source = init_source ;
    src->pub.fill_input_buffer = fill_input_buffer ;
    src->pub.skip_input_data = skip_input_data ;
    src->pub.resync_to_restart = jpeg_resync_to_restart ; /* use default method */
    src->pub.term_source = term_source ;
    src->pub.bytes_in_buffer = 0 ; /* forces fill_input_buffer on first read */
    src->pub.next_input_byte = NULL ; /* until buffer loaded */
    src->input = (JOCTET *)isrc->getData() ;
    src->input_size = isrc->getSize() ;

    if (setjmp(jerr.setjmp_buffer)) {
	 delete cinfo.src ;
	 jpeg_destroy_decompress(&cinfo) ;
	 return ;
    }

    jpeg_read_header(&cinfo, TRUE) ;
    jpeg_calc_output_dimensions(&cinfo) ;

    unsigned int iWidth = (unsigned int)cinfo.image_width ;
    unsigned int iHeight = (unsigned int)cinfo.image_height ;

    delete cinfo.src ;
    jpeg_destroy_decompress(&cinfo) ;

#if HAVE_LIBEXIF
    ExifData *eData = exif_data_new_from_data(isrc->getData(), isrc->getSize()) ;
    for (unsigned int i=0; i<EXIF_IFD_COUNT; ++i) {
	 ExifEntry *entry = exif_content_get_entry(eData->ifd[i], EXIF_TAG_ORIENTATION) ;
	 if (!entry) continue ;
	 if (entry->format==EXIF_FORMAT_SHORT) {
	   if (exif_get_short(entry->data, exif_data_get_byte_order(eData))==6) {
		unsigned int tmp = iWidth ;
		iWidth = iHeight ;
		iHeight = tmp;
	   }
	   break ;
	 }
    }
    exif_data_unref(eData) ;
#endif

    isrc->setDims(iWidth, iHeight) ;
  }

}
