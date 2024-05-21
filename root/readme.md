IOT Mesh Proxy
IoT Mesh Proxy to securely carry data and serial emulation across a dynamically discoverd mesh of sensor nodes.
Secure Peer Discovery: Users initiate secure pairing with other nodes using Diffie-Hellman key exchange for encrypted communication.

Dynamic Peer Discovery: Users can discover new nodes within range at any time.

Automatic Mesh Routing: The bridge automatically finds the shortest path for data delivery within the mesh network.

Multi-Proxy Node Hopping: The bridge utilizes multiple proxy nodes for efficient data forwarding across the mesh.

Capability Discovery: The bridge can automatically identify nodes with specific capabilities like data logging, serial terminal debugging, or broadband uplink.

UART Stream Relay: The bridge relays UART data streams through multiple nodes, enabling remote debugging using Linux or Windows machines.

Automatic UART stream disassebly and reassebly at both ends.

Automatic Mesh routing with shortest hop / best RSSI

UART Stream relay through multi-nodes to allow debug using remote linux or windows nods.

Delegated Trust: Once a user grants trust to a node, other nodes within the same trust network can automatically pair with it (even if installed later) without requiring further user interaction. This simplifies onboarding new nodes within a trusted group.

Register based read write capability across the network. allows basic ModBus style data exchange over the mesh.

Large binary or text transfer chunked automatically

Automatic time awake synchronization to allow all nodes maximum sleep time to maximize battery life.

Automatic recent message buffer retention to allow mesage retransmission.

CRC based message validation with automatic retransmission requests.

Design Genisis & Introduction
This work began with the intent of allowing the use of ESPNow to provide a backhaul alternative for the serial stream. It also supplies basic command processing to support a Modbus-style command-response pattern that rides over the same telemetry channel.

Serial data is not packet-oriented, so we batch up any data that has arrived in the last X milliseconds or at each newline boundary (\n) and send it over as a message, making it available to the other end's stream reader.

If more data is arriving from the input serial stream than can be sent in a single message, then it will be segmented and sent in chunks. These chunks are added to the other end's serial buffer transparently. If the serial buffer is full on the other end, then UARFULL will block further transmissions until UARTACK is received.

The system computes a CRC for each message and will request message replacement if the CRC fails. Senders are expected to maintain a buffer of recent messages so they can be resent if needed.

Secure Pairing of NOdes
Pairing nodes is done by a user action, such as a button press, which must occur on at least two nodes. Once the user action is taken, the nodes enter pairing mode for a limited amount of time, such as 60 seconds. During that time, the nodes start broadcasting their MAC address and public key for their AppId.

Any nodes also in pairing mode at the same time with a matching AppId will respond with a direct message containing their own public key. Once this handshake is complete, Diffie-Hellman key exchange is used to compute a shared secret for each node. This shared secret is used to encrypt the channel in the add peer command with ESPNow.

Requiring the user to place both nodes in pairing mode allows the user to control which nodes are allowed to join the network. For nodes that can provide a user interface, additional confirmation, such as another button press when the blink pattern matches or PIN confirmation, may be used to allow secondary user confirmation.

Power Saving
To save power, an encrypted peer connection pairing mode is only kept alive while the node is awake and within a reasonable amount of time after the last message is sent or received from that peer.

If the node needs to exchange data with that peer later, it needs to send a broadcast request to CONNECT. The receivers will look up the MAC address, and if it's present in the approved nodes list, they will re-establish the ESPNOW encrypted peer connection using previously negotiated keys.

These broadcast connections must be sent during the wake interval. However, if the requesting node doesn't get a response, it wakes up more often and stays awake longer, trying to find an interval when the other party is listening.

Basic Message FORMAT:
APP      - X3 -  Unique ID must be matched or the message is ignored
                 Start Ndx=0 Any messages that do not match APP configured
                   for this firmware are ignored.
DESTID   - X4-   Unique ID of Node message is targeted to.  This is 
                 internally mapped to a MAC address by each node 
                 Start NDX=3.  This started as MAC address but didn't
                 want to give up that much space. 
MTYPE    - X3 -  Message Type Unique string for different kinds of data 
                 sequences.  May also be considered unique kind of command
                 at least for actions that result in imperitive action.
                 Start NDX=7  
MSGID    - X3 -  Unique message id used for different kinds of ACK
                 and error processing.  Maintained as a counter 
                 inside of each client. Incremented for each message
                 they send. 
                 START NDX=10
