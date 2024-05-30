# IOT Mesh Proxy 
### STATUS: UNDER CONSTRUCTION - Please provide comments and contact us before using in an Important project See: license.txt

#### The Challenge of Large-Scale Sensor Networks:

Imagine deploying thousands of sensor nodes across hundreds of acres, an area far exceeding the reach of typical Wi-Fi mesh networks. Now, picture doing this without a dedicated IT team or expensive network infrastructure – a daunting task, right? The goal is to efficiently collect data from all these battery-powered nodes, ensuring long lifespans with limited solar recharge. Existing solutions often fall short. 

#### A Cost-Effective and Dynamic Solution:

IoTMeshProxy's innovative approach tackles this challenge head-on. It utilizes low-cost nodes (under $5 each) that seamlessly join the network mesh in an ad-hoc manner.  It eliminates the need for pre-configured placements. The network itself is self-organizing, automatically adjusting routing paths as new nodes are added. Additionally, it supports the automatic discovery of nodes offering specific services, such as data logging or high-bandwidth uplinks.

#### Scalability and Adaptability:

The beauty of this system lies in its scalability and adaptability. Weak signal areas can be easily addressed by adding a few bridge nodes. Similarly, exceeding bandwidth limits in specific zones can be resolved by adding uplink nodes with the routing automatically adapting to optimize traffic flow.  The system overcomes common issues with typical mesh systems by using a semi-directed routing approach. This allows each message to follow the shortest available path, with exceptions made to avoid overloaded connections. Only discovery and network formation messages are distributed more widely, and only as far as needed to establish the shortest paths. This dramatically reduces the total message volume that each node sees, improving overall network data capacity and extending battery life.

