/* util.h

*/

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


