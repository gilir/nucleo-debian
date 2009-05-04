/*
 *
 * nucleo/utils/MD5.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * This code originally comes from http://www.fourmilab.ch/md5/
 * See http://en.wikipedia.org/wiki/MD5 for some background information
 *
 */

#include <nucleo/utils/MD5.H>
#include <nucleo/utils/ByteOrder.H>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>

namespace nucleo {

  /*
   * This code implements the MD5 message-digest algorithm.
   * The algorithm is due to Ron Rivest.	This code was
   * written by Colin Plumb in 1993, no copyright is claimed.
   * This code is in the public domain; do with it what you wish.
   *
   * Equivalent code is available from RSA Data Security, Inc.
   * This code has been tested against that, and is equivalent,
   * except that you don't need to include two pages of legalese
   * with every copy.
   *
   * To compute the message digest of a chunk of bytes, declare an
   * MD5Context structure, pass it to MD5Init, call MD5Update as
   * needed on buffers full of bytes, and then call MD5Final, which
   * will fill a supplied 16-byte array with the digest.
   */

  /* Brutally hacked by John Walker back from ANSI C to K&R (no
	prototypes) to maintain the tradition that Netfone will compile
	with Sun's original "cc". */

  /* Converted to C++ by Nicolas Roussel */

  // -------------------------------------------------------------------------

  static inline void
  byteReverse(unsigned char *buf, unsigned int longs) {
    if (ByteOrder::isLittleEndian()) return ;
    do {
	 uint32_t t = (uint32_t) ((unsigned) buf[3] << 8 | buf[2]) << 16 | ((unsigned) buf[1] << 8 | buf[0]) ;
	 *(uint32_t *)buf = t ;
	 buf += 4 ;
    } while (--longs) ;
  }

  // -------------------------------------------------------------------------
  // The four core functions

#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

  // This is the central step in the MD5 algorithm.
  
#define MD5STEP(f, w, x, y, z, data, s)						\
  ( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

  // The core of the MD5 algorithm, this alters an existing MD5 hash
  // to reflect the addition of 16 longwords of new data.  MD5Update
  // blocks the data and converts bytes into longwords for this
  // routine.

  void
  MD5::transform(uint32_t *buffer, uint32_t *input) {
    register uint32_t a, b, c, d;

    a = buffer[0];
    b = buffer[1];
    c = buffer[2];
    d = buffer[3];

    MD5STEP(F1, a, b, c, d, input[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, input[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, input[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, input[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, input[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, input[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, input[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, input[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, input[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, input[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, input[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, input[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, input[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, input[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, input[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, input[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, input[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, input[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, input[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, input[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, input[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, input[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, input[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, input[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, input[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, input[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, input[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, input[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, input[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, input[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, input[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, input[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, input[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, input[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, input[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, input[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, input[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, input[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, input[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, input[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, input[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, input[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, input[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, input[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, input[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, input[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, input[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, input[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, input[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, input[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, input[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, input[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, input[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, input[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, input[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, input[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, input[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, input[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, input[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, input[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, input[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, input[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, input[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, input[9] + 0xeb86d391, 21);

    buffer[0] += a;
    buffer[1] += b;
    buffer[2] += c;
    buffer[3] += d;
  }

  // -------------------------------------------------------------------------

  // Start MD5 accumulation. Set bit count to 0 and buffer to
  // mysterious initialization constants.

  MD5::MD5(void) {
    clear() ;
  }

  void
  MD5::clear(void) {
    buf[0] = 0x67452301 ;
    buf[1] = 0xefcdab89 ;
    buf[2] = 0x98badcfe ;
    buf[3] = 0x10325476 ;
    bits[0] = bits[1] = 0 ;
    for (int i=0; i<64; ++i) in[i] = 0 ;
  }

  // Update context to reflect the concatenation of another buffer
  // full of bytes.

  void
  MD5::eat(const unsigned char *data, unsigned int size) {
    // Update bitcount
    uint32_t t = bits[0];
    if ((bits[0] = t + ((uint32_t) size << 3)) < t)
	bits[1]++; 	/* Carry from low to high */
    bits[1] += size >> 29;

    t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

    // Handle any leading odd-sized chunks

    if (t) {
	unsigned char *p = (unsigned char *) in + t;

	t = 64 - t;
	if (size < t) {
	    memcpy(p, data, size);
	    return;
	}
	memcpy(p, data, t);
	byteReverse(in, 16);
	transform((uint32_t*)buf, (uint32_t *) in);
	data += t;
	size -= t;
    }
    /* Process data in 64-byte chunks */

    while (size >= 64) {
	memcpy(in, data, 64);
	byteReverse(in, 16);
	transform((uint32_t*)buf, (uint32_t *) in);
	data += 64;
	size -= 64;
    }

    /* Handle any remaining bytes of data. */

    memcpy(in, data, size);
  }

  // -------------------------------------------------------------------------

  // Final wrapup - pad to 64-byte boundary with the bit pattern 1 0*
  // (64-bit count of bits processed, MSB-first)

  void
  MD5::digest(bytearray digest) {
    // Compute number of bytes mod 64
    unsigned count = (bits[0] >> 3) & 0x3F;

    // Set the first char of padding to 0x80.  This is safe since
    // there is always at least one byte free
    unsigned char *p = in + count;
    *p++ = 0x80;

    // Bytes of padding needed to make 64 bytes
    count = 64 - 1 - count;

    // Pad out to 56 mod 64
    if (count < 8) {
	 // Two lots of padding:  Pad the first block to 64 bytes
	 memset(p, 0, count);
	 byteReverse(in, 16);
	 transform(buf, (uint32_t *) in);
	 // Now fill the next block with 56 bytes
	 memset(in, 0, 56);
    } else {
	 // Pad block to 56 bytes
	 memset(p, 0, count - 8);
    }
    byteReverse(in, 14);

    // Append length in bits and transform
    ((uint32_t *) in)[14] = bits[0];
    ((uint32_t *) in)[15] = bits[1];

    transform(buf, (uint32_t *) in);
    byteReverse((unsigned char *) buf, 4);

    memcpy(digest, buf, 16);

    clear() ; // In case it's sensitive
  }

  std::string
  MD5::digest(void) {
    bytearray d ;
    digest(d) ;

    std::stringstream result ;
    for (int i=0; i<16; ++i) 
	 result << std::hex << std::setfill('0') << std::setw(2) << (int)d[i] ;
    // std::cerr << "MD5: " << result.str() << std::endl ;
    return result.str() ;
  }

}
