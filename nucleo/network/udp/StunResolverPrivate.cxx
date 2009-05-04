/*
 *
 * nucleo/network/udp/StunResolverPrivate.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/* ====================================================================
 * The Vovida Software License, Version 1.0 
 * 
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * ====================================================================
 * 
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */

// http://en.wikipedia.org/wiki/Network_address_translation
// http://en.wikipedia.org/wiki/STUN
// http://sourceforge.net/projects/stun/

#include <nucleo/network/udp/StunResolverPrivate.H>

#include <arpa/inet.h>

#include <assert.h>
#include <unistd.h>
#include <cstring>

#include <iostream>
#include <cstdlib>

std::ostream& // unmodified
operator<<( std::ostream& strm, const StunAddress4& addr) {
  uint32_t ip = addr.addr;
  strm << ((int)(ip>>24)&0xFF) << ".";
  strm << ((int)(ip>>16)&0xFF) << ".";
  strm << ((int)(ip>> 8)&0xFF) << ".";
  strm << ((int)(ip>> 0)&0xFF) ;
  strm << ":" << addr.port;
  return strm;
}

static char* // unmodified
encode(char* buf, const char* data, unsigned int length) {
  memcpy(buf, data, length);
  return buf + length;
}

static char* // unmodified
encode16(char* buf, uint16_t data) {
  uint16_t ndata = htons(data);
  memcpy(buf, reinterpret_cast<void*>(&ndata), sizeof(uint16_t));
  return buf + sizeof(uint16_t);
}

static char* // unmodified
encode32(char* buf, uint32_t data) {
  uint32_t ndata = htonl(data);
  memcpy(buf, reinterpret_cast<void*>(&ndata), sizeof(uint32_t));
  return buf + sizeof(uint32_t);
}

static char* // unmodified
encodeAtrAddress4(char* ptr, uint16_t type, const StunAtrAddress4& atr) {
  ptr = encode16(ptr, type);
  ptr = encode16(ptr, 8);
  *ptr++ = atr.pad;
  *ptr++ = IPv4Family;
  ptr = encode16(ptr, atr.ipv4.port);
  ptr = encode32(ptr, atr.ipv4.addr);
  return ptr;
}

static char* // unmodified
encodeAtrChangeRequest(char* ptr, const StunAtrChangeRequest& atr) {
  ptr = encode16(ptr, ChangeRequest);
  ptr = encode16(ptr, 4);
  ptr = encode32(ptr, atr.value);
  return ptr;
}

static char* // unmodified
encodeAtrString(char* ptr, uint16_t type, const StunAtrString& atr) {
  assert(atr.sizeValue % 4 == 0);
  ptr = encode16(ptr, type);
  ptr = encode16(ptr, atr.sizeValue);
  ptr = encode(ptr, atr.value, atr.sizeValue);
  return ptr;
}

static char* // unmodified
encodeAtrError(char* ptr, const StunAtrError& atr) {
  ptr = encode16(ptr, ErrorCode);
  ptr = encode16(ptr, 6 + atr.sizeReason);
  ptr = encode16(ptr, atr.pad);
  *ptr++ = atr.errorClass;
  *ptr++ = atr.number;
  ptr = encode(ptr, atr.reason, atr.sizeReason);
  return ptr;
}

static char* // unmodified
encodeAtrUnknown(char* ptr, const StunAtrUnknown& atr) {
  ptr = encode16(ptr, UnknownAttribute);
  ptr = encode16(ptr, 2+2*atr.numAttributes);
  for (int i=0; i<atr.numAttributes; i++)
    ptr = encode16(ptr, atr.attrType[i]);
  return ptr;
}

static char* // unmodified
encodeXorOnly(char* ptr) {
  ptr = encode16(ptr, XorOnly);
  return ptr;
}

static char* // unmodified
encodeAtrIntegrity(char* ptr, const StunAtrIntegrity& atr) {
  ptr = encode16(ptr, MessageIntegrity);
  ptr = encode16(ptr, 20);
  ptr = encode(ptr, atr.hash, sizeof(atr.hash));
  return ptr;
}

static void // modified
computeHmac(char* hmac, const char* input, int length, const char* key, int sizeKey) {
  strncpy(hmac,"hmac-not-implemented",20);
}

