/* diffi.h - Diffie-Hellman key exchange utility to support key exchange
*   over untrusted channels.
* 
* Define DHPRIME_P, DHPRIME_G in your own module before 
* importing to use your own prime factors.  Using your own prime numbers
* makes more secure but also makes it non-interoperable with anybody
* who is not using the same numbers.
*/

#ifndef diffieh
  #define diffieh

#include <esp_random.h>


#ifndef DHPRIME_P
  #define DHPRIME_P 329565970448322901398560964842595861041ULL
#endif

#ifndef DHPRIME_G 
  #define DHPRIME_G 329565970448322901398560964842595860927ULL
#endif 


// Function to generate a random number within a given range (inclusive)
uint64_t random_in_range(uint64_t min, uint64_t max) {
  uint64_t tr;
  esp_fill_random(&tr, 8);  
  //esp_random(&tr);  
  while (tr < min) {    
    uint8_t adjv;
    esp_fill_random(&adjv, 1);  
    tr = tr >> 8;
    *(uint8_t *)tr = adjv;
  }
  while (tr > max) {
    tr = tr << 1;
  }
  return (long long) tr;
}

uint64_t genPrivateKey() {
  return random_in_range(1, DHPRIME_P - 2);
}

uint64_t genPublicKey(uint64_t privKey) {
  return (uint64_t) pow(DHPRIME_G, privKey) % DHPRIME_P;
}

uint64_t calcSharedSecret(uint64_t otherPubKey, uint64_t myPrivKey) {
  return (uint64_t) pow(otherPubKey, myPrivKey) % DHPRIME_P;
}


#endif
