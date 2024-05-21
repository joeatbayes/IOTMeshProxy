/*  SerialProxy.h
  
Emulated Serial Port over ESPNow Mesh

This project provides an emulated serial port that replaces the default
serial port.  It Routes traffic through a remote ESP32 node for delivery
and display via serial bridge for viewing on Unix, Windows, and Mac
devices.

The project originated from the need to access serial debug streams 
from field devices without requiring a physical laptop connection. 
These devices are Sealed, making physical access for diagnostics  
difficult. Connecting to a laptop via USB cables
is potentially hazardous due to high voltages (up to 36V) on
the same board as the 3.3V CPU supply. A previous debugging 
session resulted in a destroyed laptop due to an accidental 
short.    USB optical isolators have turned out supper buggy
and have consumed as much diagnostic time as testing the original
circuits.

This emulated serial port eliminates the need for a physical connection
ensuring safer and more convenient debugging.

Choosing ESPNow and Mesh Networking: ESPNow: Many target devices are 
hundreds of feet away from Wi-Fi nodes and battery-powered.
Minimizing radio usage is crucial, and ESPNow's low-power nature
makes it ideal.

Mesh Networking: The sensor nodes are often too far apart for 
single-hop data delivery. A dynamic mesh that can automatically 
add new nodes without pre-configuration is required. This 
allows for quick and easy addition of new nodes by Rainamp 
customers, who may not have technical expertise.  No
pre-existing knowledge of mac addresses or network topology
can be required but it still must be secure against undesired
devices joining the network. Pre-shared keys or keys used
to secure comminications between more than 2 devices are not 
allowed. 

In essence, the system enables easy and safe debug stream access 
even for non-technical users in the field.

See: ../readme.md for protocol and design overview.
*   (C) Joseph Ellsworth 2024 All Rights Reserved
*   See License.txt
*/

#ifndef serialProxyh
  #define serialProxyh

#include "Stream.h"
#include <iostream>
#include <sstream>
#include <string>
#include <functional>
#include "IoTMeshProxy.h"
// Include your network messaging library headers here
// (replace with your specific library)
// For example: #include <custom_network_library.h>

class NetworkStream : public Stream {

private:
  std::string txBuffer;
  std::string rxBuffer;
  IoTMeshProxy *mprox;
  Stream *ser;

public:
  NetworkStream(Stream *serialSource, IoTMeshProxy *mp, int rxBuffSize, int txBuffSize) {
    ser = serialSource;
    mprox = mp;
    txBuffer.reserve(rxBuffSize);
    rxBuffer.reserve(rxBuffSize);
  }


  // Implementations for Stream virtual functions
  int available() override {
    // In this case, available data over the network might be unreliable
    // You can return 0 or implement logic to check for available data
    // based on your network library.
    return 0;
  }

  int read() override {
    // Reading from a network stream might not be character-by-character
    // You might need to implement buffering or rely on higher-level 
    // network message parsing depending on your library.
    return -1; // Indicate no character available for now
  }

  int peek() override {
    // Similar to read(), peeking might not be straightforward
    // Implement based on your network library capabilities.
    return -1;
  }

  // Implementations for Print virtual functions (assuming write is used)
  size_t write(uint8_t b) override {
    // Handle writing a single byte to the network stream
    // Implement based on your network library.
    buffer_ += static_cast<char>(b);
    if (buffer_.size() >= BUFFER_SIZE || b == '\n') {
      // Send buffer content over the network (replace with actual implementation)
      send_message(address_, buffer_);
      buffer_.clear();
    }    
    return 1; // Indicate successful write (replace with actual implementation)
  }

  size_t write(const uint8_t* buffer, size_t size) override {
    // Handle writing a buffer of bytes to the network stream
    // Implement based on your network library.
    // You can use send_message or a separate function for efficiency.
    std::string data(buffer, size);
    send_message(address_, data);
    return size; // Indicate successful write (replace with actual implementation)
  }

  std::string address_;

  // Function to send message using your network messaging library
  // (replace with your specific implementation)
  virtual void send_message(const std::string& address, const std::string& message) = 0;
};

int main() {
  // Use NetworkStream instead of std::cout
  NetworkStream network_stream("server_address");
  network_stream.print("This message will be sent over the network: ");
  network_stream.println(42);
  return 0;
}


#endif
