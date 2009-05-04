#ifndef n_PluginManager_H
#define n_PluginManager_H

/*
 *
 * nucleo/core/PluginManager.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <string>
#include <map>

namespace nucleo {

  class PluginManager {

  private:

    std::string plugindir ;
    std::string pluginlistdir ;

    /*
	* plugin-list is a list of:
	*
	*   <library-name>
	*      <service-1> <symbol-1>
	*          <tag-1>
	*          ...
	*          <tag-n>
	*      ...
	*      <service-m> <symbol-m>
	*          <tag-1>
	*          ...
	*          <tag-p>
	*/

    struct Module {
	 std::string name ;
	 std::string path ;
	 void *handle ;
	 Module(std::string n, std::string p) : name(n), path(p), handle(0) {}
    } ;

    struct Plug {
	 Module *module ;
	 std::string symbol ;
	 Plug(Module *m, std::string s) : module(m), symbol(s) {}
    } ;

    typedef std::multimap<std::string, Plug*> Service ; // key is a tag
    typedef std::map<std::string, Service*> PluginDirectory ;  // key is a service name

    PluginDirectory directory ;

    PluginManager(void) ;
    void addEntry(std::string service, std::string tag,
			   Module *module, std::string symbol) ;
    void loadList(void) ;
    void *find(const std::string service, const std::string tag) ;

    static PluginManager *singleton ;

  public:

    // Use "*" to match "any tag"
    static void *getSymbol(const std::string service, const std::string tag) ;

  } ;

}

#endif
