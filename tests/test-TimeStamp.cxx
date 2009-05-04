/*
 *
 * tests/test-TimeStamp.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/TimeStamp.H>
#include <nucleo/utils/FileUtils.H>

#include <iostream>

using namespace nucleo ;

inline void
showIntDate(TimeStamp::inttype t) {
  try {
    TimeStamp ts(t) ;
    std::cout << "  " << t << " --> " << ts.getAsString() << " --> " << ts.getAsInt() << std::endl ;
  } catch (...) {
    std::cout << "  showIntDate: exception caught when trying to display " << t << std::endl ;
  }
}

inline void
showStringDate(std::string t) {
  try {
    TimeStamp ts(t) ;
    std::cout << "  " << t << " --> " << ts.getAsInt() << " --> " << ts.getAsString() << std::endl ;
  } catch (...) {
    std::cout << "  showStringDate: exception caught when trying to display " << t << std::endl ;
  }
}

int
main(int argc, char **argv) {
  std::cout << "-- Special values --" << std::endl << std::endl ;

  TimeStamp undef(TimeStamp::undef) ; std::cout << "  undef: " << undef.getAsString() << std::endl ;
  TimeStamp mini(TimeStamp::min) ; std::cout << "  mini: " << mini.getAsString() << std::endl ;
  TimeStamp zero(0) ; std::cout << "  zero: " << zero.getAsString() << std::endl ;
  TimeStamp maxi(TimeStamp::max) ; std::cout << "  maxi: " << maxi.getAsString() << std::endl ;
  std::cout << "  mini<maxi: " << (mini<maxi) << std::endl ;
  std::cout << "  mini>maxi: " << (mini>maxi) << std::endl ;
  std::cout << "  mini==maxi: " << (mini==maxi) << std::endl ;
  
  std::cout << std::endl << "-- 32-bit min/max --" << std::endl << std::endl ;

  showStringDate("1901-12-13T20:45:52Z") ;
  showIntDate(-2147483648000LL) ;
  showIntDate(-2147483648000LL-1000) ;
  std::cout << std::endl ;
  showStringDate("2038-01-19T03:14:07Z") ;
  showIntDate(2147483647000LL) ;
  showIntDate(2147483647000LL+1000) ;

  std::cout << std::endl << "-- Now --" << std::endl << std::endl ;

  TimeStamp now ;
  TimeStamp::inttype now_as_int = now.getAsInt() ;
  std::cout << "  as int: " << now_as_int << std::endl ;
  std::string nowAsString = now.getAsString() ;
  std::cout << "  as string: " << nowAsString << std::endl ;
  TimeStamp t2(now_as_int) ;
  std::cout << "  (as int) as int: " << t2.getAsInt() << std::endl ;
  std::cout << "  (as int) as string: " << t2.getAsString() << std::endl ;
  TimeStamp t3(nowAsString) ;
  std::cout << "  (as string) as int: " << t3.getAsInt() << std::endl ;
  std::cout << "  (as string) as string: " << t3.getAsString() << std::endl ;

  int year=0, month=0, day=0, hour=0, min=0, sec=0, msec=0 ;
  now.getAsLocalTime(&year,&month,&day,&hour,&min,&sec,&msec) ;
  std::cout << "  as local time: "
		  << year << "-" << month << "-" << day
		  << "T" << hour << ":" << min << ":" << sec << "." << msec
		  << std::endl ;

  std::cout << std::endl << "-- Now +/- something --" << std::endl << std::endl ;

  TimeStamp::inttype one_second = 1000 ;
  TimeStamp::inttype one_minute = 60*one_second ;
  TimeStamp::inttype one_hour = 60*one_minute ;
  TimeStamp::inttype one_day = 24*one_hour ;
  TimeStamp::inttype one_week = 7*one_day ;

  std::cerr << "Now: " << now.getAsString() << std::endl ;
  std::cerr << "  - 1 second: " << TimeStamp::createAsStringFromInt(now.getAsInt()-one_second) << std::endl ;
  std::cerr << "  - 1 minute: " << TimeStamp::createAsStringFromInt(now.getAsInt()-one_minute) << std::endl ;
  std::cerr << "  - 1 hour:   " << TimeStamp::createAsStringFromInt(now.getAsInt()-one_hour) << std::endl ;
  std::cerr << "  - 1 day:    " << TimeStamp::createAsStringFromInt(now.getAsInt()-one_day) << std::endl ;
  std::cerr << "  - 1 week:   " << TimeStamp::createAsStringFromInt(now.getAsInt()-one_week) << std::endl ;

  now.getAsUTCTime(&year,&month,&day,&hour,&min,&sec,&msec) ;
  std::cerr << "  - 1 month:  " << TimeStamp::createAsStringFrom(year, month-1, day, hour, min, sec, msec) << std::endl ;
  std::cerr << "  + 3 month:  " << TimeStamp::createAsStringFrom(year, month+3, day, hour, min, sec, msec) << std::endl ;

  std::cerr << "  - 1 year:   " << TimeStamp::createAsStringFrom(year-1, month, day, hour, min, sec, msec) << std::endl ;
  std::cerr << "  + 3 year:   " << TimeStamp::createAsStringFrom(year+3, month, day, hour, min, sec, msec) << std::endl ;

  std::cout << std::endl << "-- UTC offset: " << TimeStamp::getLocalUTCOffset() << " --" << std::endl << std::endl ;

  if (argc>1) {
    std::cout << std::endl << "-- " << argv[1] << " --" << std::endl << std::endl ;
    TimeStamp filetime(getFileTime(argv[1])) ;
    std::cout << "  as int: " << filetime.getAsInt() << std::endl ;
    std::cout << "  as string: " << filetime.getAsString() << std::endl ;
  }
  
  return 0 ;
}
