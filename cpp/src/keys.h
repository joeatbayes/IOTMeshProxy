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
// keys if they know the algorithm. The Keys_KEY1
// value could likley be reverse engineered if 
// a hacker knows the algorithm and both MAC 
// which they can get from snooping and / or 
// reverse engineering the firmware. I consider it
// reasonable risk for now since these keys are 
// at risk if the snooper is in RF range when the
// user is configuring devices and adding them to
// the network.   Will revisit when I find a 
// good eliptic curve base diffie hellman library
// is not needed when using a backhaul like BLE
// which handles trusted peer addition using a 
// different mechanism.
// 
// NOTE: See ESP32-S3 data sheet RAS Accelerator
//   Large-number modular exponentiation with two optional acceleration options
//   Should be able to use this to support diffie hellman

#ifndef MP_KEY
 #define mpkeysh

//#include <stdlib.h>
//#include <ctype.h>
#include <string.h>
//#include <math.h>
#include <stdio.h>
#include "util.h"
#include <Preferences.h>


#ifndef KEYS_DEFAULT_SHARED
  const uint64_t KEYS_DEFAULT_SHARED = 7241282984813;
#endif

#ifndef IMMPref
   Preferences IMPPref;
#endif


#ifndef KEYS_KEY1
  // overridge this by defining application or company specific 
  // _KEY1.  If this number is different then key handshake 
  // will not work. 
  const uint64_t KEYS_KEY1 = 5370282887863;
#endif 


class IMP_Keys {
  public: 
  // Return the currently active shared password
  // if saved in prefrences return that password
  // otherwise use the one in the firmware here
  static uint64_t getSharedPassword() {
    uint64_t retVal;
    IMPPref.begin("IMP", true);
    retVal = (uint64_t) IMPPref.getULong64("IMP_SHARED_KEY", KEYS_DEFAULT_SHARED);
    IMPPref.end();
    return retVal;
  }

  static uint64_t makeInitKey(uint64_t startKey, uint8_t *mac1, uint8_t *mac2, uint64_t rnum ) {
      uint64_t k1m;
      uint64_t k2m;
      memcpy(&k1m,mac1,6);
      memcpy(&k2m,mac2,6);
      return startKey | k1m ^ k2m | rnum;
  }


  static uint64_t makeInitKey(uint64_t startKey, uint8_t *mac1, uint8_t *mac2 ) {
      uint64_t rnum = random_in_range(0,__UINT64_MAX__);
      return makeInitKey(startKey, mac1, mac2, rnum);
  }

  static void formatKey64(char *dest, uint64_t numIn) {
    sprintf(dest, "%16X", numIn);
  }


 } // class

#endif 
