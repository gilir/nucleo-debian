/*
 *
 * nucleo/image/source/nudppImageSource.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/image/source/nudppImageSource.H>
#include <nucleo/utils/FileUtils.H>
#include <nucleo/image/encoding/Conversion.H>

#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdexcept>
#include <iostream>
#include <cstring>

namespace nucleo {

#define DEBUG_LEVEL 0

  nudppImageSource::nudppImageSource(const URI &uri, Image::Encoding t_encoding) {
    _hostOrGroup = uri.host ; 
    _realport = _port = uri.port ;

    std::string query = uri.query ;

    std::string encoding ;
    source_encoding = Image::JPEG ;    
    if (URI::getQueryArg(query, "encoding", &encoding))
	 source_encoding = Image::getEncodingByName(encoding) ;
    
    target_encoding = t_encoding==Image::PREFERRED ? source_encoding : t_encoding ;

    unsigned char *tmp = Image::AllocMem(nudppImageSink::MaxFragmentSize) ;
    lastImage.setData(tmp, nudppImageSink::MaxFragmentSize, Image::FREEMEM) ;
    _state = CLOSED ;
    _skeeper = 0 ;
  }

  bool
  nudppImageSource::start(void) {
    if (_state!=CLOSED) return false ;

    _socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_socket<0) throw std::runtime_error("nudppImageSource: can't create socket") ;

    struct sockaddr_in name ;  
    memset(&name, 0, sizeof(name)) ;
    name.sin_family = AF_INET ;
    name.sin_addr.s_addr = htonl(INADDR_ANY) ;
    name.sin_port = htons(_port) ;

    if (_hostOrGroup!="" && _hostOrGroup!="localhost" && _hostOrGroup!="127.0.0.1") {
#if DEBUG_LEVEL>0
	 std::cerr << "This is a multicast source" << std::endl ;
#endif

	 int one=1 ;
#ifdef SO_REUSEPORT
	 setsockopt(_socket, SOL_SOCKET, SO_REUSEPORT, (char *)&one, sizeof(int)) ;
#endif
	 setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(int)) ;

	 struct ip_mreq mreq ;
	 mreq.imr_multiaddr.s_addr = inet_addr(_hostOrGroup.c_str()) ;
	 mreq.imr_interface.s_addr = htonl(INADDR_ANY) ;
	 if (setsockopt(_socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0)
	   throw std::runtime_error("nudppImageSource: can't set multicast group membership") ;
    }
  
    socklen_t lenmyaddr = sizeof(sockaddr_in);
    if (bind(_socket, (struct sockaddr *)&name, lenmyaddr) < 0)
	 throw std::runtime_error("nudppImageSource: bind failed") ;

    int rcvbuf ;
    for (unsigned int i=30; i>0; --i) {
	 rcvbuf = 1 << i ;
	 int res = setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, sizeof(rcvbuf)) ;
	 if (res!=-1) break ;
    }

#if DEBUG_LEVEL>0
    std::cerr << "SO_RCVBUF=" << rcvbuf << std::endl ;
    std::cerr << "MaxFragmentSize=" << nudppImageSink::MaxFragmentSize << std::endl ;
#endif

    if (getsockname(_socket, (sockaddr *)&name, &lenmyaddr)!=-1)
	 _realport = name.sin_port ; 

    _state = OUT_OF_SYNC ;

    _skeeper = FileKeeper::create(_socket, FileKeeper::R) ;
    subscribeTo(_skeeper) ;

    frameCount = 0 ; previousImageTime = TimeStamp::undef ; sampler.start() ;
    return true ;
  }

  void
  nudppImageSource::react(Observable *obs) {
    if (obs==_skeeper && _skeeper->getState()&FileKeeper::R)
	 _receiveFragment() ;
  }

  void
  nudppImageSource::_receiveFragment(void) {
    if (_state!=SYNC) _bytesRead=0 ;

    struct msghdr msg ;
    memset(&msg, 0, sizeof (msg)) ;
    struct iovec iov[2] ;
    iov[0].iov_base = (char *)&_info ;
    iov[0].iov_len = sizeof(_info) ;
    iov[1].iov_base = (char *)(lastImage.getData()+_bytesRead) ; 
    iov[1].iov_len = nudppImageSink::MaxFragmentSize ;
    msg.msg_iov = iov ;
    msg.msg_iovlen = 2 ;
  
    ssize_t s = recvmsg(_socket, &msg, 0) ;
    if (s==-1) {
	 perror("nudppImageSource") ;
	 stop() ;
	 return ;
    }
    unsigned int fragmentSize = s-sizeof(_info) ;

    _info.inum = ntohs(_info.inum) ;
    _info.fnum = ntohs(_info.fnum) ;
    _info.encoding = ntohl(_info.encoding) ;
    _info.width = ntohs(_info.width) ;
    _info.height = ntohs(_info.height) ;
    _info.totalsize = ntohl(_info.totalsize) ;

    if (!_info.fnum) {
	 if (_state==SYNC && _inum==_info.inum) {
#if DEBUG_LEVEL>0
	   std::cerr << "First fragment of image #" << _info.inum << " was received twice" << std::endl ;
#endif
	   return ;
	 }

	 _inum = _info.inum ;
	 _fnum = 0 ;

	 if (lastImage.getSize()<_info.totalsize) {
	   // Buffer too small, create a new one
	   unsigned int l = nudppImageSink::MaxFragmentSize + _info.totalsize ;
	   unsigned char *tmp = Image::AllocMem(l) ;
	   memmove(tmp, iov[1].iov_base, fragmentSize) ;
	   lastImage.setData(tmp, _info.totalsize, Image::FREEMEM) ;
	 } else if (_state==SYNC) {
	   // We were already synced on another image so the fragment went
	   // into the wrong place. Move it to the beginning of the buffer.
	   memmove(lastImage.getData(), iov[1].iov_base, fragmentSize) ;
	   // FIXME: potential aliasing problem
	 }

	 lastImage.setEncoding((Image::Encoding)(_info.encoding)) ;
	 lastImage.setDims(_info.width, _info.height) ;   

	 _state = SYNC ;
	 _bytesRead = fragmentSize ;
	 if (_bytesRead==_info.totalsize) {
	   lastImage.setTimeStamp() ;
	   frameCount++ ; sampler.tick() ;
	   if (!_pendingNotifications) notifyObservers() ;
	 }
#if DEBUG_LEVEL>1
	 std::cerr << _info.inum << "/" << _info.fnum << ": " << _bytesRead << "/" << lastImage.getSize() << " " << _info.totalsize << std::endl ;
#endif

	 return ;
    }

    // If we're out of sync, just ignore this fragment
    if (_state!=SYNC) {
#if DEBUG_LEVEL>0
	 std::cerr << "Out of sync: " << _info.inum << "/" << _info.fnum << " (" << _inum << "/" << _fnum << ")" << std::endl ;
#endif
	 return ;
    }
  
    // If this fragment is not the one we were waiting for, ignore it
    // and return to out-of-sync state
    if (_info.inum!=_inum || _info.fnum!=(_fnum+1)) {
#if DEBUG_LEVEL>0
	 std::cerr << "Ignoring " << _info.inum << "/" << _info.fnum << " (" << _inum << "/" << _fnum << ")" << std::endl ;
#endif
	 return ;
    }

    // We got our fragment!
    _fnum++ ;
    _bytesRead += fragmentSize ;
    if (_bytesRead==_info.totalsize) {
	 lastImage.setTimeStamp() ;
	 frameCount++ ; sampler.tick() ;
	 if (!_pendingNotifications) notifyObservers() ;
    }
#if DEBUG_LEVEL>1
    std::cerr << _info.inum << "/" << _info.fnum << ": " << _bytesRead << "/" << lastImage.getSize() << " " << _info.totalsize << std::endl ;
#endif
  }

  bool
  nudppImageSource::getNextImage(Image *img, TimeStamp::inttype reftime) {
    if (_state==CLOSED || !frameCount || reftime>=lastImage.getTimeStamp()) return false ;
    _state = OUT_OF_SYNC ;
    img->linkDataFrom(lastImage) ;
    previousImageTime = lastImage.getTimeStamp() ;
    return convertImage(img, target_encoding) ;
  }

  bool
  nudppImageSource::stop(void) {
    if (_state==CLOSED) return false ;

    shutdown(_socket,2) ;
    close(_socket) ;
    unsubscribeFrom(_skeeper) ;
    delete _skeeper ;
    _skeeper = 0 ;

    sampler.stop() ;
    _state = CLOSED ;
    return true ;
  }

}
