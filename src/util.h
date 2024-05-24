/* util.h

*/

#ifndef IMP_UTIL_H
  #define IMP_UTIL_H
  #include <stdio.h>
  #include <stdlib.h>
  #include <Arduino.h>
  #include "esp_random.h"
  
  #define ESP32CRC
  #ifdef ESP32CRC
    #include <esp_crc.h>
    //#include "crc.h"
  #else
    #include "CRC.h" // see: https://github.com/RobTillaart/CRC/
    #include "CRC16.h"
  #endif

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


long getHexInt(uint8_t *data, short startNdx, short len) {
    char buff[16];
    strncpy(buff, (const char *) data + startNdx, len);
    buff[len]=0;
    long aint = (int)strtoul(buff, NULL, 16);
    //Serial.printf("startNdx=%d len=%d buff=%s aint=%x aintD=%d",
    //  startNdx, len, buff, aint, aint);
    //delay(50);
    return aint;
}

int calcCRC(const uint8_t *data, int size) {
        Serial.printf("L274: data=%s size=%d\n", data, size);
        delay(50);
        #ifdef ESP32CRC
          int crcCalc = esp_crc16_le(0, data, size);
        #else
          int crcCalc= calcCRC16((const uint8_t  *) buff, size);
        #endif
        Serial.printf("L281: crcCalc=%x\n", crcCalc);
        delay(50);
        return crcCalc;
}




#endif
