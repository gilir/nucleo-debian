#ifndef n_AppleIRController_H
#define n_AppleIRController_H

/*
 *
 * nucleo/helpers/AppleIRController.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/ReactiveObject.H>

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>

#include <list>
#include <queue>
#include <map>

namespace nucleo {

  class AppleIRController : public Observable {

  public:

    typedef enum {
	 Unknown,
	 PlayPause, Long_PlayPause,
	 VolumeUp_Press,
	 VolumeUp_Release,
	 VolumeDown_Press,
	 VolumeDown_Release,
	 Menu, Long_Menu,
	 Forward, Long_Forward_Press, Long_Forward_Release,
	 Backward, Long_Backward_Press, Long_Backward_Release
    } event_type ;

    struct event {
	 int control_id ;
	 event_type type ;
	 std::string name ;
    } ;

  protected:

    io_object_t device ;
    IOHIDDeviceInterface **interface ;
    std::list<IOHIDElementCookie> cookies ;
    IOHIDQueueInterface **queue ;
    CFRunLoopSourceRef eventSource ;

    int id ;
    std::queue<std::string> messages ;
    std::map<std::string, std::pair<event_type,std::string> > hidmsg2event ;

    void listProperties(void) ;

    static void hidEventCallback(void *target, IOReturn result, void *refcon, void *sender) ;

  public:

    int debugLevel ;

    AppleIRController(int debugLevel=0) ;
    ~AppleIRController(void) ;

    bool getNextEvent(event *) ;

  } ;

}

#endif
