#ifndef n_XmppConnection_H
#define n_XmppConnection_H

/*
 *
 * nucleo/network/xmpp/XmppConnection.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/nucleo.H>
#include <nucleo/core/ReactiveObject.H>
#include <nucleo/core/URI.H>
#include <nucleo/network/tcp/TcpConnection.H>
#include <nucleo/utils/StringUtils.H>
#include <nucleo/xml/XmlParser.H>

#if NUCLEO_SUPPORTS_GNUTLS
#include <gnutls/gnutls.h>
#endif

#include <stdarg.h>

namespace nucleo {

  class XmppConnection : public ReactiveObject {

    // http://www.xmpp.org/specs/rfc3920.html
    // http://www.xmpp.org/specs/rfc3921.html

  public:

    int debugLevel ;

    // Warning: nodes retrieved from the inbox must be deleted or
    // detached right away (the parser might delete them anytime)
    XmlParser::InBox inbox ;

  protected:

    std::ostream *logstream ;
    TcpConnection *connection ;
#if NUCLEO_SUPPORTS_GNUTLS
    gnutls_session tls_session ;
    gnutls_certificate_credentials tls_creds ;
#endif
    XmlParser parser ;
    XmlStructure *features ;
    int waiting ;
    URI uri ;

    // TCP helpers
    unsigned int pushBytes(const char *data, unsigned int length) ;
    void pullBytes(void) ;

    // XMPP helpers
    bool newStream(void) ;

    // TLS helpers
    bool tls_init(void) ;
    void tls_deinit(void) ;

    // SASL helpers
    bool authenticate_anonymous(void) ;
    bool authenticate_plain(std::string user, std::string password) ;
    bool authenticate_digest_md5(std::string user, std::string password) ;

    void react(Observable *obs) ;

  public:

    XmppConnection(int debugLevel=0, std::string logfile="") ;
    ~XmppConnection(void) ;

    void log(std::string info, std::string tag="INFO") ;

    // connect + authenticate + bindResource + startSession
    bool connect(std::string xmpp_uri) ;

    bool connect(std::string host, bool tls_secured, int port=5222) ;
    bool authenticate(std::string user, std::string password) ;
    bool bindResource(std::string resource) ;
    bool startSession(void) ;

    bool disconnect(void) ;

    bool isConnected(void) { return (connection!=0) ; }
    bool isSecured(void) ;

    std::string getURI(void) { return uri.asString() ; }
    std::string getServer(void) { return uri.host ; }
    std::string getJID(bool full=true) ;

    // -----------------------------------------------------------

    bool sendXML(const char *xmldata, unsigned int size) ;
    bool sendXML(const char *xmldata) ;
    bool sendXML(const std::string xmldata) ;

    // -----------------------------------------------------------

    // Extra args are (key,val) attribute pairs ending with 0. A
    // negative number of milliseconds will block:
    //     waitFor(-1, "stream:features", 0)
    //     waitFor(1000, "proceed", "xmlns","urn:ietf:params:xml:ns:xmpp-tls", 0)
    //     waitFor(-1, "", "id","uniq123", 0)
    XmlParser::InBox::iterator waitFor(long milliseconds, cistring tagname, ...) ;

    XmlParser::InBox::iterator waitFor(long milliseconds) { return waitFor(milliseconds,"",0) ; }
    XmlParser::InBox::iterator wait(void) { return waitFor(-1,"",0) ; }

    bool isWaiting(void) { return (waiting>0) ; }

    // -----------------------------------------------------------

    XmlParser::InBox::iterator clearBox(void) ;
    XmlParser::InBox::iterator clearBox(XmlParser::InBox::iterator &it) ;
    XmlParser::InBox::iterator clearBox(XmlParser::InBox::iterator &first, 
								XmlParser::InBox::iterator &last) ;

    XmlStructure *popBox(void) ;

    // -----------------------------------------------------------

    bool registerUser(std::string username, std::string password) ;

    XmlParser::InBox::iterator discover(std::string entity, std::string what="info") ;

    typedef enum {PRES_UNAVAILABLE, PRES_XA, PRES_AWAY, PRES_DND, PRES_ONLINE, PRES_CHAT} presenceType ;
    bool sendPresence(std::string jid="", std::string status="", presenceType p=PRES_ONLINE) ;

    bool sendSubscriptionRequestTo(std::string jid) ;

    typedef enum {MSG_NORMAL, MSG_HEADLINE, MSG_CHAT, MSG_GROUPCHAT, MSG_ERROR} messageType ;
    // body is used for the <error> tag of pERROR messages
    bool sendMessage(std::string jid, std::string body, messageType type=MSG_CHAT,
				 std::string subject="", std::string thread="") ;

  } ;

}

#endif
