#ifndef IMP_DEFS_H
  #define IMP_DEFS_H

  #define IMP_MT_PAIR 0x1        // 01
  #define IMP_MT_PAIR_RESP 0x2   // 02
  #define IMP_MT_CONNECT 0x3     // 03
  #define IMP_MT_INVALID 0x4     // 04
  #define IMP_MT_NOTAIL 0x5      // 05
  #define IMP_MT_SLEEP_SCHED 0x6 // 06

  #define IMP_MT_UART 0xA        // 10
  #define IMP_MT_UART_FULL 0xB   // 11
  #define IMP_MT_UART_ACK 0xC    // 12
  #define IMP_MT_REQ_TEMP 0xD    // 13
  #define IMP_MT_REQ_REG  0xE    // 14
  #define IMP_MT_READ_RESP 0xF   // 15
  #define IMP_MT_WRITE_REG 0x10  // 16
  #define IMP_MT_WRITE_RESP 0x11 // 17
  #define IMP_MT_CHUNKSTART 0x12 // 18
  #define IMP_MT_CHUNK 0x13      // 19
  #define IMP_MT_CHUNK_ACK 0x14  // 20
  #define IMP_MT_CHUNK_FULL 0x15 // 21
  #define IMP_MT_CHUNK_ERR 0x16  // 22

  #define IMP_MT_DISC_CAP 0x20   // 32
  #define IMP_MT_DISC_MAC 0x21   // 33
  #define IMP_MT_DISC_PEER 0x22  // 34
  #define IMP_ROUTE_SEL 0x23     // 35
  #define IMP_ROUTE_REM 0x24     // 36 
  #define IMP_USAGE   0x25       // 37 - usage information for every node in delivery chain.
  #define IMP_REQ_USAGE 0x26     // 38 - request usage information for every node in delivery chain.


#endif

    