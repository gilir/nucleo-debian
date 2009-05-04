/*
 *
 * nucleo/plugins/vnc/vncImageSource.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#include <nucleo/plugins/vnc/vncImageSource.H>
#include <nucleo/image/processing/basic/Paint.H>
#include <nucleo/image/encoding/Conversion.H>
#include <nucleo/gl/window/keysym.H>
#include <nucleo/utils/FileUtils.H>
#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/utils/ByteOrder.H>

#include <stdint.h>
typedef uint32_t CARD32 ;
typedef uint16_t CARD16 ;
typedef uint8_t CARD8 ;

extern "C" {
#include <rfbproto.h>
#include <vncauth.h>
}

#include <unistd.h>
#include <cstdio>
#include <cmath>

#include <stdexcept>

/*
 * NOTE: VNC supports bitsPerPixel of 8, 16 and 32 only (not 24). Raw
 * RGB is in fact raw RGBA...
 */

/*
 * When leaving a window, should we generate keyRelease events for
 * pressed modifiers (VNC client does so) ?
 */

#define DEBUG_LEVEL 0

// ------------------------------------------------------------------------------------------------------

extern "C" {
  void *vncImageSource_factory(const nucleo::URI *u, nucleo::Image::Encoding e) {
    return (void *)(new nucleo::vncImageSource(e, *u)) ;
  }
}

// ------------------------------------------------------------------------------------------------------

namespace nucleo {

#if DEBUG_LEVEL>=1
  static void
  PrintPixelFormat(rfbPixelFormat *format) {
    if (format->bitsPerPixel == 1) {
	 fprintf(stderr,"  Single bit per pixel.\n");
	 fprintf(stderr,
		    "  %s significant bit in each byte is leftmost on the screen.\n",
		    (format->bigEndian ? "Most" : "Least"));
    } else {
	 fprintf(stderr,"  %d bits per pixel.\n",format->bitsPerPixel);
	 if (format->bitsPerPixel != 8) {
	   fprintf(stderr,"  %s significant byte first in each pixel.\n",
			 (format->bigEndian ? "Most" : "Least"));
	 }
	 if (format->trueColour) {
	   fprintf(stderr,"  True colour: max red %d green %d blue %d",
			 format->redMax, format->greenMax, format->blueMax);
	   fprintf(stderr,", shift red %d green %d blue %d\n",
			 format->redShift, format->greenShift, format->blueShift);
	 } else {
	   fprintf(stderr,"  Colour map (not true colour).\n");
	 }
    }
  }
#endif

  // ----------------------------------------------------------------------

  void
  vncImageSource::_receive(char *data, unsigned int length) {
    int bytestoread = length-_buffer.length() ;
    if (bytestoread>0) {
	 int fd = _conn->getFd() ;
	 int buffersize = (bytestoread<1024) ? 1024 : bytestoread ;
	 char *tmpbuffer = new char [buffersize] ;
	 int bytesread = 0 ;
	 while (bytesread<bytestoread) {
	   int newbytes = read(fd, (char *)(tmpbuffer+bytesread), buffersize-bytesread) ;
	   // std::cerr << "newbytes = " << newbytes << std::endl ;
	   if (newbytes==-1) {
		return ;
		throw std::runtime_error("vncImageSource: Connection closed") ;
	   }
	   bytesread += newbytes ;
	   // std::cerr << newbytes << " new bytes (" << bytestoread << ", " << bytestoread-bytesread << " to go)" << std::endl ;
	 }
	 _buffer.append((char *)tmpbuffer, bytesread) ;
	 delete [] tmpbuffer ;
    }
    // std::cerr << "Reading " << length << " bytes from buffer" << std::endl ;
    memmove(data, _buffer.c_str(), length) ;
    _buffer.erase(0, length) ;
  }

  // ------------------------------------------------------------------

  vncImageSource::vncImageSource(Image::Encoding encoding, const URI& uri) {
    _rencoding = (encoding==Image::PREFERRED||encoding==Image::CONVENIENT) ? Image::ARGB : encoding ;
    _hostname = uri.host ;
    _port = 5900+uri.port ;
    URI::getQueryArg(uri.query, "password", &_password) ;
    _conn = 0 ;
  }

