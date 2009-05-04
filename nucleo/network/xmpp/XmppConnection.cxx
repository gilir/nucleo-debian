/*
 *
 * nucleo/network/xmpp/XmppConnection.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/network/xmpp/XmppConnection.H>

#include <nucleo/core/ReactiveEngine.H>
#include <nucleo/core/TimeKeeper.H>
#include <nucleo/core/UUID.H>
#include <nucleo/core/TimeStamp.H>
#include <nucleo/utils/TimeUtils.H>
#include <nucleo/utils/Base64.H>
#include <nucleo/utils/MD5.H>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

namespace nucleo {

  static const char *css_stylesheet =
    "body { font-family: Arial; font-size: 10px; }\n"
    "XMPPLog { display: block; padding: 4; margin: 2; }\n"
    "XMPPLog.SEND { margin-right: 40%; background: #DDDDDD; }\n"
    "XMPPLog.RECV { margin-left: 40%; }\n"
    "XMPPLog.INFO { background: #555588; color: #FFFFFF; }\n"
    "XMPPLog.DBG { background: #666666; color: #FFFFFF; }\n" ;

  // -----------------------------------------------------------------------------------------

  static std::string
  logFormat(const char *data, unsigned int size) {
#if 1
    std::stringstream result ;
    for (size_t i=0; i<size; ++i) {
	 char c = data[i] ;
	 switch (c) {
	 case '\r': break ;
	 case '\n': break ;
	 case '<': if (i>0 && data[i-1]!='>') result << "<br>" ; result << "&lt;" ; break ;
	 case '>': result << "&gt;<br>" ; break ;
	 default: result << c ; break ;
	 }
    }
    return result.str() ;
#else
    int textindent=0, delta=5 ;
    std::stringstream result ;
    for (size_t i=0; i<size; ++i) {
	 char c = data[i] ;
	 switch (c) {
	 case '\r': break ;
	 case '\n': break ;
	 case '<': 
	   if (i<size-1 && data[i+1]=='/') textindent -= delta ;
	   result << "<div style=\"text-indent:"  << textindent << "px\">" << "&lt;" ; 
	   break ;
	 case '>': 
	   result << "&gt;</div>" ; 
	   if (i>0 && data[i-1]!='/') textindent += delta ;
	   break ;
	 default: result << c ; break ;
	 }
    }
    return result.str() ;
#endif
  }

  // -----------------------------------------------------------------------------------------

  XmppConnection::XmppConnection(int dlevel, std::string logfile) : parser(&inbox) {
    debugLevel = dlevel ;
    logstream = 0 ;
    connection = 0 ;
    features = 0 ;
    waiting = 0 ;
#if NUCLEO_SUPPORTS_GNUTLS
    tls_session = 0 ;
    tls_creds = 0 ;
#endif

    if (!logfile.empty()) {
	 logstream = new std::ofstream(logfile.c_str()) ;
	 *logstream
	   << "<html>\n"
	   << "<head>\n"
	   << "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'/>\n"
	   << "<style type='text/css'>\n<!--\n" << css_stylesheet << "\n-->\n</style>\n"
	   << "</head>\n"
	   << "<body>\n" ;
    }
  }

  // -----------------------------------------------------------------------------------------

  void
  XmppConnection::log(std::string info, std::string tag) {
    if (debugLevel>0)
	 std::cerr << "XmppConnection::log " << info << std::endl ;

    if (!logstream) return ;

    TimeStamp stamp ;
    info = stamp.getAsString() + ": " + info ;
    *logstream
	 << std::endl
	 << "<XMPPLog class='" << tag << "' timestamp='" << stamp.getAsInt() << "'>" << std::endl
	 << logFormat(info.data(), info.size()) << std::endl 
	 << "</XMPPLog>" << std::endl ;
  }

  // -----------------------------------------------------------------------------------------

  bool
  XmppConnection::connect(std::string xmpp_uri) {
    if (connection) return false ;

    URI tmp(xmpp_uri) ; // FIXME: check RFC4622

    URI logged(tmp) ; logged.password = "******" ;
    log("Connecting to "+logged.asString(),"DBG") ;

    if (!connect(tmp.host, (tmp.scheme=="xmpp-tls"), tmp.port?tmp.port:5222))
	 return false ;

    if (!authenticate(tmp.user, tmp.password))
	 return false ;

    std::string resource ;
    if (!tmp.path.empty() && tmp.path.length()>1)
	 resource.assign(tmp.path, 1, tmp.path.length()-1) ;
    if (!bindResource(resource))
	 return false ;

    if (!startSession()) return false ;

    return true ;
  }

  // -----------------------------------------------------------------------------------------

  bool
  XmppConnection::connect(std::string host, bool tls_secured, int port) {
    if (connection) return false ;

    uri.clear() ;
    uri.scheme = tls_secured ? "xmpp-tls" : "xmpp" ;
    uri.host = host ;
    if (port!=5222) uri.port = port ;

    try {
	 connection = new TcpConnection(host, port) ;
    } catch (...) {
	 return false ;
    }

    if (logstream) {
	 host = connection->machineLookUp(&port) ;
	 log("Connected to "+host,"DBG") ;
    }

    subscribeTo(connection) ;

    if (!newStream()) return false ;

    if (tls_secured
	   && features->find("starttls","xmlns","urn:ietf:params:xml:ns:xmpp-tls",0)) {
	 log("TLS","DBG") ;
	 if (!tls_init()) return false ;
	 if (!newStream()) return false ;
    }

    clearBox() ;
    return true ;
  }

  // -----------------------------------------------------------------------------------------

  unsigned int
  XmppConnection::pushBytes(const char *data, unsigned int length) {
    unsigned int result=0 ;
#if NUCLEO_SUPPORTS_GNUTLS
    if (tls_session) {
	 char *ptr = (char *)data ;
	 unsigned int remaining = length ;
	 do {
	   int size = gnutls_record_send(tls_session, ptr, remaining) ;
	   if (size<0 && size!=GNUTLS_E_INTERRUPTED && size!=GNUTLS_E_AGAIN)
		break ;
	   remaining -= size ;
	   ptr += size ;
	 } while(remaining) ;
	 result = length-remaining ;
    } else
#endif
	 try {
	   result = connection->send(data, length, true) ;
	 } catch (...) {
	   log("pushBytes failed","DBG") ;
	   disconnect() ;
	   return 0 ;
	 }

    if (debugLevel>2)
	 std::cerr << "XmppConnection::pushBytes: pushed " << result << " bytes" << std::endl ;
    return result ;
  }

  void
  XmppConnection::pullBytes(void) {
    if (! (connection->getState()&FileKeeper::R)) {
	 // nothing to read
	 return ;
    }

    char buffer[4096] ;
    int size = 0 ;
#if NUCLEO_SUPPORTS_GNUTLS
    if (tls_session) {
	 do {
	   size = gnutls_record_recv(tls_session, buffer, sizeof(buffer)-1) ;
	 } while (size==GNUTLS_E_AGAIN) ;
    } else
#endif
	 try {
	   size = connection->receive(buffer, sizeof(buffer), false) ;
	 } catch (...) { 
	   log("pullBytes failed","DBG") ;
	   disconnect() ;
	   return ;
	 }

    if (size<=0) {
	 disconnect() ;
	 return ;
    }

    if (debugLevel>2)
	 std::cerr << "XmppConnection::pullBytes: pulled " << size << " bytes" << std::endl ;
    if (logstream)
	 *logstream
	   << std::endl
	   << "<XMPPLog class='RECV' timestamp='" << TimeStamp::createAsInt() << "'>" << std::endl
	   << logFormat(buffer, size) << std::endl 
	   << "</XMPPLog>" << std::endl ;
    while (parser.parse(buffer, size)==XmlParser::DONE) {
	 parser.reset() ;
	 size = 0 ;
    }
  }

  // -----------------------------------------------------------------------------------------

  bool
  XmppConnection::newStream(void) {
    clearBox() ;
    std::string header = 
	 "<?xml version='1.0'?>\n"
	 "<stream:stream to='" + uri.host + "' xmlns='jabber:client'"
	 " xmlns:stream='http://etherx.jabber.org/streams' version='1.0'>" ;
    sendXML(header) ;
    XmlParser::InBox::iterator iFeatures = waitFor(30000, "stream:features", 0) ;
    if (iFeatures==inbox.end()) {
	 std::cerr << "XmppConnection::connect: no stream:features" << std::endl ;
	 return false ;
    }

    delete(features) ;
    features = *iFeatures ;
    features->detach() ;
    inbox.erase(iFeatures) ;
    return true ;
  }

  // -----------------------------------------------------------------------------------------

  bool
  XmppConnection::tls_init(void) {
#if NUCLEO_SUPPORTS_GNUTLS
    sendXML("<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>") ;
    XmlParser::InBox::iterator r = waitFor(5000, "", 
								   "xmlns","urn:ietf:params:xml:ns:xmpp-tls", 0) ;
    if (r==inbox.end() || (*r)->name!="proceed") {
	 std::cerr << "XmppConnection::tls_init: no 'TLS proceed' from server" << std::endl ;
	 disconnect() ;
	 return false ;
    }

    const int protocol_priority[] = { GNUTLS_TLS1, GNUTLS_SSL3, 0 } ;
    const int kx_priority[] = { GNUTLS_KX_RSA, 0 } ;
    const int cipher_priority[] = {
	 GNUTLS_CIPHER_AES_256_CBC, // used by gloox
	 GNUTLS_CIPHER_AES_128_CBC, // used by gloox
	 GNUTLS_CIPHER_3DES_CBC,    // used by iksemel & gloox
	 GNUTLS_CIPHER_ARCFOUR,     // used by iksemel & gloox
	 0
    } ;
    const int comp_priority[] = { GNUTLS_COMP_ZLIB, GNUTLS_COMP_NULL, 0 } ;
    const int mac_priority[] = { GNUTLS_MAC_SHA, GNUTLS_MAC_MD5, 0 } ;

    if (gnutls_global_init()!=0
	   || gnutls_certificate_allocate_credentials(&tls_creds)!=0
	   || gnutls_init(&tls_session, GNUTLS_CLIENT)!=0) {
	 std::cerr << "XmppConnection::tls_init: TLS init failed" << std::endl ;
	 disconnect() ;
	 return false ;
    }

    // gnutls_certificate_set_x509_trust_file(tls_creds, ...) ;
    // gnutls_certificate_set_x509_key_file(tls_creds, ...) ;
    gnutls_protocol_set_priority(tls_session, protocol_priority);
    gnutls_cipher_set_priority(tls_session, cipher_priority);
    gnutls_compression_set_priority(tls_session, comp_priority);
    gnutls_kx_set_priority(tls_session, kx_priority);
    gnutls_mac_set_priority(tls_session, mac_priority);
    gnutls_credentials_set(tls_session, GNUTLS_CRD_CERTIFICATE, tls_creds);

    gnutls_transport_set_ptr(tls_session, (void*)connection->getFd()) ;

    if (!gnutls_handshake(tls_session)) return true ;

    std::cerr << "XmppConnection::tls_init: TLS handshake failed" << std::endl ;
    disconnect() ;
#else
    std::cerr << "XmppConnection::tls_init: TLS not supported" << std::endl ;
    disconnect() ;
#endif
    return false ;
  }

  void
  XmppConnection::tls_deinit(void) {
#if NUCLEO_SUPPORTS_GNUTLS
    if (tls_session) {
	 gnutls_bye(tls_session, GNUTLS_SHUT_WR) ;
	 gnutls_deinit(tls_session) ;
    }
    if (tls_creds) gnutls_certificate_free_credentials(tls_creds) ;
    gnutls_global_deinit() ;
    tls_session = 0 ;
    tls_creds = 0 ;
#endif
  }

  bool
  XmppConnection::isSecured(void) {
#if NUCLEO_SUPPORTS_GNUTLS
    return (tls_session!=0) ;
#else
    return false ;
#endif
  }

  // -----------------------------------------------------------------------------------------

  bool
  XmppConnection::authenticate(std::string user, std::string password) {
    if (!connection || !features) return false ;

    bool sasl_init_ok = false ;
    bool tryDIGESTMD5=false, tryPLAIN=false, tryANONYMOUS=false ;

    XmlStructure *mechanisms = features->find("mechanisms",
									 "xmlns","urn:ietf:params:xml:ns:xmpp-sasl",0) ;

    if (!mechanisms || !(mechanisms->children.size())) {
	 std::cerr << "XmppConnection::authenticate: server does not support SASL" << std::endl ;
	 // FIXME: try http://www.xmpp.org/extensions/xep-0078.html 
	 return false ;
    } else {
	 for (std::list<XmlStructure*>::iterator m=mechanisms->children.begin();
		 m!=mechanisms->children.end(); ++m) {
	   XmlStructure *mechanism = *m ;
	   if (mechanism->name!="mechanism") continue ;
	   if (!user.empty()) {
		if (mechanism->cdata=="DIGEST-MD5") tryDIGESTMD5 = true ;
		else if (mechanism->cdata=="PLAIN") tryPLAIN = true ;
	   } else 
		if (mechanism->cdata=="ANONYMOUS") tryANONYMOUS = true ;
	 }
    }

    if (connection && !sasl_init_ok && tryDIGESTMD5) {
	 log("SASL authentication (DIGEST-MD5)","DBG") ;
	 sasl_init_ok = authenticate_digest_md5(user, password) ;    
    }

    if (connection && !sasl_init_ok && tryPLAIN) {
	 log("SASL authentication (PLAIN)","DBG") ;
	 sasl_init_ok = authenticate_plain(user, password) ;
    }

    if (connection && !sasl_init_ok && tryANONYMOUS) {
	 log("SASL authentication (ANONYMOUS)","DBG") ;
	 sasl_init_ok = authenticate_anonymous() ;
    }

    if (!connection || !sasl_init_ok) return false ;

    uri.user = user ;
    uri.password = password ;
    if (!newStream()) return false ;
    clearBox() ;
    return true ;
  }

  bool
  XmppConnection::authenticate_anonymous(void) {
    sendXML("<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='ANONYMOUS'/>") ;

    XmlParser::InBox::iterator r = waitFor(5000, "", 
								   "xmlns","urn:ietf:params:xml:ns:xmpp-sasl", 0) ;
    if (r!=inbox.end() && (*r)->name=="success") return true ;
    std::cerr << "XmppConnection::authenticate_anonymous: SASL authentication failed" << std::endl ;
    return false ;
  }

  bool
  XmppConnection::authenticate_plain(std::string user, std::string password) {
    std::string auth = "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='PLAIN'>"
	 + Base64::encode('\0'+user+'\0'+password)
	 + "</auth>" ;
    sendXML(auth) ;

    XmlParser::InBox::iterator r = waitFor(5000, "", 
								   "xmlns","urn:ietf:params:xml:ns:xmpp-sasl", 0) ;
    if (r!=inbox.end() && (*r)->name=="success") return true ;
    std::cerr << "XmppConnection::authenticate_plain: SASL authentication failed" << std::endl ;
    return false ;
  }

  bool
  XmppConnection::authenticate_digest_md5(std::string user, std::string password) {
    sendXML("<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='DIGEST-MD5'/>") ;
    XmlParser::InBox::iterator c = waitFor(5000, "challenge",
								   "xmlns","urn:ietf:params:xml:ns:xmpp-sasl",0) ;
    if (c==inbox.end()) {
	 std::cerr << "XmppConnection::authenticate_digest_md5: no MD5 challenge" << std::endl ;
	 return false ;
    }

    std::string challenge = Base64::decode((*c)->cdata) ;
    delete *c ;
    inbox.erase(c) ;

    std::map<std::string,std::string> cdict ;
    while (true) {
	 std::string::size_type p1 = challenge.find(",") ;
	 std::string value(challenge,0,p1) ;
	 challenge.erase(0,p1+1) ;
	 std::string::size_type p2 = value.find("=") ;
	 if (p2!=std::string::npos) {
	   std::string key(value,0,p2) ;
	   value.erase(0,p2+1) ;
	   trimString(value, "\"") ;
	   cdict[key] = value ;
	 }
	 if (challenge.size()==0) break ;
	 if (p1==std::string::npos) break ;
    }

#if 0
    std::cerr << "----------" << std::endl ;
    for (std::map<std::string,std::string>::iterator i=cdict.begin();
	    i!=cdict.end(); ++i)
	 std::cerr << (*i).first << ": " << (*i).second << std::endl ;
    std::cerr << "----------" << std::endl ;
#endif

    std::string cnonce = UUID::createAsString() ;
    std::string nc_value = "00000001" ;
    std::string digest_uri = "xmpp/"+uri.host ;

    MD5 md5 ;
    MD5::bytearray md5hash ;
    md5.eat(user+":"+cdict["realm"]+":"+password) ;
    md5.digest(md5hash) ;

    md5.clear() ;
    md5.eat(md5hash,16) ;
    md5.eat(":"+cdict["nonce"]+":"+cnonce) ;
    std::string hex_h_a1 = md5.digest() ;

    md5.clear() ;
    md5.eat("AUTHENTICATE:"+digest_uri) ;
    std::string hex_h_a2 = md5.digest() ;
    md5.clear() ;
    md5.eat(hex_h_a1+":"+cdict["nonce"]+":"+nc_value+":"+cnonce+":"+cdict["qop"]+":"+hex_h_a2) ;
    std::string response_value = md5.digest() ;

    std::stringstream digest_response ;
    digest_response << "username=\"" << user << "\","
				<< "realm=\"" << cdict["realm"] << "\","
				<< "nonce=\"" << cdict["nonce"] << "\","
				<< "cnonce=\"" << cnonce << "\","
				<< "nc=" << nc_value << ",qop=" << cdict["qop"] << ","
				<< "digest-uri=\"" << digest_uri << "\","
				<< "response=" << response_value << ","
				<< "charset=utf-8" ;
    std::string tmp = digest_response.str() ;

    std::string xml = "<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"+Base64::encode(tmp)+"</response>" ;
    sendXML(xml) ;

    XmlParser::InBox::iterator r = waitFor(-1, "", 
								   "xmlns","urn:ietf:params:xml:ns:xmpp-sasl", 0) ;
    if (r!=inbox.end() && (*r)->name=="success") return true ;
    std::cerr << "XmppConnection::authenticate_digest_md5: SASL authentication failed" << std::endl ;
    return false ;
  }

  // -----------------------------------------------------------------------------------------

  bool
  XmppConnection::bindResource(std::string resource) {
    if (!connection 
	   || !features
	   || !features->find("bind","xmlns","urn:ietf:params:xml:ns:xmpp-bind",0))
	 return false ;

    log("Resource binding","DBG") ;

    std::string id=UUID::createAsString() ;
    std::string request = 
	 "<iq type='set' id='" + id + "'>"
	 "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'" ;
    if (!resource.empty()) 
	 request = request + "><resource>" + resource + "</resource></bind></iq>" ;
    else
	 request = request + "/></iq>" ;
    sendXML(request) ;
  
    XmlParser::InBox::iterator r = waitFor(5000, "iq", "id",id.c_str(), 0) ;
    if (r==inbox.end() || (*r)->getAttr("type")!="result") {
	 std::cerr << "XmppConnection::bindResource: failed to bind resource '" << resource << "'" << std::endl ;
	 return false ;
    }
  
    XmlStructure *jidnode = (*r)->walk(1,"bind",1,"jid",0) ;
    if (!jidnode || jidnode->cdata.empty()) return false ;

    URI jid("xmpp://"+jidnode->cdata) ;
    if (debugLevel>0 && uri.user!=jid.user) 
	 std::cerr << "XmppConnection::bindResource: '" << uri.user << "' != '" << jid.user << "'" << std::endl ;
    if (debugLevel>0 && uri.host!=jid.host)
	 std::cerr << "XmppConnection::bindResource: '" << uri.host << "' != '" << jid.host << "'" << std::endl ;
    uri.user = jid.user ;
    uri.host = jid.host ;
    uri.path = jid.path ;

    clearBox(r) ;
    return true ;
  }

  // -----------------------------------------------------------------------------------------

  bool
  XmppConnection::startSession(void) {
    if (!connection
	   || !features
	   || !features->find("session","xmlns","urn:ietf:params:xml:ns:xmpp-session",0))
	 return false ;

    log("Session establishment","DBG") ;

    std::string id=UUID::createAsString(), resource ;
    std::string request = 
	 "<iq type='set' id='" + id + "'>"
	 "<session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>"
	 "</iq>" ;
    sendXML(request) ;

    XmlParser::InBox::iterator r = waitFor(5000, "iq", "id",id.c_str(), 0) ;
    if (r==inbox.end() || (*r)->getAttr("type")!="result") {    
	 std::cerr << "XmppConnection::connect: failed to initiate session" << std::endl ;
	 return false ;
    }

    clearBox(r) ;
    return true ;
  }

  // -----------------------------------------------------------------------------------------

  bool
  XmppConnection::sendXML(const char *xmldata, unsigned int size) {
    if (!connection) return false ;
    if (logstream)
	 *logstream
	   << std::endl
	   << "<XMPPLog class='SEND' timestamp='" << TimeStamp::createAsInt() << "'>" << std::endl
	   << logFormat(xmldata, size) << std::endl 
	   << "</XMPPLog>" << std::endl ;
    return (size==pushBytes(xmldata, size)) ;
  }

  bool
  XmppConnection::sendXML(const char *xmldata) {
    return sendXML(xmldata, strlen(xmldata)) ;
  }

  bool
  XmppConnection::sendXML(const std::string xmldata) {
    return sendXML(xmldata.data(), xmldata.length()) ;
  }

  // -----------------------------------------------------------------------------------------

  XmlParser::InBox::iterator
  XmppConnection::waitFor(long milliseconds, cistring tagname, ...) {
    if (!connection) return inbox.end() ;

    waiting++ ; // Stop notifying observers right now

    std::list<XmlStructure::KeyValuePair> attribs ;
    va_list ap ;
    va_start(ap,tagname) ;
    for (char *key=va_arg(ap, char*); key; key=va_arg(ap, char*))
	 attribs.push_back(XmlStructure::KeyValuePair(key, va_arg(ap, char*))) ;
    va_end(ap) ;

    if (debugLevel>1) {
	 std::cerr << "XmppConnection::waitFor: " ;
	 if (milliseconds>-1) std::cerr << "t<" << milliseconds << " " ;
	 std::cerr << "'" << tagname.c_str() << "'" ;
	 for (std::list<XmlStructure::KeyValuePair>::iterator kv=attribs.begin(); kv!=attribs.end(); ++kv)
	   std::cerr << " " << (*kv).first << "='" << (*kv).second << "'" ;
	 std::cerr << std::endl ;
    }

    // unsigned long epoch = TimeStamp::createAsInt() ;
    XmlParser::InBox::iterator result = inbox.begin() ;
    bool found = false ;
    do {
	 for (; result!=inbox.end(); result++) {
	   XmlStructure *node = *result ;
	   cistring nodename = node->name.c_str() ;
	   found = (tagname.empty() || nodename==tagname) ;
	   if (found)
		for (std::list<XmlStructure::KeyValuePair>::iterator kv=attribs.begin(); kv!=attribs.end(); ++kv)
		  found = found && (node->getAttr((*kv).first)==(*kv).second) ;
	   if (found) break ;
	 }
	 if (!found) {
#if 0
	   if (milliseconds>-1) {
		long elapsed = TimeStamp::createAsInt()-epoch ;
		if (elapsed>milliseconds) break ;
		ReactiveEngine::step(milliseconds-elapsed) ;
	   } else {
		ReactiveEngine::step() ;
	   }
#else
	   pullBytes() ;
#endif
	 }
    } while (!found && connection) ;

    waiting-- ;
    if (!waiting) notifyObservers() ;

    return (found ? result : inbox.end()) ;
  }

  XmlParser::InBox::iterator
  XmppConnection::clearBox(void) {
    for (XmlParser::InBox::iterator i=inbox.begin(); i!=inbox.end(); ++i) delete *i ;
    inbox.clear() ;
    return inbox.end() ;
  }

  XmlParser::InBox::iterator
  XmppConnection::clearBox(XmlParser::InBox::iterator &it) {
    delete *it ;
    return inbox.erase(it) ;
  }

  XmlParser::InBox::iterator
  XmppConnection::clearBox(XmlParser::InBox::iterator &first, XmlParser::InBox::iterator &last) {
    for (XmlParser::InBox::iterator i=first; i!=last; ++i) delete *i ;
    return inbox.erase(first, last) ;
  }

  XmlStructure *
  XmppConnection::popBox(void) {
    if (inbox.size()==0) return 0 ;
    XmlStructure *node = inbox.front() ;
    inbox.pop_front() ;
    return node ;
  }

  // -----------------------------------------------------------------------------------------

  std::string
  XmppConnection::getJID(bool full) {
    std::string result = uri.user ;
    if (!uri.host.empty()) result = result + "@" + uri.host ;
    if (full & !uri.path.empty()) result = result + uri.path ;
    return result ;
  }

  // -----------------------------------------------------------------------------------------

  void
  XmppConnection::react(Observable *obs) {
    if (connection && obs==connection) {
	 pullBytes() ;    
	 if (!waiting) notifyObservers() ;
    }
  }

  // -----------------------------------------------------------------------------------------

  bool
  XmppConnection::registerUser(std::string username, std::string password) {
    if (!connection) return false ;

    std::string uuid = UUID::createAsString() ;
    std::string request =
	 "<iq type='set' id='" + uuid + "'>"
	 "<query xmlns='jabber:iq:register'>"
	 "<username>" + username + "</username>"
	 "<password>" + password + "</password>"
	 "</query></iq>" ;
    sendXML(request) ;

    XmlParser::InBox::iterator i = waitFor(-1,"iq","id", uuid.c_str(),0) ;
    return (i!=inbox.end() && (*i)->getAttr("type")=="result") ;
  }

  XmlParser::InBox::iterator
  XmppConnection::discover(std::string entity, std::string what) {
    if (!connection) return inbox.end() ;

    std::string id = UUID::createAsString() ;
    std::string r = 
	 "<iq id='" + id + "' to='" + entity + "' type='get'>"
	 "<query xmlns='http://jabber.org/protocol/disco#" + what + "'/>"
	 "</iq>" ;
    sendXML(r) ;
    return waitFor(5000, "iq", "id",id.c_str(), 0) ;
  }

  bool
  XmppConnection::sendSubscriptionRequestTo(std::string jid) {
    if (!connection || jid.empty()) return false ;
    std::string msg = "<presence to='" + jid + "' type='subscribe' />" ;
    sendXML(msg) ;
    return true ;
  }

  bool
  XmppConnection::sendPresence(std::string jid, std::string status, presenceType p) {
    if (!connection) return false ;

    std::string msg = "<presence" ;
    if (!jid.empty()) msg = msg + " to='" + jid + "'" ;
    if (p==PRES_UNAVAILABLE) msg = msg + " type='unavailable'" ;
    msg = msg + ">" ;
    char *show[] = {"","xa","away","dnd","","chat"} ;
    if (p!=PRES_ONLINE && p!=PRES_UNAVAILABLE) msg = msg + "<show>" + show[p] + "</show>" ;
    if (!status.empty()) msg = msg + "<status>" + status + "</status>" ;
    msg = msg + "</presence>" ;
    sendXML(msg) ;
    return true ;
  }

  bool
  XmppConnection::sendMessage(std::string jid, std::string body, messageType type,
						std::string subject, std::string thread) {
    if (!connection) return false ;

    std::string id = UUID::createAsString() ;
    std::string r = "<message" ;
    if (!jid.empty()) r = r + " to='" + jid + "'" ;
    const char *typeName[] = {"normal", "headline", "chat", "groupchat", "error"} ;
    r = r + " type='" + typeName[type] + "' id='" + id + "'>" ;
    if (!thread.empty()) r = r + "<thread>" + thread + "</thread>" ;
    if (!subject.empty()) r = r + "<subject>" + subject + "</subject>" ;
    if (!body.empty()) 
	 if (type==MSG_ERROR)
	   r = r + "<error>" + body + "</error>" ;
	 else
	   r = r + "<body>" + body + "</body>" ;
    r = r + "</message>" ;
    sendXML(r) ;
    return true ;
  }

  // -----------------------------------------------------------------------------------------

  bool
  XmppConnection::disconnect(void) {
    if (!connection) return false ;
    log("Disconnecting","DBG") ;
    sendXML("</stream:stream>") ;
    unsubscribeFrom(connection) ;
    delete features ;
    features = 0 ;
#if NUCLEO_SUPPORTS_GNUTLS
    if (tls_session) tls_deinit() ;
#endif
    delete connection ;
    connection = 0 ;
    return true ;
  }

  // -----------------------------------------------------------------------------------------

  XmppConnection::~XmppConnection(void) {
    disconnect() ;
    delete logstream ;
  }

  // -----------------------------------------------------------------------------------------

}
