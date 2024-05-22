/* util.h

*/

#ifndef IMP_UTIL_H
  #define IMP_UTIL_H
  #include <stdio.h>
  #include <stdlib.h>
  #include <Arduino.h>
  #include "esp_random.h"
  

// Format a standard format 6 byte MAC for human
// readability
void formatMac(char *dest, const uint8_t *mac) {
  sprintf(dest, "%X:%X:%X:%X:%X:%X",  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ;
}

// return number of milliseconds that have elapsed since
// the start time passed in.
unsigned long elapMs(unsigned long start) {
  return millis() - start;
}



inline int min(int x, int y) {
  return (x < y) ? x : y;
}

inline int max(int x, int y) {
  return (x > y) ? x : y;
}


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
  return (uint64_t) tr;
}



#endif