static uint32_t // modified
stunRand(void) {
  static bool init=false ;
  if (!init) {
    init = true ;
    srandom(time(0)*getpid()) ;
  }
  return random(); 
}

void // unmodified
stunBuildReqSimple(StunMessage* msg, const StunAtrString& username,
			    bool changePort, bool changeIp, unsigned int id) {
  assert( msg );
  memset( msg , 0 , sizeof(*msg) );
	
  msg->msgHdr.msgType = BindRequestMsg;
	
  for ( int i=0; i<16; i=i+4 ) {
    assert(i+3<16);
    int r = stunRand();
    msg->msgHdr.id.octet[i+0]= r>>0;
    msg->msgHdr.id.octet[i+1]= r>>8;
    msg->msgHdr.id.octet[i+2]= r>>16;
    msg->msgHdr.id.octet[i+3]= r>>24;
  }
	
  if ( id != 0 ) msg->msgHdr.id.octet[0] = id; 
	
  msg->hasChangeRequest = true;
  msg->changeRequest.value =(changeIp?ChangeIpFlag:0) | (changePort?ChangePortFlag:0);
	
  if ( username.sizeValue > 0 ) {
    msg->hasUsername = true;
    msg->username = username;
  }
}

static bool // unmodified
stunParseAtrChangeRequest( char* body, unsigned int hdrLen,  StunAtrChangeRequest& result )
{
   if ( hdrLen != 4 )
   {
      std::clog << "hdr length = " << hdrLen << " expecting " << sizeof(result) << std::endl;
		
      std::clog << "Incorrect size for ChangeRequest" << std::endl;
      return false;
   }
   else
   {
      memcpy(&result.value, body, 4);
      result.value = ntohl(result.value);
      return true;
   }
}

static bool // unmodified
stunParseAtrError( char* body, unsigned int hdrLen,  StunAtrError& result )
{
   if ( hdrLen >= sizeof(result) )
   {
      std::clog << "head on Error too large" << std::endl;
      return false;
   }
   else
   {
      memcpy(&result.pad, body, 2); body+=2;
      result.pad = ntohs(result.pad);
      result.errorClass = *body++;
      result.number = *body++;
		
      result.sizeReason = hdrLen - 4;
      memcpy(&result.reason, body, result.sizeReason);
      result.reason[result.sizeReason] = 0;
      return true;
   }
}

static bool // unmodified
stunParseAtrUnknown( char* body, unsigned int hdrLen,  StunAtrUnknown& result )
{
   if ( hdrLen >= sizeof(result) )
   {
      return false;
   }
   else
   {
      if (hdrLen % 4 != 0) return false;
      result.numAttributes = hdrLen / 4;
      for (int i=0; i<result.numAttributes; i++)
      {
         memcpy(&result.attrType[i], body, 2); body+=2;
         result.attrType[i] = ntohs(result.attrType[i]);
      }
      return true;
   }
}


static bool // unmodified
stunParseAtrString( char* body, unsigned int hdrLen,  StunAtrString& result )
{
   if ( hdrLen >= STUN_MAX_STRING )
   {
      std::clog << "String is too large" << std::endl;
      return false;
   }
   else
   {
      if (hdrLen % 4 != 0)
      {
         std::clog << "Bad length string " << hdrLen << std::endl;
         return false;
      }
		
      result.sizeValue = hdrLen;
      memcpy(&result.value, body, hdrLen);
      result.value[hdrLen] = 0;
      return true;
   }
}


static bool // unmodified
stunParseAtrIntegrity( char* body, unsigned int hdrLen,  StunAtrIntegrity& result )
{
   if ( hdrLen != 20)
   {
      std::clog << "MessageIntegrity must be 20 bytes" << std::endl;
      return false;
   }
   else
   {
      memcpy(&result.hash, body, hdrLen);
      return true;
   }
}

