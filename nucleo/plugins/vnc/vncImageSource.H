#ifndef n_vncImageSource_H_
#define n_vncImageSource_H_

/*
 *
 * nucleo/plugins/vnc/vncImageSource.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/source/ImageSource.H>
#include <nucleo/network/tcp/TcpConnection.H>
#include <nucleo/core/URI.H>
#include <nucleo/core/FileKeeper.H>

namespace nucleo {

  class vncImageSource : public ImageSource {

    // See http://www.realvnc.com/

  private:
    
    std::string _hostname ;
    int _port ;
    std::string _password ;

    Image::Encoding _rencoding ;

    TcpConnection *_conn ;
    std::string _buffer ;

    Image _subimg ;

    void _receive(char *data, unsigned int length) ;

  protected:

    void react(Observable*) ;

  public:

    vncImageSource(Image::Encoding e, const URI& uri) ;
    ~vncImageSource(void) { stop() ; }

    ImageSource::state getState(void) { return (_conn?STARTED:STOPPED) ; }

    bool start(void) ;
    using ImageSource::getNextImage ;
    bool getNextImage(Image *img, TimeStamp::inttype reftime) ;
    bool stop(void) ;

    void updateRequest(bool include=true) ;
    void updateRequest(int x, int y, int w, int h, bool incremental=true) ;
    void keyEvent(unsigned long keysym, bool down_flag=true) ;
    void pointerEvent(int x, int y, unsigned char button_mask=0) ;

  } ;

}

#endif
