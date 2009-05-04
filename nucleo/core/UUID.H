#ifndef n_UUID_H
#define n_UUID_H

/*
 *
 * nucleo/core/UUID.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <string>

namespace nucleo {

  class UUID {

  public:

    typedef unsigned char bytearray[16] ;

  private:

    bytearray bytes ;

  public:

    UUID(void) ;

    // Fills the specified bytearray
    void getAsBytes(bytearray output) ;

    // Returns a pointer to the internal bytearray or its size
    unsigned char *getBytes(void) { return (unsigned char*)bytes ; }
    unsigned int getSize(void) { return 16 ; }

    // returns a 36 characters string (e.g. B4639888-1394-4EDD-BD08-56BF11DE1A2B)
    std::string getAsString(void) ;

    static void createAsBytes(bytearray output) ;
    static void createAsBytesFromString(std::string input, bytearray output) ;

    static std::string createAsString(void) ;
    static std::string createAsStringFromBytes(bytearray input) ;

  } ;

}

#endif
