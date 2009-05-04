#ifndef n_nudpImageSource_H
#define n_nudpImageSource_H

/*
 *
 * nucleo/image/source/nudpImageSource.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/source/ImageSource.H>
#include <nucleo/core/URI.H>
#include <nucleo/core/FileKeeper.H>
#include <nucleo/network/udp/UdpReceiver.H>
#include <nucleo/image/encoding/Conversion.H>

namespace nucleo {

  // ------------------------------------------------------------------

  class nudpImageSource : public ImageSource {

  private:

    std::string _hostOrGroup ;
    int _port ;
    Image::Encoding source_encoding ;

    UdpReceiver *_udp ;

  protected:

    void init(std::string hostOrGroup, int port,
		    Image::Encoding source_encoding,
		    Image::Encoding target_encoding) ;
    
    void react(Observable*) ;

  public:

    nudpImageSource(std::string hostOrGroup, int port,
				Image::Encoding source_encoding,
				Image::Encoding target_encoding) ;

    nudpImageSource(const URI &uri, Image::Encoding target_encoding) ;
    ~nudpImageSource() { stop() ; }

    ImageSource::state getState(void) { return (_udp?STARTED:STOPPED) ; }

    bool start(void) ;
    bool getNextImage(Image *img, TimeStamp::inttype reftime) ;
    bool stop(void) ;

    int getPortNumber(void) { return _udp ? _udp->getPortNumber() : 0 ; }
    std::string getURI(void) ;

  } ;

  // ------------------------------------------------------------------

}

#endif
