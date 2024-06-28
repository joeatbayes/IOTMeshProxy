/*  imp_msg.h

Base class for message processors
subclasses of imp_msg will provide
message specific processing

  # Must be imported after IOTMeshProxy.h
*/


//#include <unordered_map>

#ifndef IoTMeshProxy
  class IoTMeshProxy;
#endif 

#ifndef IMP_MSG_HAND_H
  #define IMP_MSG_HAND_H

#include "imp_parsed_msg.h"
#include "IoTMeshProxy.h"

//std::unordered_map<std::string, std::string> u =
//    {
//        {"RED", "#FF0000"},
//        {"GREEN", "#00FF00"},
//        {"BLUE", "#0000FF"}
//    };
    

class ImpMsgHand {
   public: 
      virtual short msgType() {
        return 0x01;
       }

      virtual const char *msgTypeStr() {
        return "ABSTRACT";
      }


      virtual int processMessage(IoTMeshProxy *proxy,  ImpParsedMsg *msg) {
        return 0;
      
      }


   private:    

};



class ImpMsgPair : public ImpMsgHand {
  public:
    virtual short msgType() {
        return 0x01;
    }

    virtual const char *msgTypeStr() {
        return "pair";
    }
    
    virtual int proc_message(IoTMeshProxy *imp,  ImpParsedMsg *msg) {
      Serial.printf("proc_message ImpMsgPair");
      char buff[80];
      int nb = snprintf(buff, 79, "%4x", msg->msgId);
      imp->connectWithSharedPassword(msg->mac);
      imp->sendMsg((uint8_t *) msg->mac, (int) msg->appId, (int) msg->targId, 
          (int) IMP_MTYPE::PAIR_REQ, (int) imp->nextMsgId(), buff, nb);
      return 0;
    }

};




#endif