PAYLOAD - Char Str upto 250 character max bytes less overhead in 
                 this protocol.
                 Start NDX=11,  MAXNDX=250-8; 
                 
CRC     - X8     Computed CRC for prior string. 
                 Start NDX is STRLEN(MESSAGE) - 8;
Message Types / Commands:
Commands honor basic message format above. Any additional data is sent as payload above.

     01 - PAIR      - BROADCAST Followed by HEX encoded uint64 public key. Sent after system
                    has detected user action asking for device to enter pairing mode
                    will be resent every 5 seconds for a 60 seconds after the user action.
                    sent as a broadcast message. Public key is used to compute a shared 
                    secret that will be used for a dedicated / encrypted pair channel. 
                    for that peer. Response to PAIR message will be a PAIRRESP generated by
                    any other active active node also in PAIRING mode during same time frame.
                    ignored by any nodes that do not match.

     02 - PAIRRESP  - Pairing response followed by Local public key. Used by original sender
                    of PAIR to generate a shared secret key used in ESPNoW to set encryption
                    for that peer.  

     03 - CONNECT   - BROADCAST - MACID - Sent by a disconnected client when it wants 
                    another node that it has already connected and exchanged keys with
                    to establish a encrypted peering connection using the previously
                    negotationed keys.  This implies the need to persistently save
                    previously negotiated connections so they can be reloaded at
                    reboot.

     03 - INVALID   - Indicates message received with payload of MSGID failed CRC.  Payload 
                      format  ORIG-MSG-ID(X3)SUPPLIED-CHECKSUM(X3)CALCULATED-CHECKSUM(X3)
                      receiver can ignore but a high reliability system will re-transmit 
                      that message if it is still in a ring buffer.
     04 - NOTAVAIL  - Indicates messageID from INVALID command is no longer available. 
    
     10 - UART      - Data in payload is String to be delivered to emulated stream where it can 
                    be read by client using readln.  Since this is a serial stream emulator 
                    any data should be added to the UART Buffer.  Response will either be 
                    UARTFULL or UARTACK.
     11 - UARTFULL  - Buffer to receive UART DATA is full sender should block on their end 
                    payload is MSGID of payload of UART not processed.
     12 - UARTACK   - Prior UART command has been processed and added to internal buffer.
                    payload.  Payload is message ID that has been processed. senders
                    may ignore this but once received can be removed from ring buffer.

     13 - REQTEMP   - Requests temperature from CPU. Returns message READRESP

     13 - READREG   - Requests read of Registors identified in in payload, Registors are 
                    hex encoded integers (X4) delimited by ','  Returns message READRESP

     14 - READRESP  - Response from READ request.  Registers will be returned as comma
                    delimited list app is responsible to handle any encoding and parsing
                    max size of encoded data must be less than max payload size or will
                    be truncated.  If Requested register can not be read then max value 
                    for X4 will be returned. 

     15 - WRITEREG  - Update registers, Payload is in form of REGID=VALUE\tREGID=VALUE
                      RegID will be X4,  Each entry is delimited from next set by \t.
                      values must be encoded / decoded by app logic for those registers.
                      Any side effects such as activating motors, etc is handled by
                      app level logic.  All values must fit in message payload size.
                      Response is WRITERESP

      16 - WRITERESP-

      20 - CHUNKSTART-Optional before starting sending chunks. Payload is RegId(X4),
                      Expected data size (X8), optional resourceName char[]  
                      Intended to give receiver a ability o prepaer buffers large enough
                      to hold entire received data  for all chunks. 

      21 - CHUNK    - Binary chunk payload contains a encoded binary chunk of data in 
                      payload.   Format is RegId(X4)LAST(X1)Encoded data.  Data plus
                      header must fit in payload size. LAST will be 0 if more chunks
                      to come. 1 if this is the last chunk.  This exists mostly to 
                      allow transfer of files using many messages. Response is 
                      CHUNKACK, CHUNKFULL, CHUNKERR.  First Chunk message for the 
                      By convention first chunk 
                      could contain a filename and total chunks expected. 
      22 - CHUNKACK
      23 - CHUNKFULL
      24 - CHUNKERR
PROXY & MESH
Proxy and Mesh Commands: Delivering Messages & Serial Emulation Across a diffuse sensor Network
Enable Proxy and mesh command message delivery to any node within the network, even if it requires traversing multiple intermediate nodes due to range limitations.

