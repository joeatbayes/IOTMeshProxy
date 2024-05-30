
#ifndef IMP_PARSED_MSG_H
  #define IMP_PARSED_MSG_H
  

//#include <ctype.h>
//#include <stdlib.h>
//#include <string.h>
#include "util.h"


struct ImpParsedMsg {
   uint8_t mac[6];
   short appId;
   short targId;
   short msgType;
   short msgId;
   uint8_t *body;
   short bsize;

   ImpParsedMsg(const uint8_t *pMAC,  short pappId, short ptargId,  short pmsgType, 
    short pmsgId, uint8_t *pbody, short pbsize) : appId(pappId), targId(ptargId),
    msgType(pmsgType), msgId(pmsgId), bsize(pbsize), body(pbody)
    {
      memcpy(mac,pMAC,6);            
      // NOTE: Decided to memory allocation and copy here to allow
      // queueing and latter processing of messages.
      body = (uint8_t *) malloc(pbsize+1);         
      memcpy(body,pbody,pbsize);
      body[pbsize]=0;
    }

    int toStr(char *buff, int maxSize) {
        int nb = snprintf(buff, maxSize -1, "%x:%x:%x:%x:%x%x app:%x,targ:%x,mtype:%x,mid:%x,blen:%x,dta:%s",
           mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],
           appId, targId, msgType,  msgId, bsize, body);
        buff[maxSize]=0;
        buff[nb]=0;      
        return nb;  
    }

    ~ImpParsedMsg() {
        if (body != NULL) {
            free(body);
            body = NULL;
        }
    }

};


#endif