  bool
  vncImageSource::start(void) {
    try {

	 _conn = new TcpConnection(_hostname, _port) ;
	 subscribeTo(_conn) ;

	 // -------------------------------------------------------------------------------
	 // Greetings

	 rfbProtocolVersionMsg pv;
	 _receive(pv, sz_rfbProtocolVersionMsg) ;
	 pv[sz_rfbProtocolVersionMsg] = 0 ;

	 int major,minor;
	 if (sscanf(pv,rfbProtocolVersionFormat,&major,&minor) != 2)
	   throw std::runtime_error("vncImageSource: not a valid VNC server") ;

#if DEBUG_LEVEL>=1
	 std::cerr << "vncImageSource: VNC server supports protocol version" << major << "." << minor ;
	 std::cerr << "(viewer " << rfbProtocolMajorVersion << "." << rfbProtocolMinorVersion << ")" << std::endl ;
#endif

	 major = rfbProtocolMajorVersion ;
	 minor = rfbProtocolMinorVersion ;
	 sprintf(pv,rfbProtocolVersionFormat,major,minor) ;
	 _conn->send(pv, sz_rfbProtocolVersionMsg) ;

	 // -------------------------------------------------------------------------------
	 // Authentication

	 CARD32 authScheme ;
	 _receive((char *)&authScheme, 4) ;
	 authScheme = ByteOrder::swap32ifle(authScheme);

	 switch (authScheme) {

	 case rfbConnFailed: {
	   CARD32 reasonLen ;
	   _receive((char *)&reasonLen, 4) ;
	   reasonLen = ByteOrder::swap32ifle(reasonLen) ;
	   char *reason = new char [reasonLen] ;
	   _receive(reason, reasonLen) ;
	   std::string msg = "vncImageSource: VNC connection failed (" ;
	   msg.append(reason, reasonLen) ;
	   msg.append(")") ;
	   throw std::runtime_error(msg) ;
	 } break ;

	 case rfbNoAuth:
#if DEBUG_LEVEL>=1
	   std::cerr << "vncImageSource: No authentication needed" << std::endl ;
#endif
	   break;

	 case rfbVncAuth: {
	   CARD8 challenge[CHALLENGESIZE];
	   _receive((char *)challenge, CHALLENGESIZE) ;

	   if (_password!="") {
		vncEncryptBytes(challenge, (char *)_password.c_str()) ;
	   } else {
		char *passwd ;
		do
		  passwd = getpass("VNC Password: ");
		while ((!passwd) || (strlen(passwd) == 0)) ;
		if (strlen(passwd) > 8) passwd[8] = '\0';
		vncEncryptBytes(challenge, passwd);
		// "Forget" the clear-text password
		for (int i = strlen(passwd); i >= 0; i--)  passwd[i] = '\0';
	   }

	   _conn->send((char *)challenge, CHALLENGESIZE) ;

	   CARD32 authResult;
	   _receive((char *)&authResult, 4) ;
	   authResult = ByteOrder::swap32ifle(authResult);

	   switch (authResult) {
	   case rfbVncAuthOK:
#if DEBUG_LEVEL>=1
		std::cerr << "vncImageSource: VNC authentication succeeded" << std::endl ;
#endif
		break;
	   case rfbVncAuthFailed:
		throw std::runtime_error("vncImageSource: VNC authentication failed") ;
	   case rfbVncAuthTooMany:
		throw std::runtime_error("vncImageSource: VNC authentication failed - too many tries");
	   default:
		throw std::runtime_error("vncImageSource: Unknown VNC authentication result") ;
	   }
	 } break;

	 default:
	   throw std::runtime_error("Unknown authentication scheme from VNC server") ;
	 }

	 // -------------------------------------------------------------------------------

	 rfbClientInitMsg ci;
	 ci.shared = 1 ;
	 _conn->send((char *)&ci, sz_rfbClientInitMsg) ;

	 rfbServerInitMsg si;
	 _receive((char *)&si, sz_rfbServerInitMsg) ;

	 si.framebufferWidth = ByteOrder::swap16ifle(si.framebufferWidth);
	 si.framebufferHeight = ByteOrder::swap16ifle(si.framebufferHeight);
	 si.format.redMax = ByteOrder::swap16ifle(si.format.redMax);
	 si.format.greenMax = ByteOrder::swap16ifle(si.format.greenMax);
	 si.format.blueMax = ByteOrder::swap16ifle(si.format.blueMax);
	 si.nameLength = ByteOrder::swap32ifle(si.nameLength);

	 char *desktopName = new char [si.nameLength + 1] ;
	 _receive(desktopName, si.nameLength) ;

	 desktopName[si.nameLength] = 0;

#if DEBUG_LEVEL>=1
	 fprintf(stderr,"Desktop name \"%s\"\n",desktopName);
	 fprintf(stderr,"Connected to VNC server, using protocol version %d.%d\n",
		    rfbProtocolMajorVersion, rfbProtocolMinorVersion);
	 fprintf(stderr,"VNC server default format:\n");
	 PrintPixelFormat(&si.format);
#endif

	 // --- Pixel format ------------------------------------------

	 rfbPixelFormat simple_format ;
	 simple_format.bitsPerPixel = 32 ;
	 simple_format.depth = 24 ;
	 simple_format.bigEndian = ByteOrder::isLittleEndian() ? 0 : 1 ;
	 simple_format.trueColour = 1 ;
	 simple_format.redMax = simple_format.greenMax = simple_format.blueMax = 255 ;
	 // we want ARGB
	 if (ByteOrder::isLittleEndian()) {
	   simple_format.redShift = 8 ;
	   simple_format.greenShift = 16 ;
	   simple_format.blueShift = 24 ;
	 } else {
	   simple_format.blueShift = 0 ;
	   simple_format.greenShift = 8 ;
	   simple_format.redShift = 16 ;
	 }
	 simple_format.pad1 = simple_format.pad2 = 0 ; // makes the compiler happy...

	 rfbSetPixelFormatMsg spf;
	 spf.type = rfbSetPixelFormat;
	 spf.format = simple_format ;
	 spf.format.redMax = ByteOrder::swap16ifle(spf.format.redMax);
	 spf.format.greenMax = ByteOrder::swap16ifle(spf.format.greenMax);
	 spf.format.blueMax = ByteOrder::swap16ifle(spf.format.blueMax);
	 _conn->send((char *)&spf, sz_rfbSetPixelFormatMsg) ;

	 // --- Encoding ----------------------------------------------

	 char buf[sz_rfbSetEncodingsMsg + 5 * 4]; // 5 = MAX_ENCODINGS
	 rfbSetEncodingsMsg *se = (rfbSetEncodingsMsg *)buf;
	 CARD32 *encs = (CARD32 *)(&buf[sz_rfbSetEncodingsMsg]);
	 se->type = rfbSetEncodings;
	 se->nEncodings = 0;
	 encs[se->nEncodings++] = ByteOrder::swap32ifle(rfbEncodingRaw) ;
	 encs[se->nEncodings++] = ByteOrder::swap32ifle(rfbEncodingCoRRE);
#if 0
	 // Not (yet) implemented...
	 encs[se->nEncodings++] = ByteOrder::swap32ifle(rfbEncodingCopyRect);
	 encs[se->nEncodings++] = ByteOrder::swap32ifle(rfbEncodingHextile);
	 encs[se->nEncodings++] = ByteOrder::swap32ifle(rfbEncodingRRE);
#endif

	 int len = sz_rfbSetEncodingsMsg + se->nEncodings * 4;
	 se->nEncodings = ByteOrder::swap16ifle(se->nEncodings);
	 _conn->send(buf, len) ;

	 // --- Image setup -------------------------------------------

	 // ARGB is 4 bytes per pixel
	 unsigned int imgSize = si.framebufferWidth*si.framebufferHeight*4 ;

	 unsigned char *imgPtr = Image::AllocMem(imgSize) ;
	 lastImage.setEncoding(Image::ARGB) ;
	 lastImage.setDims(si.framebufferWidth, si.framebufferHeight) ;
	 lastImage.setData(imgPtr, imgSize, Image::FREEMEM) ;

	 unsigned char *subimgPtr = Image::AllocMem(imgSize) ;
	 _subimg.setEncoding(Image::ARGB) ;
	 _subimg.setData(subimgPtr, imgSize, Image::FREEMEM) ;

	 // --- Here we go ! ------------------------------------------

	 updateRequest(false) ;
	 frameCount = 0 ; previousImageTime = TimeStamp::undef ; sampler.start() ;
	 return true ;

    } catch (std::runtime_error e) {
	 std::cerr << e.what() << std::endl ;
	 stop() ;
	 return false ;
    }
  }

