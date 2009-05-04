#ifndef n_StunResolverPrivate_H
#define n_StunResolverPrivate_H

/*
 *
 * nucleo/network/udp/StunResolverPrivate.H --
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

#include <stdint.h>

#include <iostream>

#define STUN_VERSION "0.96"

#define STUN_MAX_STRING 256
#define STUN_MAX_UNKNOWN_ATTRIBUTES 8
#define STUN_MAX_MESSAGE_SIZE 2048

#define STUN_PORT "3478"

typedef struct {
  unsigned char octet[16]; 
}  uint128_t ;

/// define a structure to hold a stun address 
const uint8_t  IPv4Family = 0x01;
const uint8_t  IPv6Family = 0x02;

// define  flags  
const uint32_t ChangeIpFlag   = 0x04;
const uint32_t ChangePortFlag = 0x02;

// define  stun attribute
const uint16_t MappedAddress    = 0x0001;
const uint16_t ResponseAddress  = 0x0002;
const uint16_t ChangeRequest    = 0x0003;
const uint16_t SourceAddress    = 0x0004;
const uint16_t ChangedAddress   = 0x0005;
const uint16_t Username         = 0x0006;
const uint16_t Password         = 0x0007;
const uint16_t MessageIntegrity = 0x0008;
const uint16_t ErrorCode        = 0x0009;
const uint16_t UnknownAttribute = 0x000A;
const uint16_t ReflectedFrom    = 0x000B;
const uint16_t XorMappedAddress = 0x8020;
const uint16_t XorOnly          = 0x0021;
const uint16_t ServerName       = 0x8022;
const uint16_t SecondaryAddress = 0x8050; // Non standard extention

// define types for a stun message 
const uint16_t BindRequestMsg               = 0x0001;
const uint16_t BindResponseMsg              = 0x0101;
const uint16_t BindErrorResponseMsg         = 0x0111;
const uint16_t SharedSecretRequestMsg       = 0x0002;
const uint16_t SharedSecretResponseMsg      = 0x0102;
const uint16_t SharedSecretErrorResponseMsg = 0x0112;

typedef struct  {
  uint16_t msgType;
  uint16_t msgLength;
  uint128_t id;
} StunMsgHdr;

typedef struct {
      uint16_t type;
      uint16_t length;
} StunAtrHdr;

typedef struct {
  uint16_t port;
  uint32_t addr;
} StunAddress4;

typedef struct {
  uint8_t pad;
  uint8_t family;
  StunAddress4 ipv4;
} StunAtrAddress4;

typedef struct {
  uint32_t value;
} StunAtrChangeRequest;

typedef struct {
  char value[STUN_MAX_STRING];      
  uint16_t sizeValue;
} StunAtrString;

typedef struct {
  char hash[20];
} StunAtrIntegrity;

typedef struct {
  uint16_t pad; // all 0
  uint8_t errorClass;
  uint8_t number;
  char reason[STUN_MAX_STRING];
  uint16_t sizeReason;
} StunAtrError;

typedef struct {
  uint16_t attrType[STUN_MAX_UNKNOWN_ATTRIBUTES];
  uint16_t numAttributes;
} StunAtrUnknown;

typedef struct {
  StunMsgHdr msgHdr;
	
  bool hasMappedAddress;
  StunAtrAddress4  mappedAddress;
	
  bool hasResponseAddress;
  StunAtrAddress4  responseAddress;
	
  bool hasChangeRequest;
  StunAtrChangeRequest changeRequest;
	
  bool hasSourceAddress;
  StunAtrAddress4 sourceAddress;
	
  bool hasChangedAddress;
  StunAtrAddress4 changedAddress;
	
  bool hasUsername;
  StunAtrString username;
	
  bool hasPassword;
  StunAtrString password;
	
  bool hasMessageIntegrity;
  StunAtrIntegrity messageIntegrity;
	
  bool hasErrorCode;
  StunAtrError errorCode;
	
  bool hasUnknownAttributes;
  StunAtrUnknown unknownAttributes;
	
  bool hasReflectedFrom;
  StunAtrAddress4 reflectedFrom;

  bool hasXorMappedAddress;
  StunAtrAddress4  xorMappedAddress;
	
  bool xorOnly;

  bool hasServerName;
  StunAtrString serverName;
      
  bool hasSecondaryAddress;
  StunAtrAddress4 secondaryAddress;
} StunMessage; 

void
stunBuildReqSimple(StunMessage* msg, const StunAtrString& username,
			    bool changePort, bool changeIp, unsigned int id) ;

unsigned int
stunEncodeMessage(const StunMessage& msg, 
			   char* buf, unsigned int bufLen, 
			   const StunAtrString& password,
			   bool verbose) ;

bool
stunParseMessage(char* buf, unsigned int bufLen, StunMessage& msg, bool verbose) ;

#endif
