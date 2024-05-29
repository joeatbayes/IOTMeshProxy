/*  imp_msg.h

Base class for message processors
subclasses of imp_msg will provide
message specific processing

  # Must be imported after IOTMeshProxy.h
*/

#include "IoTMeshProxy.h"
//#include <unordered_map>

#ifndef IMP_MSG_HAND_H
  #define IMP_MSG_HAND_H

struct parsedMsg {
   short appId;
   short targId;
   short msgType;
   short msgId;
   char  *body;
};

//std::unordered_map<std::string, std::string> u =
//    {
//        {"RED", "#FF0000"},
//        {"GREEN", "#00FF00"},
//        {"BLUE", "#0000FF"}
//    };
    

class ImpMsg {
   
   public: 
      char *msgTypeStr() {

      }


      virtual int proc_message(IoTMeshProxy *proxy,  parsedMsg msg) {
      
      }


   private:    

};



class ImpMsgPair : public ImpMsg {
    
    virtual int proc_message(IoTMeshProxy *proxy,  parsedMsg msg) {
    }

};



#endif
