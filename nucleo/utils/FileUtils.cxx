/*
 *
 * nucleo/utils/FileUtils.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>
#include <nucleo/utils/FileUtils.H>

#include <sys/ioctl.h>
#include <cstdio>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <dirent.h>
#include <libgen.h>

#if HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif

#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <cstring>

namespace nucleo {

  // -------------------------------------------------------------------------

  std::string
  reducePath(std::string p) {
    const char *path = p.c_str() ;
    char *result ;
    int ipath, iresult ;

    int lpath = strlen(path) ;
    result = new char [lpath] ;

    // Initial slashes are special...
    for (ipath=iresult=0; path[ipath]=='/'; ipath++) {
	 result[iresult++] = '/' ;
    }

    int dots=0 ;
    while (ipath<lpath) {

	 char c = path[ipath] ;

	 // std::cerr << "ipath=" << ipath << " c=" << c << endl ;

	 switch (c) {

	 case '.':
	   dots++ ;
	   break ;

	 case '/':
	   switch (dots) {
	   case 1: // Found a ./
		if (iresult>0 && result[iresult-1]=='/')
		  // It's a /./
		  dots=0 ;
		else {
		  result[iresult++] = '.' ;
		  result[iresult++] = '/' ;
		}
		break ;
	   case 2: // Found a ../
		if (iresult>0 && result[iresult-1]=='/') {
		  // It's a /../
		  int prev ;
		  for (prev=iresult-2; prev>0 && result[prev]!='/'; prev--) ;
		  // prev is the index of last /
		  if (prev>=0) {
		    iresult = prev+1 ;
		  } else {
		    result[iresult++] = '.' ;
		    result[iresult++] = '.' ;
		    result[iresult++] = '/' ;
		  }
		} else {
		  result[iresult++] = '.' ;
		  result[iresult++] = '.' ;
		  result[iresult++] = '/' ;
		}
		dots = 0 ;
		break ;
	   default:
		result[iresult++] = '/' ;
		break ;
	   }
	   break ;

	 default:
	   while (dots--) result[iresult++] = '.' ;
	   dots = 0 ;
	   result[iresult++] = c ;
	   break ;
	 }

	 ipath++ ;
    }

    while (dots--) result[iresult++] = '.' ;
    result[iresult] = '\0' ;

    std::string r ;
    r.assign(result) ;
    delete [] result ;
    return r ;
  }

  bool
  fileExists(const char *filename) {
    struct stat statinfo ;
    if (stat(filename, &statinfo)==-1) return false ;
    return true ;
  }

  bool fileIsDir(const char *filename) {
    struct stat statinfo ;
    if (stat(filename, &statinfo)==-1) return false ;
    return S_ISDIR(statinfo.st_mode) ;
  }

  uint64_t
  getFileSize(const char *filename) {
    struct stat statinfo ;
    if (stat(filename, &statinfo)==-1) return 0 ;
    return statinfo.st_size ;
  }

  TimeStamp::inttype
  getFileTime(const char *filename) {
    struct stat statinfo ;
    if (stat(filename, &statinfo)==-1) return 0 ;
    TimeStamp::inttype milliseconds = (TimeStamp::inttype)statinfo.st_mtime*(TimeStamp::inttype)1000 ;
    return milliseconds ;
  }

  void
  readFromFile(const char *filename, unsigned char *data, unsigned int size) {
    int fd = open(filename, O_RDONLY) ;
    if (fd==-1) {
	 std::string msg("can't open ") ;
	 msg.append(filename) ;
	 msg.append(" (readFromFile)") ;
	 throw std::runtime_error(msg) ;
    }
    if (read(fd, (char*) data, size)!=(int)size) {
	 std::string msg("can't read from ") ;
	 msg.append(filename) ;
	 msg.append(" (readFromFile)") ;
	 throw std::runtime_error(msg) ;
    }
    close(fd) ;
  }

  const char *
  getExtension(const char *filename) {
    int iext ;
    for (iext=strlen(filename)-1; iext>=0 && filename[iext]!='.'; --iext) ;
    if (iext<0) return 0 ;
    return filename+iext ;
  }

  int
  createFile(const char *filename) {
    int fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR) ;
    fchmod(fd, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH) ;
    return fd ;
  }

  bool
  createDir(const char *path) {
    if (fileIsDir(path)) {
	 // std::cerr << "createDir: " << path << " is a directory" << std::endl ;
	 return true ;
    }
    char *tmppath = strdup(path) ;
    std::string parent = strdup(dirname(tmppath)) ;
    if (!createDir(parent.c_str())) {
	 // std::cerr << "createDir: unable to create " << parent << std::endl ;
	 free(tmppath) ;
	 return false ;
    }
    int result = mkdir(path,S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH) ;
    // std::cerr << "createDir: mkdir returned " << result << " for " << path << std::endl ;
    free(tmppath) ;
    return result==0 ;
  }

  void
  setblocking(int fd, int doblock) {
    long nonblocking = !doblock ;
    if (ioctl(fd, FIONBIO, &nonblocking)==-1)
	 throw std::runtime_error("ioctl FIONBIO failed (setblocking)") ;
  }

  int
  getavail(int fd) {
    int avail ;
    if (ioctl(fd, FIONREAD, &avail)==-1) 
	 throw std::runtime_error("ioctl FIONREAD failed (getavail)") ;
      // std::cerr << __FILE__ << " (" << __LINE__ << "): " << avail << std::endl ;
    return avail ;
  }

  int
  readOneLine(int fd, char *buffer, int size) {
    char c ;
    int ok=0, len=0 ;
    while (len<size) {
	 buffer[len]='\0' ;
	 if (read(fd, &c, 1)<1) break ;
	 ok = 1 ;
	 if (c=='\n') break ;
	 buffer[len++] = c ;
    }

    if (ok) return len ;
    throw std::runtime_error("nothing to read (readOneLine)") ;
  }

  bool
  listFiles(std::string path,
		  std::vector<std::string> *filenames,
		  bool recursive, bool includedirs) {
    // FIXME: use fts_open et al.

    struct stat buf ;
    if (!stat(path.c_str(), &buf)) {
	 if (!S_ISDIR(buf.st_mode)) {
	   filenames->push_back(path) ;
	   return true ;
	 }
    } else {
	 std::cerr << "listFiles: unable to stat " << path << std::endl ;
	 return false;
    }

    bool result = true ;

    struct dirent **namelist;
    int n = scandir(path.c_str(), &namelist, 0, alphasort) ;
    if (n<0) return false ;
    for (int i=0; i<n; ++i) {
	 std::string filename = namelist[i]->d_name ;
	 if (filename!="." && filename!="..") {
	   std::string fullname = path+"/"+filename ;
	   if (!stat(fullname.c_str(), &buf)) {
		if (S_ISDIR(buf.st_mode)) {
		  if (includedirs) filenames->push_back(fullname) ;
		  if (recursive) listFiles(fullname,filenames) ;
		} else {
		  filenames->push_back(fullname) ;
		}
	   } else {
		std::cerr << "listFiles: unable to stat " << fullname << std::endl ;
		result = false ;
	   }
	 }
	 free(namelist[i]) ;
    }
    free(namelist) ;

    return result ;
  }
  
  // -------------------------------------------------------------------------

}
