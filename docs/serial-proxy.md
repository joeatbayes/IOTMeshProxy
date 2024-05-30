#Serial Proxy Configuration & Example

## Serial Debug Enabled Module
This is the module which is generating the serial stream we need to display and interact with. Including the ability to send commands bi-directional.   This is accomplished by using the IMP_Serial_proxy class which allows all commands to Serial.print to be intercepted and redirected over the IoTMeshProxy.

##  IoTMeshProxy relay units
These are the modules that can extend the range between the ultimate serial console and the units producing the serial debug stream.  They act as a self organizing mesh of nodes and will relay the traffic in encrypted form across as many nodes as needed for final delivery.

## IOTMeshProxy to Serial Bridge 
This  is the module configured to bridge one or more serial streams onto a single physcial sterial stream connected to a desktop running linux, windows or macos via a USB to serial adapter. Includes a command sequence which allows different serial streams to be routed. 

## IoTMeshProxy to Bluetooth Serial bridge
This is the module conigured to bridge one or more serial streams onto a single physical serial stream.  Includes a Bluetooth GUI which allows selecting which module to set as the active serial stream.    

TODO: Define a serial bridge module which generates different backend log files for many modules generating serial data streams by writing onto a SD or bridging onto single serial data stream with prefixes that allow a program running on the linux box to separate them back out into distrete debugging streams. 



