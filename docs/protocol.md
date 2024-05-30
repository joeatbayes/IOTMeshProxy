The basic protocol is designed to be compatible with any underlying delivery
vehicle that has both broadcast and unicast capability and the ability to pass
string messages.  The first version assumes MaxMessage is 250 bytes but it can 
easily be adapted to larger or smaller message formats.   The basic overhead 
is 18 bytes so the shortest message format must accomodate that amount of overhead
while still carying a meaningful amount of data.

Implmenations are feasible with ESPNow,  Lora, BLE and even higher level TCP/IP with broadcast and Unicast.  The first version was implemented over ESPNOW.

----
##  Basic Message FORMAT:

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
    MSGID    - X4 -  Unique message id used for different kinds of ACK
                     and error processing.  Maintained as a counter 
                     inside of each client. Incremented for each message
                     they send. 
                     START NDX=10
    PAYLOAD - Char Str upto 250 character max bytes less overhead in 
                     this protocol.
                     Start NDX=14,  MAXNDX=250-8,
                     MaxLen= 250 - (14+8) = 230 bytes
                     
    CRC     - X4     Computed 16bit little endian CRC for prior string. 
                     Start NDX is STRLEN(MESSAGE) - 4;

*


----
## Message Types / Commands:

   Commands honor basic message format above.  Any additional data is sent as
   payload above. 

```
     01 - PAIRM     - BROADCAST Sent after system has detected user action 
                    asking for device to enter pairing mode will be resent 
                    every 5 seconds for a 60 seconds after the user action.
                    sent as a broadcast message.  Receiving nodes that are
                    in pairing mode will respond with PAIR-REQ message.                    

     02 - PAIR-REQ - Pairing response Used by original  by receiver of PAIRM
                    to indicate a desire to create a pair. Once sent both 
                    nodes will add the other as active peer using Pre-
                    shared key to encrypt the channel. Once sent the receiver 
                    of PAIRM will send PAIR-RAND to the partner.   On devices where
                    a user interface is possible this message will include PIN 
                    information from user.   
                    
                    NOTE: When Pairing is enabled by user the system will have 
                    gathered data such as number of times user pressed the pair 
                    button in a 10 second internval then blink a LED that many times
                    the user can be required to press the button the same number of 
                    times on the peer device.  The communication system doesn't
                    specify the method but in this instance it is treated as a 
                    1 digit PIN. Devices with more UI capability may use a longer
                    PIN.


     03 - PAIR-RAND - Sent by receiver of PAIRM. on encrypted channel after 
                    sending PAIR-REQ. Sent once every 3 seconds until PAIR-RAND-ACK
                    is recieved or 15 seconds have elapsed.  Inlcudes a large RANDOM
                    # which is used in keys.h to compute a new shared key for 
                    connection between these peers. It is this new key that is saved
                    for future communication between these nodes. 
                     
                    There is a small risk of two devices waking near simutaneously
                    causing both nodes to try to send PAIR-RAND to each other.
                    In this case the node with the numerically lowest MAC address 
                    will win and it's random number will be used. 
                     
     04 - PAIR-RAND-ACK - Sent by receiver of PAIRM to indicate the random number
                    exchanged for the key has been received and sucessfully applied.
                    Once the PAIR-RAND-ACK is reeived both nodes switch to the new 
                    computed key. 

     05 - PAIR-TIMING - Sent by both Nodes after PAIR-RAND-ACK has been exchanged
                    contains timing data about each nodes intent for sleep and 
                    wake cycles.  The counter party nodes should use this to 
                    adjus wake cycles to match so they are both awake during the 
                    same intervals. 
                    

     06 - NODE-AVAIL- Broadcast sent when node wakes up from deep sleep. 


     07 - NODE-AVAIL-ACK - used by peer to allow node that sent the NODE-AVAIL 
                    to know that it's NODE-AVAIL message
                    has been received and processed by a previously paired node.
                    Payload for this is a flag indicating whether we node sending 
                    ACK desires a secure connection. When set this value 
                    indicaes a node that it has already connected and exchanged keys with
                    to establish a encrypted peering connection using the previously
                    negotationed keys.  This implies the need to persistently save
                    previously negotiated connections so they can be reloaded at
                    reboot.  If a Node sending NODE-AVAIL does not receive ACK then 
                    it may indicate loss of time synchronization and require Resync
                    time process to be initiated. 

     08 - INVALID   - Indicates message received with payload of MSGID failed CRC.  Payload 
                    format  ORIG-MSG-ID(X3)SUPPLIED-CHECKSUM(X3)CALCULATED-CHECKSUM(X3)
                    receiver can ignore but a high reliability system will re-transmit 
                    that message if it is still in a ring buffer.

     09 - NOTAVAIL  - Indicates messageID from INVALID command is no longer available. 
    
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

     14 - READREG   - Requests read of Registors identified in in payload, Registors are 
                    hex encoded integers (X4) delimited by ','  Returns message READRESP

     15 - READRESP  - Response from READ request.  Registers will be returned as comma
                    delimited list app is responsible to handle any encoding and parsing
                    max size of encoded data must be less than max payload size or will
                    be truncated.  If Requested register can not be read then max value 
                    for X4 will be returned. 

     16 - WRITEREG  - Update registers, Payload is in form of REGID=VALUE\tREGID=VALUE
                      RegID will be X4,  Each entry is delimited from next set by \t.
                      values must be encoded / decoded by app logic for those registers.
                      Any side effects such as activating motors, etc is handled by
                      app level logic.  All values must fit in message payload size.
                      Response is WRITERESP

      17- WRITERESP-

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
```      

----
## PROXY & MESH

#### Proxy and Mesh Commands: Delivering Messages & Serial Emulation Across a diffuse sensor Network

Enable Proxy and mesh command message delivery to any node within the
network, even if it requires traversing multiple intermediate nodes
due to range limitations.


  - **Routing:** Messages received from MAC1 can be routed to MAC2, but the system also allows for dynamic proxy discovery.
  - **Proxy Discovery:** Each node records the MAC addresses of nodes that respond to its messages. This allows the network to discover a chain of proxy nodes with a clear path to a specific target.
  - **TargetID and Capability-Based Discovery:** To support this, a TargetID was added to the core message format. This identifies the ultimate destination of the message, independent of the current forwarding node. Additionally, nodes can be discovered based on specific capabilities (e.g., data logging, broadband access) by traversing the network to reachable nodes.

#### Path Selection and Maintenance:

  - **Path Selection:** Once a viable path is found, the originating node chooses the path with the best combination of hop count and wireless signal strength.
  - **Lightweight Path Management:** Once a path is established, each node only needs to remember the next node in the chain for a specific target until the path times out, fails, or network conditions change. These paths have a Time-To-Live (TTL) value to ensure updates based on dynamic network conditions.

#### Discovery Process:

  - **Initiating Discovery:** A node needing to send a message initiates a discovery request by sending it to all its paired neighbors.
  - **Peer Evaluation:** Each neighbor evaluates the request:
      - If it has a direct connection to the target node, it responds directly.
      - Otherwise, it forwards the request to its own peers, continuing the chain until a node with a path to the target is found.
  - Capability-Based Discovery: In most cases, discovery is based on desired capabilities (data logging, broadband access) rather than specific MAC addresses.
   

----


----
### Proxy, MESH & Discovery Commands


