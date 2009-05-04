/*
 *
 * nucleo/helpers/AppleIRController.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/helpers/AppleIRController.H>

#include <IOKit/IOCFPlugIn.h>

#include <stdexcept>
#include <iostream>
#include <sstream>

// http://www.cocoadev.com/index.pl?UsingTheAppleRemoteControl
// http://developer.apple.com/documentation/DeviceDrivers/Conceptual/HID/workingwith/

namespace nucleo {

  // ------------------------------------------------------------------

  AppleIRController::AppleIRController(int dlevel) {
    debugLevel = dlevel ;
    device = 0 ;
    interface = 0 ;
    eventSource = 0 ;
    id = 0 ;
    hidmsg2event["14_12_11_6_5__2"] = std::pair<event_type,std::string>(VolumeUp_Press,"VolumeUp_Press") ;
    hidmsg2event["14_12_11_6_5__0"] = std::pair<event_type,std::string>(VolumeUp_Release,"VolumeUp_Release") ;
    hidmsg2event["14_13_11_6_5__3"] = std::pair<event_type,std::string>(VolumeDown_Press,"VolumeDown_Press") ;
    hidmsg2event["14_13_11_6_5__0"] = std::pair<event_type,std::string>(VolumeDown_Release,"VolumeDown_Release") ;
    hidmsg2event["14_8_6_5_14_8_6_5__3"] = std::pair<event_type,std::string>(PlayPause,"PlayPause") ;
    hidmsg2event["18_14_6_5_18_14_6_5__5"] = std::pair<event_type,std::string>(Long_PlayPause,"Long_PlayPause") ;
    hidmsg2event["14_7_6_5_14_7_6_5__2"] = std::pair<event_type,std::string>(Menu,"Menu") ;
    hidmsg2event["14_6_5_14_6_5__1"] = std::pair<event_type,std::string>(Long_Menu,"Long_Menu") ;
    hidmsg2event["14_9_6_5_14_9_6_5__4"] = std::pair<event_type,std::string>(Forward,"Forward") ;
    hidmsg2event["14_6_5_4_2__3"] = std::pair<event_type,std::string>(Long_Forward_Press,"Long_Forward_Press") ;
    hidmsg2event["14_6_5_4_2__0"] = std::pair<event_type,std::string>(Long_Forward_Release,"Long_Forward_Release") ;
    hidmsg2event["14_10_6_5_14_10_6_5__5"] = std::pair<event_type,std::string>(Backward,"Backward") ;
    hidmsg2event["14_6_5_3_2__2"] = std::pair<event_type,std::string>(Long_Backward_Press,"Long_Backward_Press") ;
    hidmsg2event["14_6_5_3_2__0"] = std::pair<event_type,std::string>(Long_Backward_Release,"Long_Backward_Release") ;

    // -----------------------

    CFMutableDictionaryRef hidMatchDictionary = IOServiceMatching("AppleIRController") ;

    io_iterator_t hidObjectIterator = 0 ;
    if (kIOReturnSuccess!=IOServiceGetMatchingServices(kIOMasterPortDefault, 
											hidMatchDictionary,
											&hidObjectIterator))
	 throw std::runtime_error("AppleIRController: no matching HID device") ;

    IOCFPlugInInterface **plugin = 0 ;
    while ( (device=IOIteratorNext(hidObjectIterator)) ) {
	 SInt32 score = 0 ;	
	 if (kIOReturnSuccess==IOCreatePlugInInterfaceForService(device,
												  kIOHIDDeviceUserClientTypeID,
												  kIOCFPlugInInterfaceID,
												  &plugin, &score))
	   break ;
    }
    IOObjectRelease(hidObjectIterator);
    if (!device || !plugin)
	 throw std::runtime_error("AppleIRController: IOCreatePlugInInterfaceForService failed") ;

    if (debugLevel>1) listProperties() ;

    HRESULT plugInResult = (*plugin)->QueryInterface(plugin,
										   CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID),
										   (void**)&interface) ;
    (*plugin)->Release(plugin) ;
    if (plugInResult!=S_OK)
	 throw std::runtime_error("AppleIRController: Couldn't create HID class device interface") ;

    CFArrayRef elements ;
    if (kIOReturnSuccess!=(*(IOHIDDeviceInterface122 **)interface)->copyMatchingElements(interface, NULL, &elements))
	 throw std::runtime_error("AppleIRController: Couldn't get HID cookies from the device") ;

    for (CFIndex i=0; i<CFArrayGetCount(elements); i++) {
	 CFDictionaryRef element = (CFDictionaryRef)CFArrayGetValueAtIndex(elements, i) ;
	 long number ;
	 CFTypeRef object = (CFDictionaryGetValue(element, CFSTR(kIOHIDElementCookieKey)));
	 if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID()) continue;
	 if(!CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, &number)) continue;
	 cookies.push_back((IOHIDElementCookie)number) ;
    }

    if ((*interface)->open(interface, kIOHIDOptionsTypeSeizeDevice) == kIOReturnExclusiveAccess)
	 std::cerr << "AppleIRController: someone has already got exclusive access" << std::endl ;

    queue = (*interface)->allocQueue(interface) ;
    if (!queue)
	 throw std::runtime_error("AppleIRController: Couldn't allocate interface queue") ;

    if ((*queue)->create(queue, 0, 12) != kIOReturnSuccess)
	 throw std::runtime_error("AppleIRController: Unable to create queue") ;

    for (std::list<IOHIDElementCookie>::iterator i=cookies.begin(); i!=cookies.end(); ++i) 
	 (*queue)->addElement(queue, *i, 0) ;

    if ((*queue)->createAsyncEventSource(queue, &eventSource) != kIOReturnSuccess )
	 std::cerr << "AppleIRController: createAsyncEventSource failed" << std::endl ;

    if ((*queue)->setEventCallout(queue, hidEventCallback, this, 0) != kIOReturnSuccess ) 
	 std::cerr << "AppleIRController: setEventCallout failed" << std::endl ;

    CFRunLoopAddSource(CFRunLoopGetCurrent(), eventSource, kCFRunLoopCommonModes) ;

    if ((*queue)->start(queue)!=kIOReturnSuccess)
	 std::cerr << "AppleIRController: start failed" << std::endl ;

    if (debugLevel>0) std::cerr << "AppleIRController: ready!" << std::endl ;
  }

  // ------------------------------------------------------------------

  void
  AppleIRController::hidEventCallback(void *target, IOReturn result, void *refcon, void *sender) {
    AppleIRController *rc = (AppleIRController*)target ;

    IOHIDEventStruct event ;
    AbsoluteTime zeroTime = {0,0} ;
    std::stringstream message ;
    SInt32 sumOfValues = 0 ;
    while (result == kIOReturnSuccess) {
	 result = (*rc->queue)->getNextEvent(rc->queue, &event, zeroTime, 0) ;
	 if (result!=kIOReturnSuccess) continue ;
	 // if (rc->debugLevel>0) std::cerr << "    cookie: " << event.elementCookie << " value: " << event.value << std::endl ;
	 if ((long)event.elementCookie == 19) {
	   rc->id = event.value ;
	   rc->messages.push("19__0") ;
	   rc->notifyObservers() ;
	   // FIXME: clear sumOfValues and message?
	 } else {
	   sumOfValues += event.value ;
	   message << (int)event.elementCookie << "_" ;
	 }
    }
    message << "_" << sumOfValues ;
    rc->messages.push(message.str()) ;
    rc->notifyObservers() ;
  }

  // ------------------------------------------------------------------

  void
  AppleIRController::listProperties(void) {
    char path[512] ;
    kern_return_t result = IORegistryEntryGetPath(device, kIOServicePlane, path);
    if (result == KERN_SUCCESS) std::cerr << "[ " << path << " ]" << std::endl ;
    else std::cerr << "AppleIRController: IORegistryEntryGetPath failed" << std::endl ;

    CFMutableDictionaryRef properties = 0;
    result = IORegistryEntryCreateCFProperties(device, &properties,
									  kCFAllocatorDefault, kNilOptions) ;

    if ((result == KERN_SUCCESS) && properties) {
	 CFStringRef s = CFCopyDescription(properties) ;
	 char cs[128000] ; 
	 if (CFStringGetCString(s, cs, 128000, kCFStringEncodingISOLatin1))
	   std::cerr << cs << std::endl ;
	 else
	   std::cerr << "AppleIRController: description buffer overflow" << std::endl ;
	 CFRelease(s) ;
	 CFRelease(properties);
    } else
	 std::cerr << "AppleIRController: IORegistryEntryCreateCFProperties failed" << std::endl ;
  }

  bool
  AppleIRController::getNextEvent(event *e) {
    if (messages.empty() || !e) return false ;

    std::string msg = messages.front() ;
    messages.pop() ;

    std::map<std::string,std::pair<event_type,std::string> >::iterator i = hidmsg2event.find(msg) ;
    if (i!=hidmsg2event.end()) {
	 e->type = (*i).second.first ;
	 e->name = (*i).second.second ;
    } else {
	 e->type = Unknown ;
	 e->name = msg ;
    }
    e->control_id = id ;
    return true ;
  }

  // ------------------------------------------------------------------

  AppleIRController::~AppleIRController(void) {
    if (debugLevel>0) std::cerr << "AppleIRController: bye..." << std::endl ;
  
    if (eventSource) {
	 CFRunLoopRemoveSource(CFRunLoopGetCurrent(),eventSource,kCFRunLoopDefaultMode) ;
	 CFRelease(eventSource) ;
    }
    if (interface) {
	 (*interface)->close(interface) ;
	 (*interface)->Release(interface) ;
    }
    if (device) IOObjectRelease(device) ;
  }

  // ------------------------------------------------------------------

}
