#ifndef n_qtSgImageSource_H
#define n_qtSgImageSource_H

/*
 *
 * nucleo/image/source/qtSgImageSource.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/source/ImageSource.H>
#include <nucleo/core/URI.H>
#include <nucleo/core/TimeKeeper.H>
#include <nucleo/core/TimeStamp.H>
#include <nucleo/network/udp/UdpReceiver.H>

#include <QuickTime/QuickTime.h>

namespace nucleo {

  class qtSgImageSource : public ImageSource {

  private:

    SeqGrabComponent component ;
    SGChannel channel ;
    QTVisualContextRef visualContext ;
    TimeScale timeScale ;
    ImageDescriptionHandle imgDescription ;
    ICMDecompressionSessionRef decompSession ;
    long frameNumber ;
    CVImageBufferRef cvimage ;
    GWorldPtr offscreen ;
    int iWidth, iHeight ;
    TimeStamp::inttype timeStamp ;

    Image::Encoding qtEncoding ;
    TimeKeeper *timer ;

    URI uri ;
    int qtqtEncoding ;

    std::string nSignature ;
    UdpReceiver *nSignal ;

    static void
    cb_3_ready(QTVisualContextRef visualContext, const CVTimeStamp *timeStamp, void *refCon) ;

    static void
    cb_2_decompress(void *decompressionTrackingRefCon,
				OSStatus result,
				ICMDecompressionTrackingFlags decompressionTrackingFlags,
				CVPixelBufferRef pixelBuffer,
				TimeValue64 displayTime,
				TimeValue64 displayDuration,
				ICMValidTimeFlags validTimeFlags,
				void *sourceFrameRefCon, void *reserved) {
    }

    static pascal OSErr 
    cb_1_grab(SGChannel c, Ptr data, long length, long *offset, long chRefCon, 
		    TimeValue timeValue, short writeType, long refCon) ;

  protected:

    void react(Observable*) ;

  public:

    qtSgImageSource(const URI &uri, Image::Encoding e) ;
    ~qtSgImageSource() ;

    ImageSource::state getState(void) { return (timer?STARTED:STOPPED) ; }

    bool start(void) ;
    bool getNextImage(Image *img, TimeStamp::inttype reftime) ;
    bool stop(void) ;

    bool setRate(double r) ;
    double getRate(void) ;

  } ;

}

#endif
