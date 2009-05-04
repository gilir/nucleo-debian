/*
 *
 * nucleo/core/PluginManager.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>
#include <nucleo/nucleo.H>
#include <nucleo/core/PluginManager.H>
#include <nucleo/utils/FileUtils.H>
#include <nucleo/utils/StringUtils.H>

#include <fstream>
#include <stdexcept>
#include <iostream>

#if HAVE_DLFCN_H
#include <dlfcn.h>
#elif HAVE_DL_H
#include <dl.h>
#endif

#define DEBUG_LEVEL 0

namespace nucleo {

  // -----------------------------------------------------------------------------------

  PluginManager *PluginManager::singleton = 0 ;

  void
  PluginManager::addEntry(std::string service, std::string tag,
					 Module *module, std::string symbol) {
    Service *s = 0 ;
    PluginDirectory::iterator ipd =directory.find(service) ;
    if (ipd==directory.end()) {
	 s = new Service ;
	 directory[service] = s ;
    } else
	 s = (*ipd).second ;

    s->insert(std::pair<std::string,Plug*>(tag,new Plug(module,symbol))) ;
  }    

  void
  PluginManager::loadList(void) {
    std::string plist = pluginlistdir ;
    if (plist!="") {
	 if (plist[plist.size()-1]!='/') plist = plist+'/' ;
	 plist = plist+"plugin-list" ;
    } else {
	 plist = "plugin-list" ;
    }

#if DEBUG_LEVEL>0
    std::cerr << "PluginManager: loading plugin-list from " << plist << std::endl ;
#endif

    try {
	 unsigned int s = getFileSize(plist.c_str()) ;
	 s = 0 ; // unsued variable...
    } catch (std::runtime_error e) {
	 std::cerr << e.what() << std::endl ;
	 return ;
    }

    std::ifstream stream(plist.c_str()) ;
    char tmpline[256] ;
    int indent=0, level=0 ;
    std::string library, path, service, symbol, tag ;
    bool serviceWithoutTag = false ;

    Module *module = 0 ;

    for (;;) {
	 if (stream.eof()) break ;

	 stream.getline(tmpline, 256) ;
	   
	 if (tmpline[0]=='\0' || tmpline[0]=='#') continue ;

	 int previndent = indent ;
	 for (indent=0; tmpline[indent]==' ' || tmpline[indent]=='\t'; ++indent) ;
	 if (tmpline[indent]=='\0') continue ;

	 if (indent) {
	   if (indent>previndent) {
		level++ ;
		serviceWithoutTag = false ;
	   } else if (indent<previndent)
		level-- ;
	 } else
	   level = 0 ;

	 if (serviceWithoutTag) {
	   addEntry(service, "*", module, symbol) ;
	   serviceWithoutTag = false ;
	 }

	 switch (level) {
	 case 0: // library
	   library.assign(tmpline) ;
	   trimString(library) ;
	   path = plugindir ;
	   if (path[path.size()-1]!='/') path = path+"/" ;
	   path = path + library ;
	   module = new Module(library, path) ;
	   break ;
	 case 1: // service symbol
	   symbol.assign(tmpline) ;
	   service = extractNextWord(symbol) ;
	   trimString(service) ;
	   trimString(symbol) ;
	   serviceWithoutTag = true ;
	   break ;
	 case 2: // tag
	   tag.assign(tmpline) ;
	   trimString(tag) ;
	   addEntry(service, tag, module, symbol) ;
	   serviceWithoutTag = false ;
	   break ;
	 default:
	   std::cerr << "PluginManager warning: indentation level is " << level << std::endl ;
	 }
    }

    if (serviceWithoutTag) addEntry(service, "*", module, symbol) ;

  }

  // -----------------------------------------------------------------------------------

  PluginManager::PluginManager(void) {
    plugindir = getNucleoPluginsDirectory() ;
    pluginlistdir = getNucleoResourcesDirectory() ;
    loadList() ;
  }

  // -----------------------------------------------------------------------------------

  void *
  PluginManager::find(const std::string service, const std::string tag) {
    Plug *plug = 0 ;
    PluginDirectory::iterator ipd = directory.find(service) ;
    if (ipd!=directory.end()) {
	 Service *s = (*ipd).second ;
	 Service::iterator is = (tag!="*") ? s->find(tag) : s->begin() ;
	 if (is!=s->end()) plug = (*is).second ;
    }

    if (!plug) throw std::runtime_error("PluginManager: can't find "+service+"/"+tag) ;

#if HAVE_DLOPEN
    void *handle=0 ;
    if (plug->module->handle) {
#if DEBUG_LEVEL>0
	 std::cerr << "PluginManager: using existing handle (" << plug->module->handle << ") for " << plug->module->name << std::endl ;
#endif
	 handle = (void *)plug->module->handle ;
    } else {
	 handle = dlopen(plug->module->path.c_str(), RTLD_LAZY) ;
    }
    if (!handle) throw std::runtime_error(std::string("PluginManager: ")+dlerror()) ;  
    void *addr = dlsym(handle, plug->symbol.c_str()) ;
    char *error = (char *)dlerror() ;
    if (error != NULL)  throw std::runtime_error(std::string("PluginManager: ")+error) ;
#if DEBUG_LEVEL>0
    std::cerr << "PluginManager: found " << plug->symbol << " (handle=" << plug->module->handle << " addr=" << addr << ")" << std::endl ;
#endif
    return addr ;
#endif

    throw std::runtime_error("PluginManager: no built-in support for plugins...") ;
  }

  void *
  PluginManager::getSymbol(const std::string service, const std::string tag) {
    if (!singleton) singleton = new PluginManager ;
    return singleton->find(service, tag) ;
  }

}
