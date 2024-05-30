# Handshaking - Basic process flow for Node to Node negotiations


How to determine it is time for a given node to connect to 
others since each wakeup event costs power. 

### Handshake for new Nodes
- User Action places more than 1 node in pairing MODE.
- Each Node Sends Broadcast "PAIRM" with MAC
- Responding Node Sends - PAIR-REQ using preshared
  AES Key.  
    - Once a Node identifies the peer 
      it wants to add it automatically adds that MAC
      to peer using encrypted channel.  
    - NOTE: This key to be replaced with diffie 
      hellman negotiated key once we figure out 
      how to use the RSA modular exponenation feature
      built into the hardware.

-  Peer reciever of PARM will send PAIR-RAND 
   which includes a large random number.  Both 
   peers change encryption key for their 
   communcation channel. 

-  Both peers save the Agreed key plus MAC in 
   preferences 

-  Both Nodes send PAIR-TIMING message including
   intended sleep verus wake intervals which is 
   also saved in Permenant storage.

### Handshake for previously paired Nodes When 1 node has preferred upload path
More than one node wakes up at same time.
Node determines it has a message to send or deliver


### Handshake for Previously Paired Nodes
- More than one nodes wake up at same time. 
- Starts a wakeup / last action event
- Each Node sends NODE-AVAIL broadcast
  - This is required because even if a given node does 
    not have a message pending to send one of it's peers
    may have data available.
  - Broadcast is used to reduce power required. and allow
    other peers to avoid the need to add peers for every 
    allowed pair mode until we know that peer is availble.
- Receiving Node: 
  - Checks to see if this node is in the approved 
    peer list for direct connect peers.
  - Sends NODE-AVAIL-ACK 
    - This is needed because if we don't get an an ack
      we may have lost time synchronization or upstream node
      may have been removed.  If time synch is lost then this
      node may need to wake up more often and stay awake for 
      longer until it can resync time. 
  - If either Node has messages pending for other
    both add the other using the existing peer / pair 
    encryption key.   Then sends NODE-CONNECT
    message to other node every 500ms until receive
    NODE-CONNECT-ACK.  This is not necessary if data 
    flag in Node-AVAIL-ACK indicated a conneciton is needed.    
  
  - Start sending messages pending. 

- Each node stays awake until all messages 
  have been exchanged and for a minimum 
  wake interval. 

- How to allow new nodes to inherit previous pairing so nodes
  can be replaced?

- How to allow new nodes to inherit a limited set up upstream
  pairing?
  
- NOTE: Preshared key is configured as a preference to allow easy change
  of this key. to create different clusters. 