unsigned int // unmodified
stunEncodeMessage(const StunMessage& msg, 
			   char* buf, unsigned int bufLen, 
			   const StunAtrString& password, 
			   bool verbose) {
   assert(bufLen >= sizeof(StunMsgHdr));
   char* ptr = buf;
	
   ptr = encode16(ptr, msg.msgHdr.msgType);
   char* lengthp = ptr;
   ptr = encode16(ptr, 0);
   ptr = encode(ptr, reinterpret_cast<const char*>(msg.msgHdr.id.octet), sizeof(msg.msgHdr.id));
	
   if (verbose) std::clog << "Encoding stun message: " << std::endl;
   if (msg.hasMappedAddress)
   {
      if (verbose) std::clog << "Encoding MappedAddress: " << msg.mappedAddress.ipv4 << std::endl;
      ptr = encodeAtrAddress4 (ptr, MappedAddress, msg.mappedAddress);
   }
   if (msg.hasResponseAddress)
   {
      if (verbose) std::clog << "Encoding ResponseAddress: " << msg.responseAddress.ipv4 << std::endl;
      ptr = encodeAtrAddress4(ptr, ResponseAddress, msg.responseAddress);
   }
   if (msg.hasChangeRequest)
   {
      if (verbose) std::clog << "Encoding ChangeRequest: " << msg.changeRequest.value << std::endl;
      ptr = encodeAtrChangeRequest(ptr, msg.changeRequest);
   }
   if (msg.hasSourceAddress)
   {
      if (verbose) std::clog << "Encoding SourceAddress: " << msg.sourceAddress.ipv4 << std::endl;
      ptr = encodeAtrAddress4(ptr, SourceAddress, msg.sourceAddress);
   }
   if (msg.hasChangedAddress)
   {
      if (verbose) std::clog << "Encoding ChangedAddress: " << msg.changedAddress.ipv4 << std::endl;
      ptr = encodeAtrAddress4(ptr, ChangedAddress, msg.changedAddress);
   }
   if (msg.hasUsername)
   {
      if (verbose) std::clog << "Encoding Username: " << msg.username.value << std::endl;
      ptr = encodeAtrString(ptr, Username, msg.username);
   }
   if (msg.hasPassword)
   {
      if (verbose) std::clog << "Encoding Password: " << msg.password.value << std::endl;
      ptr = encodeAtrString(ptr, Password, msg.password);
   }
   if (msg.hasErrorCode)
   {
      if (verbose) std::clog << "Encoding ErrorCode: class=" 
			<< int(msg.errorCode.errorClass)  
			<< " number=" << int(msg.errorCode.number) 
			<< " reason=" 
			<< msg.errorCode.reason 
			<< std::endl;
		
      ptr = encodeAtrError(ptr, msg.errorCode);
   }
   if (msg.hasUnknownAttributes)
   {
      if (verbose) std::clog << "Encoding UnknownAttribute: ???" << std::endl;
      ptr = encodeAtrUnknown(ptr, msg.unknownAttributes);
   }
   if (msg.hasReflectedFrom)
   {
      if (verbose) std::clog << "Encoding ReflectedFrom: " << msg.reflectedFrom.ipv4 << std::endl;
      ptr = encodeAtrAddress4(ptr, ReflectedFrom, msg.reflectedFrom);
   }
   if (msg.hasXorMappedAddress)
   {
      if (verbose) std::clog << "Encoding XorMappedAddress: " << msg.xorMappedAddress.ipv4 << std::endl;
      ptr = encodeAtrAddress4 (ptr, XorMappedAddress, msg.xorMappedAddress);
   }
   if (msg.xorOnly)
   {
      if (verbose) std::clog << "Encoding xorOnly: " << std::endl;
      ptr = encodeXorOnly( ptr );
   }
   if (msg.hasServerName)
   {
      if (verbose) std::clog << "Encoding ServerName: " << msg.serverName.value << std::endl;
      ptr = encodeAtrString(ptr, ServerName, msg.serverName);
   }
   if (msg.hasSecondaryAddress)
   {
      if (verbose) std::clog << "Encoding SecondaryAddress: " << msg.secondaryAddress.ipv4 << std::endl;
      ptr = encodeAtrAddress4 (ptr, SecondaryAddress, msg.secondaryAddress);
   }

   if (password.sizeValue > 0)
   {
      if (verbose) std::clog << "HMAC with password: " << password.value << std::endl;
		
      StunAtrIntegrity integrity;
      computeHmac(integrity.hash, buf, int(ptr-buf) , password.value, password.sizeValue);
      ptr = encodeAtrIntegrity(ptr, integrity);
   }
   if (verbose) std::clog << std::endl;
	
   encode16(lengthp, uint16_t(ptr - buf - sizeof(StunMsgHdr)));
   return int(ptr - buf);
}