Routing: Messages received from MAC1 can be routed to MAC2, but the system also allows for dynamic proxy discovery.
Proxy Discovery: Each node records the MAC addresses of nodes that respond to its messages. This allows the network to discover a chain of proxy nodes with a clear path to a specific target.
TargetID and Capability-Based Discovery: To support this, a TargetID was added to the core message format. This identifies the ultimate destination of the message, independent of the current forwarding node. Additionally, nodes can be discovered based on specific capabilities (e.g., data logging, broadband access) by traversing the network to reachable nodes.
Path Selection and Maintenance:
Path Selection: Once a viable path is found, the originating node chooses the path with the best combination of hop count and wireless signal strength.
Lightweight Path Management: Once a path is established, each node only needs to remember the next node in the chain for a specific target until the path times out, fails, or network conditions change. These paths have a Time-To-Live (TTL) value to ensure updates based on dynamic network conditions.
Discovery Process:
Initiating Discovery: A node needing to send a message initiates a discovery request by sending it to all its paired neighbors.
Peer Evaluation: Each neighbor evaluates the request:
If it has a direct connection to the target node, it responds directly.
Otherwise, it forwards the request to its own peers, continuing the chain until a node with a path to the target is found.
Capability-Based Discovery: In most cases, discovery is based on desired capabilities (data logging, broadband access) rather than specific MAC addresses.
MESH SUPPORT USING INTERMEDIATE PROXIES
ESPNow is preferred over BLE mesh for this application because some of the more stable, cheaper, and power-conservative ESP32 chips lack BLE capability. Competitive pre-certified modules with FCC certification for BLE are more expensive than low-end ESP32 chips. While ESPNow is the primary target, the system is flexible and can be extended to use BLE, LORA, or other protocols for message delivery. This allows us to use LORA when using ESPNow would require too many proxy hops. It also allows a single message to cross multiple back ends before it's ultimate delivery.

A formal proxy approach was not chosen because this is a highly distributed mesh network with hundreds of sensor nodes in the field. These nodes need to auto-discover other nodes within range and use those connections to find the telemetry delivery path without pre-defined configuration. This delivery path must be adaptable, automatically taking advantage of new nodes or finding routes even if nodes are removed. The large number of participating nodes and the fact that any node may need to act as a proxy are characteristics of not typical for Wi-Fi mesh networks.

This approach avoids flooding by using a peer-and-route discovery method based on previously established connections. Since each node has a list of neighbors it has already paired with and exchanged keys, when one of its clients wants to discover a path to another node, it simply sends a request along to each of its pre-paired neighbors. The payload of the request will be the list of MAC addresses it has traversed so far.

Since each MAC address is 8 bytes long, this limits the maximum discovery depth to about 28, which should be sufficient for most applications. If a node receives the message and knows the target MAC address, it doesn't forward the message. Instead, it responds with the message including the chain of addresses plus itself. This allows the first-level proxy or even the original sender to evaluate all valid paths and choose the shortest one before sending the ESTPROXY command.

MAC addresses are unique identifiers, but in many cases, we don't necessarily care about them. Instead, we use a 3-byte TargetID that is internally mapped to a MAC address by each node. In most cases, discovery is not for a specific MAC address, but rather to find a node with a specific characteristic, such as having broadband uplink, serial debug capability, serial command capability, or data logging capability. Each node can claim one of the 4096 unique IDs, but it must first query the network to ensure it's available and not in use by another node.

When the discovery message is sent, it may include a capability list of 3-byte codes that reflect different capabilities, replacing a target MAC address or Target NodeID. Only nodes that have those capabilities will respond. During path discovery, nodes can only forward the message if their capabilities match.

Once the node that chose a path sends the ESTPROXY command, the intermediate nodes only need to remember the target MAC address and the next step in the proxy chain. If a path fails to deliver the message, the last successful receiver sends back a fail message, triggering a new discovery path.

The analyzer node needs to know the relative RSSI (received signal strength indicator) of each responding node in the link. This is because the path may have short hops with very low signal strengths, which we want to avoid. So, during discovery, each participating node briefly enters AP mode so the peer can query for RSSI using a standard Wi-Fi scan.

Proxy, MESH & Discovery Commands
ACTIONS:
TODO: Negotiate WAKUP interval so clients and servers can go into deep sleep but still be avaialble to recieve next message. Needs to allow mechanism to stay awake or wake up often to resync time if initial connection fails.