- [IoTMesh Proxy Git repository](https://github.com/joeatbayes/IOTMeshProxy)
  - TODO: Example [Serial Stream Debug Proxy](docs/serial-proxy.md)
  - TODO: Example data bridge to MQTT
  - TODO: Example data bridge to MATTER
  - 
- [IoT Mesh Proxy Protocol design specification](docs/protocol.md)
- [Handshaking between nodes](docs/handshake.md)
- [Carriers for different RF backend delivery options](docs/carriers.md)

#### IoT Mesh Proxy to securely carry data and serial emulation across a dynamically discoverd mesh of sensor nodes.

   - Secure Peer Discovery: Users initiate pairing with
     other nodes using encrypted key exchange for
     encrypted communication.
     
   - Dynamic Peer Discovery: Users can discover new nodes 
     within range at any time.
     
   - Automatic Mesh Routing: The bridge automatically finds the
     shortest path for data delivery within the mesh network.
     
   - Multi-Proxy Node Hopping: The bridge utilizes multiple 
     proxy nodes for efficient data forwarding across the mesh.
     
   - Capability Discovery: The bridge can automatically identify 
     nodes with specific capabilities like data logging, serial 
     terminal debugging, or broadband uplink.
     
   - UART Stream Relay: The bridge relays UART data streams 
     through multiple nodes, enabling remote debugging using 
     Linux or Windows machines.
     
   - Automatic UART stream disassebly and reassebly at both ends.
   
   - Automatic Mesh routing with shortest hop / best RSSI
   
   - UART Stream relay through multi-nodes to allow
     debug using remote linux or windows nods.
     
   - Delegated Trust: Once a user grants trust to a node, 
     other nodes within the same trust network can automatically
     pair with it (even if installed later) without requiring 
     further user interaction. This simplifies onboarding new nodes
     within a trusted group.
     
   - Register based read write capability across the network.
     allows basic ModBus style data exchange over the mesh. 
     
   - Large binary or text transfer chunked automatically
   
   - Automatic time awake synchronization to allow all nodes
     maximum sleep time to maximize battery life.
     
   - Automatic recent message buffer retention to allow 
     mesage retransmission.
     
   - CRC based message validation with automatic retransmission
     requests.
      
----
## Design Genisis & Introduction

This work began with the intent of allowing the use of ESPNow to provide a backhaul alternative for the serial stream. It also supplies basic command processing to support a Modbus-style command-response pattern that rides over the same telemetry channel.

Serial data is not packet-oriented, so we batch up any data that has arrived in the last X milliseconds or at each newline boundary (\n) and send it over as a message, making it available to the other end's stream reader.

If more data is arriving from the input serial stream than can be sent in a single message, then it will be segmented and sent in chunks. These chunks are added to the other end's serial buffer transparently. If the serial buffer is full on the other end, then UARFULL will block further transmissions until UARTACK is received.

The system computes a CRC for each message and will request message replacement if the CRC fails. Senders are expected to maintain a buffer of recent messages so they can be resent if needed.

----
##  Secure Pairing of NOdes

Pairing nodes is done by a user action, such as a button press, which must occur on at least two nodes. Once the user action is taken, the nodes enter pairing mode for a limited amount of time, such as 60 seconds. During that time, the nodes start broadcasting their MAC address and public key for their AppId.

Any nodes also in pairing mode at the same time with a matching AppId will respond with a direct message containing their own public key. Once this handshake is complete, Diffie-Hellman key exchange is used to compute a shared secret for each node. This shared secret is used to encrypt the channel in the add peer command with ESPNow.

Requiring the user to place both nodes in pairing mode allows the user to control which nodes are allowed to join the network. For nodes that can provide a user interface, additional confirmation, such as another button press when the blink pattern matches or PIN confirmation, may be used to allow secondary user confirmation.


----
## Power Saving

 To save power, an encrypted peer connection pairing mode is only kept alive while the node is awake and within a reasonable amount of time after the last message is sent or received from that peer.

If the node needs to exchange data with that peer later, it needs to send a broadcast request to CONNECT. The receivers will look up the MAC address, and if it's present in the approved nodes list, they will re-establish the ESPNOW encrypted peer connection using previously negotiated keys.

These broadcast connections must be sent during the wake interval. However, if the requesting node doesn't get a response, it wakes up more often and stays awake longer, trying to find an interval when the other party is listening.


 

# BLE Mesh Flooding and Upfront Planning:

BLE mesh flooding, while conceptually simple, presents a challenge for large-scale sensor networks. Due to message congestion and the cost of message replication across nodes, it necessitates careful upfront planning involving clustering, inter-cluster communication, and dedicated backhaul for bridges. Failing to consider these factors can quickly exhaust the message transfer capacity of every node, leading to network instability, shortented battery life and unreliable data collection. This planning involves:

 * Clustering: Grouping BLE devices into clusters helps manage traffic and prevents network-wide flooding.
 * Inter-Cluster Communication: Bridges are needed to control traffic movement between clusters, ensuring messages stay within designated areas.
 * Backhaul for Bridges: These bridges require dedicated backhaul paths to deliver data to uplink nodes without overwhelming other clusters with messages.

Limitations of Flooding: This flooding approach not only exhausts the battery of the nodes that need to deliver the message, but also drains the battery of every node involved in the retransmission, significantly impacting network lifespan.

The IoTMeshproxy Solution:  Our innovative approach utilizes an auto-forming shortest path strategy with congestion avoidance. This eliminates the need for complex upfront planning by dynamically routing messages along the shortest available paths, automatically bypassing congested links. This significantly reduces message overhead and network traffic, making it ideal for large-scale deployments with minimal IT intervention.



### MESH SUPPORT USING INTERMEDIATE PROXIES


A formal proxy approach was not chosen because this is a highly distributed mesh network with hundreds of sensor nodes in the field. These nodes need to auto-discover other nodes within range and use those connections to find the telemetry delivery path without pre-defined configuration. This delivery path must be adaptable, automatically taking advantage of new nodes or finding routes even if nodes are removed. The large number of participating nodes and the fact that any node may need to act as a proxy are characteristics of not typical for Wi-Fi mesh networks.

This approach avoids flooding by using a peer-and-route discovery method based on previously established connections. Since each node has a list of neighbors it has already paired with and exchanged keys, when one of its clients wants to discover a path to another node, it simply sends a request along to each of its pre-paired neighbors. The payload of the request will be the list of MAC addresses it has traversed so far.

Since each MAC address is 8 bytes long, this limits the maximum discovery depth to about 28, which should be sufficient for most applications. If a node receives the message and knows the target MAC address, it doesn't forward the message. Instead, it responds with the message including the chain of addresses plus itself. This allows the first-level proxy or even the original sender to evaluate all valid paths and choose the shortest one before sending the ESTPROXY command.

MAC addresses are unique identifiers, but in many cases, we don't necessarily care about them. Instead, we use a 3-byte TargetID that is internally mapped to a MAC address by each node. In most cases, discovery is not for a specific MAC address, but rather to find a node with a specific characteristic, such as having broadband uplink, serial debug capability, serial command capability, or data logging capability. Each node can claim one of the 4096 unique IDs, but it must first query the network to ensure it's available and not in use by another node.

When the discovery message is sent, it may include a capability list of 3-byte codes that reflect different capabilities, replacing a target MAC address or Target NodeID. Only nodes that have those capabilities will respond. During path discovery, nodes can only forward the message if their capabilities match.

Once the node that chose a path sends the ESTPROXY command, the intermediate nodes only need to remember the target MAC address and the next step in the proxy chain. If a path fails to deliver the message, the last successful receiver sends back a fail message, triggering a new discovery path.

The analyzer node needs to know the relative RSSI (received signal strength indicator) of each responding node in the link. This is because the path may have short hops with very low signal strengths, which we want to avoid. So, during discovery, each participating node briefly enters AP mode so the peer can query for RSSI using a standard Wi-Fi scan.