  void
  vncImageSource::react(Observable*) {
    if (!_conn) return ;

    while (getavail(_conn->getFd()) || _buffer.length()) {
	 rfbServerToClientMsg msg;
	 _receive((char *)&msg, 1) ;

	 switch (msg.type) {

	 case rfbBell:
	   std::cerr << "vncImageSource: ring my bell..." << std::endl ;
	   break ;

	 case rfbSetColourMapEntries: {
	   _receive(((char *)&msg.scme) + 1, sz_rfbSetColourMapEntriesMsg - 1) ;
	   int size = msg.scme.nColours * 3 /* * CARD16 */ ;
	   char *tmp = new char [size] ;
	   _receive(tmp, size) ;
	   delete tmp ;
	 } break ;

	 case rfbServerCutText: {
	   _receive(((char *)&msg.sct) + 1, sz_rfbServerCutTextMsg - 1) ;
	   msg.sct.length = ByteOrder::swap16ifle(msg.sct.length);
	   char *tmp = new char [msg.sct.length] ;
	   _receive(tmp, msg.sct.length) ;
	   std::cerr << "vncImageSource: new text in cut buffer '" << tmp << "'" << std::endl ;
	   delete tmp ;
	 } break ;

	 case rfbFramebufferUpdate: {
	   _receive(((char *)&msg.fu) + 1, sz_rfbFramebufferUpdateMsg - 1) ;
	   msg.fu.nRects = ByteOrder::swap16ifle(msg.fu.nRects);

	   for (unsigned int i=0; i<msg.fu.nRects; ++i) {

		rfbFramebufferUpdateRectHeader rect;
		_receive((char *)&rect, sz_rfbFramebufferUpdateRectHeader) ;
		rect.r.x = ByteOrder::swap16ifle(rect.r.x);
		rect.r.y = ByteOrder::swap16ifle(rect.r.y);
		rect.r.w = ByteOrder::swap16ifle(rect.r.w);
		rect.r.h = ByteOrder::swap16ifle(rect.r.h);
		rect.encoding = ByteOrder::swap32ifle(rect.encoding);

#if DEBUG_LEVEL>=2
		std::cerr << "vncImageSource: rfbFramebufferUpdate " << rect.r.x << "," << rect.r.y ;
		std::cerr << " " << rect.r.w << "x" << rect.r.h << std::endl ;
#endif

		if ((rect.r.h * rect.r.w) == 0) continue ;

		if ((rect.r.x+rect.r.w>(int)lastImage.getWidth())
		    || (rect.r.y+rect.r.h>(int)lastImage.getHeight())) continue ;

		switch (rect.encoding) {

		case rfbEncodingRaw: {
		  int rectImgSize = rect.r.w*rect.r.h*4 ;
		  unsigned char *ptr = Image::AllocMem(rectImgSize) ;
		  _receive((char *)ptr,rectImgSize) ;
		  Image tmpImg ;
		  tmpImg.setEncoding(Image::ARGB) ;
		  tmpImg.setDims(rect.r.w, rect.r.h) ;
		  tmpImg.setData(ptr, rectImgSize, Image::FREEMEM) ;
		  drawImageInImage(&tmpImg, &lastImage, rect.r.x, rect.r.y) ;
		  sampler.tick() ; frameCount++ ;
		  if (!_pendingNotifications) notifyObservers() ;
		} break; 

		case rfbEncodingCoRRE: {
		  rfbRREHeader hdr;
		  _receive((char *)&hdr, sz_rfbRREHeader) ;
		  hdr.nSubrects = ByteOrder::swap32ifle(hdr.nSubrects);
		  CARD32 pix;
		  _receive((char *)&pix, sizeof(pix)) ;
		  char *pixel = (char*)&pix ;
		  paintImageRegion(&lastImage,
					    rect.r.x, rect.r.y, rect.r.x+rect.r.w-1, rect.r.y+rect.r.h-1,
					    pixel[1], pixel[2], pixel[3], 255) ;
		  _receive((char *)_subimg.getData(), (unsigned int)(hdr.nSubrects*8)) ;
		  CARD8 *ptr = (CARD8 *)_subimg.getData();
		  for (unsigned int j=0; j<hdr.nSubrects; ++j) {
		    pix = *(CARD32 *)ptr;
		    ptr += 3 ;
		    int x = *++ptr;
		    int y = *++ptr;
		    int w = *++ptr;
		    int h = *++ptr;
		    ++ptr ;
		    pixel = (char*)&pix ;
		    int x1=rect.r.x+x, y1=rect.r.y+y ;
		    int x2=x1+w-1, y2=y1+h-1 ;
		    paintImageRegion(&lastImage, x1,y1, x2,y2, pixel[1],pixel[2],pixel[3],255) ;
		  }		
		  sampler.tick() ; frameCount++ ;
		  if (!_pendingNotifications) notifyObservers() ;	
		} break ;

		default:
		  std::cerr << "vncImageSource: unknown rect encoding" << std::endl ;
		}

	   }

	 } break;

	 default:
	   std::cerr << "vncImageSource: unknown message type " << (int)msg.type << " from VNC server" << std::endl ;
	   break ;
	 }
    }
  }

