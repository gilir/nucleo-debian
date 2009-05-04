#ifndef n_TimeStamp_H
#define n_TimeStamp_H

/*
 *
 * nucleo/core/TimeStamp.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <stdint.h>

#include <string>

namespace nucleo {

  class TimeStamp {

  public:

    /**
	* This class is used to represent points in time.
	* 
	* The date+time information is stored as a single 64-bit integer
	* with millisecond precision, which makes it easy to compute
	* durations. It can also be converted to/from a UTC ISO 8601
	* string-based representation.
	* 
	* TimeStamps are described as the number of milliseconds elapsed
	* since 1970-01-01T00:00:00.000Z. The minimum date+time that can
	* be represented is 1901-12-13T20:45:52Z. The maximum one is
	* 2038-01-19T03:14:07Z. These values were chosen to remain
	* compatible with POSIX 32-bit based time_t representations.
	*
	* Useful links: 
	*     - http://en.wikipedia.org/wiki/Unix_time
	*     - http://en.wikipedia.org/wiki/ISO_8601
	*
	*/

    typedef int64_t inttype ;

    static inttype undef, min, max ; // undef < min < max

  protected:

    inttype milliseconds ;

    static inttype now(void) ;

    static inttype string2int(std::string s) ;
    static std::string int2string(inttype i) ;

    static inttype ext2int(int year, int month, int day, 
					  int hour, int min, int sec, int msec) ;
    static void int2ext(inttype t, 
				    int *year, int *month, int *day, 
				    int *hour, int *min, int *sec, int *msec, 
				    int *wday, int *yday, bool utc) ;

  public:

    TimeStamp(void) : milliseconds(now()) {}
    TimeStamp(TimeStamp &s) : milliseconds(s.milliseconds) {}

    // All the specified times are assumed to be UTC-based
    TimeStamp(inttype ms) ;
    TimeStamp(std::string s) : milliseconds(string2int(s)) {}

    TimeStamp(int year, int month, int day, 
		    int hour, int min, int sec, int msec) : 
	 milliseconds(ext2int(year, month, day, hour, min, sec, msec)) {}

    bool operator<(const TimeStamp &other) const {
	 return (milliseconds<other.milliseconds) ;
    }

    bool operator>(const TimeStamp &other) const {
	 return (milliseconds>other.milliseconds) ;
    }

    TimeStamp::inttype operator=(const TimeStamp::inttype t) ;

    bool operator==(const TimeStamp &other) const {
	 return (milliseconds==other.milliseconds) ;
    }

    bool operator==(const TimeStamp::inttype t) const {
	 return (milliseconds==t) ;
    }

    TimeStamp::inttype operator-(const TimeStamp &other) const {
	 return milliseconds-other.milliseconds ;
    }

    TimeStamp::inttype operator+(const TimeStamp &other) const {
	 return milliseconds+other.milliseconds ;
    }

    TimeStamp::inttype operator-(const TimeStamp::inttype t) const {
	 return milliseconds-t ;
    }

    TimeStamp::inttype operator+(const TimeStamp::inttype t) const {
	 return milliseconds+t ;
    }

    // the number of milliseconds since 1970-01-01T00:00:00.000Z
    inttype getAsInt(void) {
	 return milliseconds ; 
    }

    // a UTC ISO 8601 combined data/time string (e.g. 2006-02-25T11:59:12.113Z)
    std::string getAsString(void) {
	 return int2string(milliseconds) ; 
    } 

    // Use 0 for components you don't want
    void getAsUTCTime(int *year, int *month, int *day,
				  int *hour, int *min, int *sec, int *msec,
				  int *wday=0, int *yday=0) {
	 return int2ext(milliseconds, 
				 year, month, day, 
				 hour, min, sec, msec, 
				 wday, yday,
				 true) ;
    }

    // Use 0 for components you don't want
    void getAsLocalTime(int *year, int *month, int *day,
				    int *hour, int *min, int *sec, int *msec,
				    int *wday=0, int *yday=0) {
	 return int2ext(milliseconds, 
				 year, month, day, hour, min, sec, msec, 
				 wday, yday, 
				 false) ;
    }

    static inttype getLocalUTCOffset(void) ;

    static inttype createAsInt(void) {return now() ; }
    static inttype createAsIntFrom(std::string s) { return string2int(s) ; }
    static inttype createAsIntFrom(int year, int month, int day, int hour, int min, int sec, int msec) { return ext2int(year,month,day,hour,min,sec,msec) ; }

    static std::string createAsString(void) { return int2string(now()) ; }
    static std::string createAsStringFrom(inttype i) { return int2string(i) ; }
    static std::string createAsStringFrom(int year, int month, int day, int hour, int min, int sec, int msec) { return int2string(ext2int(year,month,day,hour,min,sec,msec)) ; }

    // Deprecated syntax...
    static inttype createAsIntFromString(std::string s) { return string2int(s) ; }
    static std::string createAsStringFromInt(inttype i) { return int2string(i) ; }

  } ;

}

#endif
