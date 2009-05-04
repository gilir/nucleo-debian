/*
 *
 * nucleo/core/TimeStamp.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>

#define __STDC_LIMIT_MACROS 1
#include <nucleo/core/TimeStamp.H>

#include <sys/time.h>
#include <time.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>

namespace nucleo {

  TimeStamp::inttype TimeStamp::undef = INT64_MIN ;
  TimeStamp::inttype TimeStamp::min = -2147483648000LL ;
  TimeStamp::inttype TimeStamp::max = 2147483647000LL ;

  // ----------------------------------------------------------------------

  TimeStamp::TimeStamp(inttype ms) {
    if ((ms>=TimeStamp::min && ms<=TimeStamp::max) || ms==TimeStamp::undef)
	 milliseconds = ms ;
    else
	 throw std::runtime_error("TimeStamp value out of range") ;
  }

  TimeStamp::inttype
  TimeStamp::operator=(const TimeStamp::inttype t) {
    if ((t>=TimeStamp::min && t<=TimeStamp::max) || t==TimeStamp::undef)
	 milliseconds = t ;
    else
	 throw std::runtime_error("TimeStamp value out of range") ;
    return t ;
  }

  TimeStamp::inttype 
  TimeStamp::now(void) {
    struct timeval stamp ;
    gettimeofday(&stamp, 0) ;
    return (inttype)stamp.tv_sec*(inttype)1000 +  (inttype)stamp.tv_usec/(inttype)1000 ;
  }

  TimeStamp::inttype 
  TimeStamp::string2int(std::string s) {
    bool isPureInt = true ;
    inttype ms = 0 ;
    for (unsigned int i=0; i<s.size(); ++i) {
	 if (s[i]<'0' || s[i]>'9') {isPureInt = false ; break ;}
	 ms = 10*ms + (s[i]-'0') ;
    }
    if (isPureInt) return ms ;

    // 2006-02-25T11:59:12.113449
    // 2006_02_25_11_59_12_113449
    // 012345678901234567890
    //           1         2

    int msec = 0 ;
    struct tm aTm ;
    bzero(&aTm, sizeof(aTm)) ;
    /*int nbitems = */ sscanf(s.c_str(),"%4d-%2d-%2dT%2d:%2d:%2d.%3dZ",
						&aTm.tm_year,&aTm.tm_mon,&aTm.tm_mday,
						&aTm.tm_hour,&aTm.tm_min,&aTm.tm_sec,
						&msec) ;

#if 0
    std::cerr << nbitems << " items parsed: "
		    << std::setfill('0') << std::setw(4) << aTm.tm_year
		    << "-" << std::setfill('0') << std::setw(2) << aTm.tm_mon
		    << "-" << std::setfill('0') << std::setw(2) << aTm.tm_mday
		    << "T" << std::setfill('0') << std::setw(2) << aTm.tm_hour 
		    << ":" << std::setfill('0') << std::setw(2) << aTm.tm_min 
		    << ":" << std::setfill('0') << std::setw(2) << aTm.tm_sec 
		    << "." << std::setfill('0') << std::setw(3) << msec 
		    << std::endl ;
#endif

    aTm.tm_year -= 1900 ;
    aTm.tm_mon-- ;

    ms = (inttype)timegm(&aTm) * (inttype)1000L + msec ;
    return ms ;
  }

  std::string
  TimeStamp::int2string(TimeStamp::inttype milliseconds) {
    time_t sec = (time_t)(milliseconds/1000) ;
    inttype msec = milliseconds - (milliseconds/1000)*1000 ;
    // std::cout << milliseconds << " " << sec << " " << msec << std::endl ;
    if (milliseconds<0 && msec!=0) {
	 sec -- ;
	 msec += 1000 ;
    }

    struct tm *tm = gmtime(&sec) ;
    // std::cerr << "-- isdst: " << tm->tm_isdst << " zone: " << tm->tm_zone << " gmtoff: " << tm->tm_gmtoff << " --" << std::endl ;

    std::stringstream result ;
    result << std::setfill('0') << std::setw(4) << 1900+tm->tm_year
		 << "-" << std::setfill('0') << std::setw(2) << 1+tm->tm_mon
		 << "-" << std::setfill('0') << std::setw(2) << tm->tm_mday
		 << "T" << std::setfill('0') << std::setw(2) << tm->tm_hour 
		 << ":" << std::setfill('0') << std::setw(2) << tm->tm_min 
		 << ":" << std::setfill('0') << std::setw(2) << tm->tm_sec
		 << "." << std::setfill('0') << std::setw(3) << (int)msec 
		 << "Z" ;
    return result.str() ;
  }

  TimeStamp::inttype
  TimeStamp::ext2int(int year, int month, int day, 
				 int hour, int min, int sec, int msec) {
    struct tm tm ;
    tm.tm_sec = sec ;
    tm.tm_min = min ;
    tm.tm_hour = hour ;
    tm.tm_mday = day ;
    tm.tm_mon = month-1 ;
    tm.tm_year = year - 1900 ;
    tm.tm_wday = tm.tm_yday = 0 ; // ignored anyway
    tm.tm_isdst = tm.tm_gmtoff = 0 ; // ignored anyway
    tm.tm_zone = 0 ;

    return (inttype)timegm(&tm) * (inttype)1000L + msec ;
  }

  void
  TimeStamp::int2ext(TimeStamp::inttype t, 
				 int *year, int *month, int *day,
				 int *hour, int *min, int *sec, int *msec,
				 int *wday, int *yday,
				 bool gmt) {
    time_t seconds = (time_t)(t/1000) ;
    if (msec) *msec = (int)(t-seconds*1000) ;

    struct tm *tm = gmt ? gmtime(&seconds) : localtime(&seconds) ;
    if (year) *year = 1900+tm->tm_year ;
    if (month) *month = 1+tm->tm_mon ;
    if (day) *day = tm->tm_mday ;
    if (hour) *hour = tm->tm_hour ;
    if (min) *min = tm->tm_min ;
    if (sec) *sec = tm->tm_sec ;
    if (wday) *wday = tm->tm_wday ;
    if (yday) *yday = tm->tm_yday ;
  }

  TimeStamp::inttype
  TimeStamp::getLocalUTCOffset(void) {
    time_t seconds = (time_t)(now()/1000) ;
    struct tm *tm = localtime(&seconds) ;
    return tm->tm_gmtoff*1000L ;
  }

  // ----------------------------------------------------------------------

}