static bool 
stunParseAtrAddress( char* body, unsigned int hdrLen,  StunAtrAddress4& result )
{
   if ( hdrLen != 8 )
   {
      std::clog << "hdrLen wrong for Address" <<std::endl;
      return false;
   }
   result.pad = *body++;
   result.family = *body++;
   if (result.family == IPv4Family)
   {
      uint16_t nport;
      memcpy(&nport, body, 2); body+=2;
      result.ipv4.port = ntohs(nport);
		
      uint32_t naddr;
      memcpy(&naddr, body, 4); body+=4;
      result.ipv4.addr = ntohl(naddr);
      return true;
   }
   else if (result.family == IPv6Family)
   {
      std::clog << "ipv6 not supported" << std::endl;
   }
   else
   {
      std::clog << "bad address family: " << result.family << std::endl;
   }
	
   return false;
}

bool // unmodified
stunParseMessage( char* buf, unsigned int bufLen, StunMessage& msg, bool verbose) {
   if (verbose) std::clog << "Received stun message: " << bufLen << " bytes" << std::endl;
   memset(&msg, 0, sizeof(msg));
	
   if (sizeof(StunMsgHdr) > bufLen)
   {
      std::clog << "Bad message" << std::endl;
      return false;
   }
	
   memcpy(&msg.msgHdr, buf, sizeof(StunMsgHdr));
   msg.msgHdr.msgType = ntohs(msg.msgHdr.msgType);
   msg.msgHdr.msgLength = ntohs(msg.msgHdr.msgLength);
	
   if (msg.msgHdr.msgLength + sizeof(StunMsgHdr) != bufLen)
   {
      std::clog << "Message header length doesn't match message size: "
           << msg.msgHdr.msgLength << " - " << bufLen << std::endl;
      return false;
   }
	
   char* body = buf + sizeof(StunMsgHdr);
   unsigned int size = msg.msgHdr.msgLength;
	
   //std::clog << "bytes after header = " << size << std::endl;
	
   while ( size > 0 )
   {
      // !jf! should check that there are enough bytes left in the buffer
		
      StunAtrHdr* attr = reinterpret_cast<StunAtrHdr*>(body);
		
      unsigned int attrLen = ntohs(attr->length);
      int atrType = ntohs(attr->type);
		
      //if (verbose) std::clog << "Found attribute type=" << AttrNames[atrType] << " length=" << attrLen << std::endl;
      if ( attrLen+4 > size ) 
      {
         std::clog << "claims attribute is larger than size of message " 
              <<"(attribute type="<<atrType<<")"<< std::endl;
         return false;
      }
		
      body += 4; // skip the length and type in attribute header
      size -= 4;
		
      switch ( atrType )
      {
         case MappedAddress:
            msg.hasMappedAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.mappedAddress )== false )
            {
               std::clog << "problem parsing MappedAddress" << std::endl;
               return false;
            }
            else
            {
               if (verbose) std::clog << "MappedAddress = " << msg.mappedAddress.ipv4 << std::endl;
            }
					
            break;  

         case ResponseAddress:
            msg.hasResponseAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.responseAddress )== false )
            {
               std::clog << "problem parsing ResponseAddress" << std::endl;
               return false;
            }
            else
            {
               if (verbose) std::clog << "ResponseAddress = " << msg.responseAddress.ipv4 << std::endl;
            }
            break;  
				
         case ChangeRequest:
            msg.hasChangeRequest = true;
            if (stunParseAtrChangeRequest( body, attrLen, msg.changeRequest) == false)
            {
               std::clog << "problem parsing ChangeRequest" << std::endl;
               return false;
            }
            else
            {
               if (verbose) std::clog << "ChangeRequest = " << msg.changeRequest.value << std::endl;
            }
            break;
				
         case SourceAddress:
            msg.hasSourceAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.sourceAddress )== false )
            {
               std::clog << "problem parsing SourceAddress" << std::endl;
               return false;
            }
            else
            {
               if (verbose) std::clog << "SourceAddress = " << msg.sourceAddress.ipv4 << std::endl;
            }
            break;  
				
         case ChangedAddress:
            msg.hasChangedAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.changedAddress )== false )
            {
               std::clog << "problem parsing ChangedAddress" << std::endl;
               return false;
            }
            else
            {
               if (verbose) std::clog << "ChangedAddress = " << msg.changedAddress.ipv4 << std::endl;
            }
            break;  
				
         case Username: 
            msg.hasUsername = true;
            if (stunParseAtrString( body, attrLen, msg.username) == false)
            {
               std::clog << "problem parsing Username" << std::endl;
               return false;
            }
            else
            {
               if (verbose) std::clog << "Username = " << msg.username.value << std::endl;
            }
					
            break;
				
         case Password: 
            msg.hasPassword = true;
            if (stunParseAtrString( body, attrLen, msg.password) == false)
            {
               std::clog << "problem parsing Password" << std::endl;
               return false;
            }
            else
            {
               if (verbose) std::clog << "Password = " << msg.password.value << std::endl;
            }
            break;
				
         case MessageIntegrity:
            msg.hasMessageIntegrity = true;
            if (stunParseAtrIntegrity( body, attrLen, msg.messageIntegrity) == false)
            {
               std::clog << "problem parsing MessageIntegrity" << std::endl;
               return false;
            }
            else
            {
               //if (verbose) std::clog << "MessageIntegrity = " << msg.messageIntegrity.hash << std::endl;
            }
					
            // read the current HMAC
            // look up the password given the user of given the transaction id 
            // compute the HMAC on the buffer
            // decide if they match or not
            break;
				
         case ErrorCode:
            msg.hasErrorCode = true;
            if (stunParseAtrError(body, attrLen, msg.errorCode) == false)
            {
               std::clog << "problem parsing ErrorCode" << std::endl;
               return false;
            }
            else
            {
               if (verbose) std::clog << "ErrorCode = " << int(msg.errorCode.errorClass) 
                                 << " " << int(msg.errorCode.number) 
                                 << " " << msg.errorCode.reason << std::endl;
            }
					
            break;
				
         case UnknownAttribute:
            msg.hasUnknownAttributes = true;
            if (stunParseAtrUnknown(body, attrLen, msg.unknownAttributes) == false)
            {
               std::clog << "problem parsing UnknownAttribute" << std::endl;
               return false;
            }
            break;
				
         case ReflectedFrom:
            msg.hasReflectedFrom = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.reflectedFrom ) == false )
            {
               std::clog << "problem parsing ReflectedFrom" << std::endl;
               return false;
            }
            break;  
				
         case XorMappedAddress:
            msg.hasXorMappedAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.xorMappedAddress ) == false )
            {
               std::clog << "problem parsing XorMappedAddress" << std::endl;
               return false;
            }
            else
            {
               if (verbose) std::clog << "XorMappedAddress = " << msg.mappedAddress.ipv4 << std::endl;
            }
            break;  

         case XorOnly:
            msg.xorOnly = true;
            if (verbose) 
            {
               std::clog << "xorOnly = true" << std::endl;
            }
            break;  
				
         case ServerName: 
            msg.hasServerName = true;
            if (stunParseAtrString( body, attrLen, msg.serverName) == false)
            {
               std::clog << "problem parsing ServerName" << std::endl;
               return false;
            }
            else
            {
               if (verbose) std::clog << "ServerName = " << msg.serverName.value << std::endl;
            }
            break;
				
         case SecondaryAddress:
            msg.hasSecondaryAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  msg.secondaryAddress ) == false )
            {
               std::clog << "problem parsing secondaryAddress" << std::endl;
               return false;
            }
            else
            {
               if (verbose) std::clog << "SecondaryAddress = " << msg.secondaryAddress.ipv4 << std::endl;
            }
            break;  
					
         default:
            if (verbose) std::clog << "Unknown attribute: " << atrType << std::endl;
            if ( atrType <= 0x7FFF ) 
            {
               return false;
            }
      }
		
      body += attrLen;
      size -= attrLen;
   }
    
   return true;
}