  bool
  vncImageSource::getNextImage(Image *img, TimeStamp::inttype reftime) {
    if (!_conn || reftime>lastImage.getTimeStamp()) return false ;
    img->linkDataFrom(lastImage) ;
    updateRequest(true) ;
    previousImageTime = lastImage.getTimeStamp() ;
    return convertImage(img, _rencoding) ;
  }

  bool
  vncImageSource::stop(void) {
    if (!_conn) return false ;

    unsubscribeFrom(_conn) ;
    delete _conn ;
    _conn=0 ;
    sampler.stop() ;
    return true ;
  }

  // ------------------------------------------------------------------

  void
  vncImageSource::updateRequest(bool incremental) {
    updateRequest(0,0,lastImage.getWidth(),lastImage.getHeight(),incremental) ;
  }

  void
  vncImageSource::updateRequest(int x, int y, int w, int h, bool incremental) {
    rfbFramebufferUpdateRequestMsg fur;

    fur.type = rfbFramebufferUpdateRequest;
    fur.incremental = incremental ? 1 : 0;
    fur.x = ByteOrder::swap16ifle(x);
    fur.y = ByteOrder::swap16ifle(y);
    fur.w = ByteOrder::swap16ifle(w);
    fur.h = ByteOrder::swap16ifle(h);

    _conn->send((char *)&fur, sz_rfbFramebufferUpdateRequestMsg) ;
  }

  void
  vncImageSource::keyEvent(unsigned long key, bool down_flag) {
    rfbKeyEventMsg ke;

    ke.type = rfbKeyEvent ;
    ke.down = down_flag ? 1 : 0 ;
    ke.key = ByteOrder::swap32ifle(key) ;
    _conn->send((char *)&ke, sz_rfbKeyEventMsg) ;
  }

  void
  vncImageSource::pointerEvent(int x, int y, unsigned char button_mask) {
    rfbPointerEventMsg pe;

    pe.type = rfbPointerEvent ;
    pe.buttonMask = button_mask ;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    pe.x = ByteOrder::swap16ifle(x) ;
    pe.y = ByteOrder::swap16ifle(y) ;
    _conn->send((char *)&pe, sz_rfbPointerEventMsg) ;
  }

}
