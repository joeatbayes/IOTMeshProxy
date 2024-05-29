# Carriers and how IOTMeshProxy will support each one

## ESPNOW
ESPNow is preferred over BLE mesh for this application because some of the more stable, cheaper, and power-conservative ESP32 chips lack BLE capability. Competitive pre-certified modules with FCC certification for BLE are more expensive than low-end ESP32 chips. While ESPNow is the primary target, the system is flexible and can be extended to use BLE, LORA, or other protocols for message delivery. This allows us to use LORA when using ESPNow would require too many proxy hops. It also allows a single message to cross multiple back ends before it's ultimate delivery. 

## ESPNow To RS232 Emulation
This was our base use case where we wanted to transport serial debug traffic from remote nodes over a unknown number of intermediate proxies for display in a serial console. This mode of operation requires a dedicaed ESPNOW / IoTMeshProxy node which is connected to a RS232 Adapter that feeds into the Serial software such as TeraTerm.  We used Hardware Serial on the ESP32-S3 module wired to a USB to TTL Serial adapter which plugs into our linux host via USB.

Contact us with your special requirements so we can help guide your implementation or supply custom features as needed. 



## LORA
Lora offers much longer range then ESPNow which can allow longer bridges with few intermediate proxy nodes.  Additional work remains to determine how we will integrate the LORA encryption module to deliver similar functionality as ESPNOW.   It should be acknowledged that LORA transmissions will be much slower than ESPNow.  

Constact us if you need a LORA based bridge so we can prioritize it. 

## BLE
BLE offsers a Advertiser mechansim which allows discovery of nodes when they are available along with messaging mechanism between nodes which allows it to work much like the ESPNOW solution without using the BLE Mesh flooding capability. 

Contact us if you need a BLE based network or a BLE bridge so we can prioritize it. 

## Bluetooth Service
Constact us if you need a BlueTooth based solution or a Bluetooth based bridge so we can prioritize it. 

## WiFi with Access Point & Server 
WiFi Access points is our default assumption for uplink where individual devices configured as bridge devices proxy messages for hundreds of IoT nodes onto the network for delivery to upstream processing.   This can be done via REST calls or via messaging protocols such as MQTT.  We use this capability to deliver our Serial emulation to the ultimate terminal end points. 

Constact us if you need a WiFi Bridge capability so we can prioritize your requirements.


## Cellular uplink
Cellular uplink nodes are the primary mechanism used to uplink data and downlink commands into isolated IoT Mesh proxy networks when access is available via traditional boradband.

Contact us with your specific requirements so we can priorize them. 

## Internet with Multicast, Unicast & Broadcast

In Networks where you have devices that have already joined as nodes in a larger network  IoT Mesh proxy can help extend that network to cover all the nooks and crannys where the WiFi signal may not be strong enough.    We use Multicast in liue of broadcast for node discovery and Unicast along with direct network socket stream connections for message processing.  We still seek to keep the individual nodes awake only during periods when interacting with the network to maintian long battery life. 

Constact us if you need a Local Network multi-cast bridge so we can prioritize it. 


## MATTER
Matter doesn't relaly provide a total proxy mesh solution but it does provide a strong mechansim to bridge devices onto the public internet in a secure fashion.  We envision providing a MATTER bridge which will allow all nodes in a IoTMesh Proxy network to become visibile as Matter interactive devices. 

Contact us if you need a Matter bridge so we can prioritize it.
