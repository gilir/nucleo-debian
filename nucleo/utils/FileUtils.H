#ifndef n_FileUtils_H
#define n_FileUtils_H

/*
 *
 * nucleo/utils/FileUtils.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/core/TimeStamp.H>

#include <stdint.h>

#include <string>
#include <vector>

namespace nucleo {

  std::string reducePath(std::string path) ;

  bool fileExists(const char *filename) ;
  bool fileIsDir(const char *filename) ;
  uint64_t getFileSize(const char *filename) ;
  TimeStamp::inttype getFileTime(const char *filename) ;
  void readFromFile(const char *filename, unsigned char *data, unsigned int size) ;

  int createFile(const char *filename) ;
  bool createDir(const char *dirname) ;

  const char *getExtension(const char *filename) ;

  void setblocking(int fd, int doblock) ;
  int getavail(int fd) ;
  int readOneLine(int fd, char *buffer, int size) ;

  bool listFiles(std::string path,
			  std::vector<std::string> *filenames,
			  bool recursive=true,
			  bool includedirs=false) ;

}

#endif
