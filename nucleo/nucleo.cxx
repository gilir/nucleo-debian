/*
 *
 * nucleo/nucleo.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>
#include <nucleo/nucleo.H>
#include <nucleo/utils/FileUtils.H>

#include <iostream>
#include <cstdlib>

#include <libgen.h>

#define VERBOSITY 0

namespace nucleo {

#ifdef __APPLE__

#include <CoreFoundation/CoreFoundation.h>

  static inline void
  n_debugCreatorAndType(UInt32 packageCreator, UInt32 packageType) {
    char *cr = (char *)&packageCreator ;
    char *ty = (char *)&packageType ;
    std::cerr << "packageCreator: " 
		    << cr[0] << cr[1] << cr[2] << cr[3] 
		    << ", packageType: " 
		    << ty[0] << ty[1] << ty[2] << ty[3]
		    << std::endl ;
  }

  static inline std::string
  n_getSTLStringFromCFURLRef(CFURLRef url) {
    char tmp[1025] ;
    CFURLRef abs_url = CFURLCopyAbsoluteURL(url) ;
    CFStringRef path = CFURLCopyFileSystemPath(abs_url,kCFURLPOSIXPathStyle) ;
    CFStringGetCString(path, tmp, 1024, kCFStringEncodingISOLatin1) ;
    CFRelease(path) ;
    CFRelease(abs_url) ;
    return std::string(tmp) ;
  }

  static inline CFBundleRef
  n_getNucleoBundle(void) {
    // First, try to find the Nucleo framework
    CFStringRef nucleoID = CFSTR("fr.lri.insitu.nucleo") ;
    CFBundleRef bundle = CFBundleGetBundleWithIdentifier(nucleoID) ;
    if (bundle) return bundle ;

    // Assume the application uses libNucleo and has everything it needs...
    bundle = CFBundleGetMainBundle() ;
    if (!bundle) return bundle ;

    UInt32 packageType, packageCreator ;
    CFBundleGetPackageInfo (bundle, &packageType, &packageCreator) ;
#if VERBOSITY>1
    n_debugCreatorAndType(packageCreator, packageType) ;
#endif
    // This is indeed a properly packaged application
    if (packageType=='APPL') return bundle ;

    // This must be a command-line tool...
    return 0 ;
  }
#endif
  
  // -------------------------------------------------------

  std::string
  getNucleoVersion(void) {
    return PACKAGE_VERSION ;
  }

  // -------------------------------------------------------

  std::string
  getNucleoPluginsDirectory(void) {
    char *envVar = getenv("NUCLEO_PLUGINS_DIR") ;
    if (envVar) {
#if VERBOSITY>0
	 std::cerr << "getPluginsDirectory: " << envVar << " (environment)" << std::endl ;
#endif
	 return envVar ;
    }

#ifdef __APPLE__
    CFBundleRef bundle = n_getNucleoBundle() ;
    if (bundle) {
	 CFURLRef url = CFBundleCopyBuiltInPlugInsURL(bundle) ;
	 std::string path = n_getSTLStringFromCFURLRef(url) ;
	 CFRelease(url) ;
	 if (fileExists(path.c_str())) {
#if VERBOSITY>0
	   std::cerr << "getPluginsDirectory: " << path << " (bundle resource)" << std::endl ;
#endif
	   return path ;
	 }
    }
#endif

#if VERBOSITY>0
    std::cerr << "getPluginsDirectory: " << NUCLEO_PLUGINS_DIR << " (compile-time default)" << std::endl ;
#endif
    return NUCLEO_PLUGINS_DIR ;
  }

  // -------------------------------------------------------

  std::string
  getNucleoResourcesDirectory(void) {
    char *envVar = getenv("NUCLEO_RESOURCES_DIR") ;
    if (envVar) {
#if VERBOSITY>0
	 std::cerr << "getResourcesDirectory: " << envVar << " (environment)" << std::endl ;
#endif
	 return envVar ;
    }

#ifdef __APPLE__
    CFBundleRef bundle = n_getNucleoBundle() ;
    if (bundle) {
	 CFURLRef url = CFBundleCopyResourcesDirectoryURL(bundle) ;
	 std::string path = n_getSTLStringFromCFURLRef(url) ;
	 CFRelease(url) ;
	 if (fileExists(path.c_str())) {
#if VERBOSITY>0
	   std::cerr << "getResourcesDirectory: " << path << " (bundle resource)" << std::endl ;
#endif
	   return path ;
	 }
    }
#endif

#if VERBOSITY>0
    std::cerr << "getResourcesDirectory: " << NUCLEO_RESOURCES_DIR << " (compile-time default)" << std::endl ;
#endif
    return NUCLEO_RESOURCES_DIR ;
  }

  // -------------------------------------------------------

}
