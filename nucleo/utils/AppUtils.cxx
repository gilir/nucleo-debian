/*
 *
 * nucleo/utils/AppUtils.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>
#include <nucleo/utils/AppUtils.H>

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

extern char* optarg ;
extern int optind ;

namespace nucleo {

  // ----------------------------------------------------------------------

  typedef struct {
    unsigned char type ;
    void* ptr ;
  } Parameter ;

  static bool
  setParameter(unsigned char type, void *ptr, char *value) {
    switch (type) {
    case 'i':
	 *((int*)ptr) = atoi(value) ;
	 break ;
    case 'u':
	 *((unsigned int*)ptr) = (unsigned int)strtol(value, (char **)NULL, 10) ;
	 break ;
    case 'l':
	 *((long*)ptr) = strtol(value,(char**)NULL,0) ;
	 break ;
    case 'd':
	 *((double*)ptr) = atof(value) ;
	 break ;
    case 'f':
	 *((float*)ptr) = (float)atof(value) ;
	 break ;
    case 's':
	 *((char**)ptr) = strdup(value) ;
	 break ;
    case 'S':
	 *((std::string*)ptr) = value ;
	 break ;
    case 'c':
	 *((unsigned char*)ptr) = (unsigned char)atoi(value) ;
	 break ;
    case ' ':
	 *((int*)ptr) = 1 ;
	 break ;
    case 'b':
	 *((bool*)ptr) = true ;
	 break ;
    default:
	 return false ;
    }

    return true ;
  }

  int
  parseCommandLine(int argc, char** argv,
			    const char* oformat, const char* pformat, ...) {
#if 0
    for (int tmp=0; tmp<argc; ++tmp)
	 std::cerr << "argv[" << tmp << "] = " << argv[tmp] << std::endl ;
#endif

    // -- Loads pairs of type/pointer in a char-indexed table -----------

    va_list ap ;
    int c ;
    Parameter argtab[256] ;
    int i, io=0, ip=0 ;
    int nbparams = strlen(pformat) ;

    for (i=0; i<256; i++) argtab[i].ptr = 0 ;

    va_start(ap,pformat) ;
    for (ip=0; ip<nbparams; ip++) {
	 void* p = va_arg(ap,void*) ;
	 argtab[(int)(oformat[io])].type = pformat[ip] ;
	 argtab[(int)(oformat[io])].ptr = p ;
	 do io++ ; while (oformat[io]==':') ;
    }
    va_end(ap) ;

    // ------------------------------------------------------------------

#ifdef __APPLE__
    if (argc==2 && !strncmp(argv[1],"-psn_",5)) {
	 // The application was launched from the Finder or with "open"

	 CFBundleRef myBundle = CFBundleGetMainBundle() ;
	 CFDictionaryRef bundleInfoDict = CFBundleGetInfoDictionary(myBundle) ;
	 if (bundleInfoDict==NULL) return 0 ;

	 char ckey[256] ;
	 strcpy(ckey,"nucleo-option-?") ;
	 CFStringRef key, value ;
	 char cvalue[1025] ; 
	 for (i=0; i<256; ++i)
	   if (argtab[i].ptr) {
		ckey[strlen(ckey)-1] = (char)i ;
		key = CFStringCreateWithCString(kCFAllocatorMalloc, ckey,
								  kCFStringEncodingISOLatin1);
		value = (CFStringRef)CFDictionaryGetValue(bundleInfoDict, key) ;
		if (value!=NULL) {
		  // std::cerr << "Found option " << ckey << std::endl ;
		  if (CFStringGetCString(value, cvalue, 1024, kCFStringEncodingISOLatin1))
		    setParameter(argtab[i].type, argtab[i].ptr, cvalue) ;
		}
	   }
	 
	 return 2 ;
    }
#endif

    // -- Use getopt to read command line arguments ---------------------

    bool ok = true ;
    while ((c = getopt(argc, argv, oformat)) != -1) {
	 if (argtab[c].ptr) {
	   ok = setParameter(argtab[c].type, argtab[c].ptr, optarg) && ok ;
	 } else {
	   ok = false ;
	 }
    }
    return (ok ? optind : -1) ; 
  }

  // ----------------------------------------------------------------------

  bool
  appIsBundled(void) {
#ifdef __APPLE__
    CFBundleRef bundle = CFBundleGetMainBundle() ;
    if (!bundle) return false ;
    UInt32 packageType, packageCreator ;
    CFBundleGetPackageInfo (bundle, &packageType, &packageCreator) ;
    if (packageType=='APPL') return true ;
#endif
    return false ;
  }

  // ----------------------------------------------------------------------

#ifdef __APPLE__
  static pascal OSErr __AEOpenDocuments_handler(const AppleEvent *appleEvt,
									   AppleEvent* reply, SInt32 refcon) {
    DocumentOpener *obj = (DocumentOpener*)refcon ;
    std::vector<std::string> theDocs ;

    AEDescList documents ;
    AECreateDesc(typeNull, NULL, 0, &documents) ;
    
    OSErr err = AEGetParamDesc(appleEvt, keyDirectObject, typeAEList, &documents) ;
    if (err != noErr) goto fail ;
    
    long n ;
    err = AECountItems(&documents, &n) ;
    if (err != noErr) goto fail;

    for (long i=1 ; i<=n; ++i) {
	 FSRef fsRef ;
	 Size actSz = sizeof(fsRef) ;
	 AEKeyword keyWd ;
	 DescType typeCd ;
	 char fullPath[PATH_MAX] ;
	 
	 err = AEGetNthPtr(&documents, i, typeFSRef, &keyWd, &typeCd,
				    (Ptr)&fsRef, sizeof(FSRef), &actSz) ;
	 if (err != noErr) goto fail ;
	 
	 err = FSRefMakePath(&fsRef, (UInt8*)fullPath, PATH_MAX) ;
	 if (err != noErr) goto fail ;
	 theDocs.push_back(fullPath) ;
    }

    obj->openDocuments(theDocs) ;
    
  fail:
    AEDisposeDesc(&documents) ;
    return(err) ;
  }
#endif

  DocumentOpener::DocumentOpener(void) {
#ifdef __APPLE__
    OSErr err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
							   NewAEEventHandlerUPP(__AEOpenDocuments_handler), (long)this,
							   false) ;
    if (err != noErr)
	 throw std::runtime_error("DocumentOpener: AEInstallEventHandler failed") ;
#else
    std::cerr << "Warning: DocumentOpener not supported on this platform" << std::endl ;
#endif
  }

  // ----------------------------------------------------------------------

}
