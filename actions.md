

# ACTIONS:
- TODO: Negotiate WAKUP interval so clients and servers can go into
  deep sleep but still be avaialble to recieve next message. Needs to
  allow mechanism to stay awake or wake up often to resync time if 
  initial connection fails. 


* Consider use of proto buf to allow clients in other languages easier parsing.
  First pass look I think it is better to keep the simple serialized formalized
  semantic currently using as it is trivial to parse in any language and 
  human readable when needed.  [version embeddable in arduino](https://github.com/nanopb/nanopb/tree/master/examples/simple)

