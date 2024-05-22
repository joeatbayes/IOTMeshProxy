// keys.h
//
// A semi secure method to allow devices in the 
// field to exchange initial keys used to secure
// an encrypted channel they use to exchange 
// true randomized keys.
//
// It starts with shared seed key which is then 
// modified with the MAC address of both devices
// which is then modified by a random number which 
// is exchanged over a non-encrypted channel. 
//
// Could not get diffie hellman to work with enough bits 
// on this class of device to make it truely secure so 
// here is a hacked version of a semi-secure derived key 
// we only use the semi-secure key to encrypt the 
// conversation for long enough to exchange a true 
// 64 bit random key. Not terribly secure against
// a malicious listener who could dereive these 
// keys if they know the algorithm.  I consider it
// reasonable risk for now since these keys are 
// at risk if the snooper is in RF range when the
// user is configuring devices and adding them to
// the network.   Will revisit this when I find a 
// good eliptic curve base diffie hellman library

#ifndef MP_KEY
 #define mpkeysh

//#include <stdlib.h>
//#include <ctype.h>
#include <string.h>
//#include <math.h>
#include <stdio.h>

#ifndef KEYS_KEY1
  // overridge this by defining application or company specific 
  // _KEY1.  If this number is different then key handshake 
  // will not work. 
  const uint64_t KEYS_KEY1 = 5370282887863;
#endif 


uint64_t makeInitKey(uint64_t startKey, uint8_t *mac1, uint8_t *mac2, uint64_t rnum ) {
    uint64_t k1m;
    uint64_t k2m;
    memcpy(&k1m,mac1,6);
    memcpy(&k2m,mac2,6);
    return startKey | k1m ^ k2m | rnum;
}



#endif 
