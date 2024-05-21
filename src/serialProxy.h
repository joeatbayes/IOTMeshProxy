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


#include <cstring>

class SerialProxyStream {
private:
  // Buffer to store received data
  uint8_t buffer[128]; // Adjust buffer size as needed
  int bufferIndex;

  // Buffer to store data to be transmitted
  uint8_t txBuffer[128]; // Adjust buffer size as needed
  int txBufferIndex;

public:
  ESPNowStream() : bufferIndex(0), txBufferIndex(0) {}

  // Function to handle received ESPNow packets (replace with your actual receive logic)
  void onReceive(const uint8_t *data, size_t len) {
    // Check for buffer overflow
    if (bufferIndex + len >= sizeof(buffer)) {
      // Handle buffer overflow (e.g., discard data or throw exception)
      return;
    }

    // Copy received data to buffer
    memcpy(buffer + bufferIndex, data, len);
    bufferIndex += len;
  }

  // Emulates Serial.readline, waits for a complete packet (ending with newline)
  std::string readline() {
    std::string result;

    // Loop until a newline is found or buffer is empty
    while (bufferIndex > 0 && result.find('\n') == std::string::npos) {
      // Extract character from buffer
      char c = static_cast<char>(buffer[0]);

      // Append character to result string
      result += c;

      // Remove character from buffer
      memmove(buffer, buffer + 1, bufferIndex - 1);
      bufferIndex--;
    }

    // Check if a newline was found
    if (result.find('\n') != std::string::npos) {
      // Remove trailing newline from result (optional)
      result.pop_back();
    }

    return result;
  }

  // Simulates writing data to ESPNow (replace with your actual transmit logic)
  size_t write(uint8_t data) {
    // Check for buffer overflow
    if (txBufferIndex >= sizeof(txBuffer)) {
      // Handle buffer overflow (e.g., return error or wait for space)
      return 0;
    }

    // Add data to transmit buffer
    txBuffer[txBufferIndex++] = data;

    // Replace with your actual ESPNow transmit logic (e.g., call a send function)
    // This example simulates successful transmission by returning the data size
    return 1;
  }
};



#endif
