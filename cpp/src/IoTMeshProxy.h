/* IoTMeshProxy.h
 *
 *    Enable Proxy and mesh command message delivery to any node within the
 *    network, even if it requires traversing multiple intermediate nodes
 *    due to range limitations.
 *
 *    See: ../readme.md for protocol and design overview.
 *
 *   (C) Joseph Ellsworth 2024 All Rights Reserved
 *   See License.txt
 *
 */
#ifndef IoTMeshProxyH
#define IoTMeshProxyH

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h> // only for esp_wifi_set_channel()
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "keys.h"
#include "util.h"
#include "math.h"
#include "queue.h"
#include "list.h"
#include "imp_parsed_msg.h"
#include "imp_msg_hand.h"

const char * const IMP_ERR_STR[] = {
    "MSG_TYPE_OUT_RANGE",
    "MSG_TYPE_NO_HAND"
};

enum class IMP_ERROR {
  MTYPE_OUT_RANGE,
  MTYPE_NO_HAND
};

enum class IMP_MTYPE {  
  PAIRM = 1,
  PAIR_REQ = 2,
  PAIR_RAND = 3,
  PAIR_RAND_ACK = 4,
  PAIR_TIMING = 5,
  NODE_AVAIL =  6,
  NODE_AVAIL_ACK = 7,  
  INVALID = 8,
  NOT_AVAIL = 9,
  UART = 10,
  UART_FULL=11,
  UART_ACK = 12,
  REQ_TEMP = 13,
  READ_REG = 14,
  READ_RESP = 15,
  WRITE_REG = 16,
  WRITE_RESP= 17,
  CHUNK_START = 20,
  CHUNK = 21,
  CHUNK_ACK = 22,
  CHUNK_FULL = 23,
  CHUNK_ERR = 24    
};

// SEE: https://github.com/espressif/esp-idf/blob/4523f2d6/components/esp_wifi/include/esp_now.h
#ifndef ImpMsgHand
class ImpMsgHand ;
#endif


#ifndef IoTMeshProxy
class IoTMeshProxy;
#endif

esp_now_peer_info_t IMP_Broadcaster = {};
const uint8_t IMP_BroadcastAddress[] = {0xFF, 0XFF, 0XFF, 0xFF, 0xFF, 0xFF};
int IMP_CHANNEL = 1;
const int IMP_DEF_APP_ID=1;
#ifndef IMP_APP_ID
  const int IMP_APP_ID = IMP_DEF_APP_ID;
#endif
const int IMP_BROAD_TARG_ID=0XFFFF;
const short IMP_MAX_MSG_LEN = 250;
const short IMP_APP_ID_LEN = 3;
const short IMP_DEST_ID_LEN = 4;
const short IMP_MTYPE_LEN = 3;
const short IMP_MSG_ID_LEN = 4;
const short IMP_CRC_LEN = 4;
const short IMP_APP_ID_START  = 0;
const short IMP_DEST_ID_START = IMP_APP_ID_START + IMP_APP_ID_LEN;
const short IMP_MTYPE_START   = IMP_DEST_ID_START + IMP_DEST_ID_LEN;
const short IMP_MSG_ID_START  = IMP_MTYPE_START   + IMP_MTYPE_LEN;
const short IMP_PAYLOAD_START = IMP_MSG_ID_START  +  IMP_MSG_ID_LEN;
const short IMP_CHAR_USED_BY_HEADER = (
    IMP_APP_ID_LEN + IMP_DEST_ID_LEN + IMP_MTYPE_LEN
    + IMP_MSG_ID_LEN);
const short IMP_CHAR_USED_BY_OVERHEAD = IMP_CHAR_USED_BY_HEADER + IMP_CRC_LEN;
const short IMP_MAX_PAYLOAD_LEN = IMP_MAX_MSG_LEN - IMP_CHAR_USED_BY_OVERHEAD;
const char  IMP_DefaultConfigName[] = "IoTMesh.cfg";
const int IMP_MAX_MSG_ID=0xFFFF;
const int IMP_MAX_DEST_ID = 0xFFFF;
const short IMP_MAX_NUM_MSG_HAND=50;

char *IMP_SBuff=NULL;
IoTMeshProxy *IMP_FirstAppHandler=NULL;  
// App handler is a the specific instance of the IoTMeshProxy which is processing
// the requests as first layer after after the static callback handeling. 
// It should not be confused with messageHandler where we have a separate instance
// of a much smaller class used to handle different kinds of messages indexed 
// by message type.   
bool IMP_isStaticInitComplete=false;
uint8_t IMP_MAC[6];


//esp_now_peer_info_t IMP_Broadcaster = {};
//uint8_tIMP_BroadcastAddress[] = {0xFF, 0XFF, 0XFF, 0xFF, 0xFF, 0xFF};
#define PAIR_BUTTON 0
#define PAIR_FOR_MS 30000
#define MSGPREF 01
#define COMMAND_PAIR 001
#define MAXHEX3 0xFFF
#define MAXHEX4 0xFFFF


int calcCRCFromFullBuff(const uint8_t *data, int size) {
        int nonCRCLen = size - IMP_CRC_LEN;
        Serial.printf("284:CRC BUFF size=%d nonCRCLen=%d dta=", size, nonCRCLen);
        delay(50);
        return calcCRC(data, nonCRCLen);    
}

#define IMP_PAYLOAD_TOO_LONG 87

const char *EspNowError(int errNum)    {
    switch (errNum)
    {
    case ESP_OK:
        return "OK";
    case ESP_ERR_ESPNOW_BASE:
        return "ESP_ERR_ESPNOW_BASE";
    case ESP_ERR_ESPNOW_NOT_INIT:
        return "ESP_ERR_ESPNOW_NOT_INIT";
    case ESP_ERR_ESPNOW_ARG:
        return "ESP_ERR_ESPNOW_ARG";
    case ESP_ERR_ESPNOW_NO_MEM:
        return "ESP_ERR_ESPNOW_NO_MEM";
    case ESP_ERR_ESPNOW_FULL:
        return "ESP_ERR_ESPNOW_FULL";
    case ESP_ERR_ESPNOW_NOT_FOUND:
        return "ESP_ERR_ESPNOW_NOT_FOUND";
    case ESP_ERR_ESPNOW_INTERNAL:
        return "ESP_ERR_ESPNOW_INTERNAL";
    case ESP_ERR_ESPNOW_EXIST:
        return "ESP_ERR_ESPNOW_EXIST";
    case ESP_ERR_ESPNOW_IF:
        return "ESP_ERR_ESPNOW_IF";
    case IMP_PAYLOAD_TOO_LONG:
        return "PAYLOAD TOO LONG";
    // case  ESP_ERR_ESPNOW_CHAN:
    //   return "ESP_ERR_ESPNOW_CHAN";
    default:
        return "ESP UNKNOWN ERROR";
    } // switch
} // func



class IoTMeshProxy
{

public:
 
    // Instance Variables
    uint64_t currOnBoardRNum = random_in_range(1, UINT64_MAX);
    unsigned long long privKey = 0;
    uint64_t pubKey = 0;
    unsigned long long sharedKey = 0;
    unsigned long pair_for_ms = PAIR_FOR_MS;
    int appId = IMP_APP_ID;
    unsigned long lastPairStarted = 0;
    unsigned long lastPairSent = 0;
    IoTMeshProxy *nextHandler = NULL;
    short currMsgId=0;
    ImpMsgHand *msgHandlers[IMP_MAX_NUM_MSG_HAND];
    // Pointer to the array of message handlers we will use 
    // for each application type.  In this version will be simple 
    // numberic index but may be better treated as index of hash
    // table. Could consider a linked list and just scan it for 
    // the correct handler. 


    /* Default constructor */
    IoTMeshProxy()
    {
        if (!IMP_isStaticInitComplete) {
            // Moved here because we want to delay calling it
            // until after Serial.begin() has been completed
            // in setup():
            InitESPNow();
            Serial.println("InitESPNow is complete()");

            for (int ndx=0; ndx<IMP_MAX_NUM_MSG_HAND; ndx++) {
              msgHandlers[ndx]=NULL;
            } 
        }
        //privKey = genPrivateKey();
        //pubKey = genPublicKey(privKey);
        IoTMeshProxy::RegisterAppHandler(this);        
        pubKey = makeInitKey(KEYS_KEY1, IMP_MAC,IMP_MAC);
        RegisterMessageHandlers(this);
    }

    bool isInPairMode()
    {
        if (lastPairStarted == 0)
        {
            return false;
        }
        else if (elapMs(lastPairStarted) < PAIR_FOR_MS)
        {
            return true;
        }
        else
        {
            return false;
        }
    } // func

   int connectWithSharedPassword(uint8_t *mac) {
      uint64_t pass = IMP_Keys::getSharedPassword();
      if (!esp_now_is_peer_exist(IMP_BroadcastAddress))
        {
            int stat = esp_now_add_peer(&IMP_Broadcaster);
            if (stat != ESP_OK)
            {
                Serial.printf("L278: failed IMP_Broadcaster add res=%d %s\n", stat, EspNowError(stat));
            } else {
                Serial.println("L288: Added IMP_Broadcaster as peer");
            }
        }

   }

   int connectWithNegotiatedPasswrod(uint8_t *mac) {

   }

   int connectWithNoPassword(uint8_t *mac) {

   }

   int disconnectPeer(uint8_t *mac) {

   }

   int forgetNegotiatedPassword(uint8_t *mac) {

   }




    // formats message and sends it to target MAC Id.   Returns any
    // error 

    int sendMsg(uint8_t *mac, const uint8_t  *msg, int dsize) {
       // Lookup the peer to see if we already have 
       // security establish if so see if we already 
       // have a peer esablished. If we do have peer 
       // established check to ensure it is using encrypted key
       // if we have one if not modify the conneciton to use 
       // the encrypted key oterhwise need to send the connect
       // message if not already connected. 
       // Check and add peer if needed and give it a second 
       // before we try again.   
       // Need to add to the recent send ring buffer 
       // so we can resend as needed.   Also need to add to the 
       // pending ACK queue before trying to send another 
       // message. so we can keep resending if not acked. 

       esp_err_t result = esp_now_send(mac, (const uint8_t *)msg, dsize);
       if (result != ESP_OK) {       
            const char *errStr = EspNowError(result);
            Serial.printf("failed send res=%d %s\n", result, errStr);
        }        
      return result;
    }

    // formats message and sends it to the target MAC ID.
    // returns ESP_IK or one of errors ESP_ERR values
    int sendMsg(uint8_t *mac, int appId, int destId, int msgType, int msgId, 
      char *data,  short dtaSize) {        
        char buff[IMP_MAX_MSG_LEN+1];
        if (dtaSize > IMP_MAX_PAYLOAD_LEN) {
            return IMP_PAYLOAD_TOO_LONG;
        }
        //Serial.printf("app=%d ", appId);
        Serial.printf("mac=%02x:%02x:%02x:%02x:%02x:%02x app=%03x dest=%04x type=%03x msg#=%04x dtaSize=%d\n", 
           mac[0],mac[1],mac[2],mac[3],mac[4],mac[5], appId, destId, msgType, msgId, dtaSize);
        dtaSize = max(0,min(dtaSize, IMP_MAX_PAYLOAD_LEN));        
        /*
        int blen=sprintf(buff, "%3x%4x%3x%3x",
            min(appId,MAXHEX3), min(destId,MAXHEX4), min(msgType,MAXHEX3), 
            min(msgId,MAXHEX4));
        */

        // WARNING: If changing this format string then you must change
        // the field size and position ordering in processMessage
        int blen=snprintf(buff, IMP_MAX_MSG_LEN, "%03x%04x%03x%04x",
            min(appId,MAXHEX3), min(destId,MAXHEX4), min(msgType,MAXHEX3), 
            min(msgId,MAXHEX4));
        short dataEndNdx = blen + dtaSize;
        char *src = data;        
        for (int ndx=blen; ndx<=dataEndNdx; ndx++){
            buff[ndx] = *src;
            *src++;
        }
        blen += dtaSize;
        buff[blen]=0;

        //Serial.printf("data=%s\n", data);
        //Serial.printf("L206 blen=%d dtaSize=%d\n", blen, dtaSize);
        //Serial.printf("L217 buff=%s\n", buff);
        
        Serial.printf("L206 blen=%d dtaSize=%d buff=%s\n", blen, dtaSize, buff);
        
        #ifdef ESP32CRC
          uint16_t crc16 = esp_crc16_le(0, (const uint8_t *) buff, blen);
        #else
          uint16_t crc16 = calcCRC16((const uint8_t  *) buff, blen);
        #endif
        Serial.printf("L212:crc16=%x\n", crc16);
        sprintf(buff+blen, "%04x", crc16);
        Serial.printf("buff after CRC=%s\n", buff);
        
        esp_err_t result = esp_now_send(mac, (uint8_t *)buff, blen+4);
        if (result != ESP_OK)
        {
            const char *errStr = EspNowError(result);
            Serial.printf("failed send res=%d %s\n", result, errStr);
        }      
        return result;  
        #ifdef XXX
        #endif
        return 0;
    }

    static void SendBroadcast(const uint8_t *msg, int size)
    {
        Serial.println("sending broadcast");
        esp_err_t result = esp_now_send(IMP_BroadcastAddress, (const uint8_t *)msg, size);
        if (result != ESP_OK)
        {
            const char *errStr = EspNowError(result);
            Serial.printf("failed send res=%d %s\n", result, errStr);
        }        
    }

    // extract CRC from end of a string so we can 
    // compare it to the CRC we calcualte locally
    static int extractCRC(const uint8_t *data, int size) {
        char buff[IMP_CRC_LEN + 1];
        short startPos = size - IMP_CRC_LEN;
        strncpy(buff, (const char *)(data+startPos), IMP_CRC_LEN);
        buff[IMP_CRC_LEN] = 0;
        Serial.printf("L269: crcbuff=%s data=%s\n", (char *) buff, (char *) data);
        return (int) strtoul(buff,NULL,16);
    }



    static bool IsCommandStr(const uint8_t *data, int dataLen)
    {
        // see if we have enough characters to represent
        // a valid command. 
        if (dataLen < IMP_CHAR_USED_BY_OVERHEAD)
        {
            return false;
        }

        // check our CRC Area to ensure it contains only
        // valid hex digits
        for (int ndx=0; ndx <= IMP_CHAR_USED_BY_HEADER; ndx++) {
            if (isxdigit(data[0]) == 0) {
                return false;
            }
        }

        // Check our CRC Area to ensure it contains only
        // valid hex digits.
        int crcStart = dataLen - IMP_CRC_LEN;
        for (int ndx=crcStart; ndx<dataLen; ndx++) {
            if (isxdigit(data[0])== 0) {
                return false;
            }
        }
        // we survived the basic checks to so it looks
        // like a potentially valid command. 
        return true;
    } // func


    // return the first handler that matches the
    // requested app id or return NULL if no matching
    // handler is found.
    static IoTMeshProxy *GetAppHandler(short appId)
    {
        IoTMeshProxy *appHandler = IMP_FirstAppHandler;
        while (true)
        {
            if (appHandler == NULL)
            {
                return NULL;
            }

            if (appHandler->appId == appId)
            {
                return appHandler;
            }

            if (appHandler->nextHandler == NULL)
            {
                return NULL;
            }
            appHandler = appHandler->nextHandler;
        }
    } // func


    void sendError(ImpParsedMsg *msg, int errCode, const char *errStr) {
       char buff[IMP_MAX_MSG_LEN+1];
       buff[IMP_MAX_MSG_LEN]=0;
       char *b;
       short nb = msg->toStr(buff, IMP_MAX_MSG_LEN);
       snprintf(b+nb, IMP_MAX_MSG_LEN-nb, " err=%d, errStr=%x", errCode, errStr);
       
    }


    void processCommand(ImpParsedMsg *msg)
    {
        Serial.println("IN instance level command processor");
        if ((msg->msgType < 0) || (msg->msgType >= IMP_MAX_NUM_MSG_HAND)) {
          sendError(msg, (int) IMP_ERROR::MTYPE_OUT_RANGE, "");
        } else {

        }
        ImpMsgHand *mpro = msgHandlers[msg->msgType];
        if (mpro == NULL ) {
          sendError(msg, (int) IMP_ERROR::MTYPE_NO_HAND, "");
        }
        mpro->processMessage(this, msg);        
    }

  
    // Static level command 
    static void ProcessCommand(const uint8_t *macAddr, uint8_t *data, int dataLen)
    {        
        long crcExt;
        char buff[IMP_MAX_MSG_LEN+1];
        char *body;
        const char *sdp = (const char *)data;
        const char *dp = (const char *)buff;        
        int crcOffset = dataLen - IMP_CRC_LEN;        
        //Serial.printf("L372: dataLen=%d data=%s\n", dataLen, data);
        //delay(50);
        crcExt = extractCRC(data, dataLen);                
        //Serial.printf("L383: CRC Extracted=%x\n", crcExt);
        //delay(50);
        int crcCalc = calcCRCFromFullBuff((const uint8_t *) data, dataLen);
        //Serial.printf("L386: CRCCalc=%x\n", crcCalc);        
        if (crcCalc != crcExt) {
            Serial.printf("CRC validation failed CRC=%x Calced=%x\n", crcExt, crcCalc);
            delay(50);
            Serial.printf("buff=%s\n", buff);
            delay(50);
            // TODO: Send Add response indicating CRC FAILURE
        }

        int appId = getHexInt(data, 0, IMP_APP_ID_LEN);
        int destId= getHexInt(data, IMP_DEST_ID_START, IMP_DEST_ID_LEN);
        int msgType =getHexInt(data, IMP_MTYPE_START,   IMP_MTYPE_LEN);
        int msgId = getHexInt(data, IMP_MSG_ID_START, IMP_MSG_ID_LEN);
        Serial.printf("app=%x, destId=%x, mtype=%x mid=%d, crc=%04x\n",
          appId, destId, msgType, msgId, crcExt);
        delay(50);
        
        // Get a pointer to our body string. 
        int bodyLen = crcOffset - IMP_PAYLOAD_START;
        strncpy(buff,sdp + IMP_PAYLOAD_START, bodyLen);
        
        ImpParsedMsg pmsg(macAddr, (short) appId, (short) destId, (short) msgType, 
          (short) msgId,  data, bodyLen);
        IoTMeshProxy *handler = GetAppHandler(appId);
        if (handler != NULL)
        {
            //  Since we only have a single ESP32 Now onDataRecv
            //  once we recognize that we have a command we look up
            //  the class instance that will be processing it send the
            //  command there. Add command dispatch to proper handlers to
            // those instances.   
            handler->processCommand(&pmsg);
        }

    } // func



    // Method called by ESPNow whenever a message is sucessfully
    // sent.  This must be a static method due to the ESPNow design
    static void OnDataSent(const uint8_t *macAddr, esp_now_send_status_t status)
    {
        Serial.println("onDataSent");
        char macStr[18];
        formatMac((char *)macStr, macAddr);
        Serial.printf("last Sent mac=%s status=%d\n", (uint8_t *)macStr, status);
        delay(50);
    } // func

    // Method called by Called by ESPNOW when messages are received.
    // This must be a static method due to the ESPNow Design
    static void OnDataRecv(const uint8_t *macAddr, const uint8_t *dataIn, int dataLen)
    {        
        Serial.printf("L431: OnDataRecv dLen=%d  \n", dataLen);        
        char macStr[18];
        uint8_t dbuff[ESP_NOW_MAX_DATA_LEN + 1];
        dataLen = min(dataLen, ESP_NOW_MAX_DATA_LEN);
        formatMac((char *)macStr, macAddr);
        memcpy(dbuff, dataIn, dataLen);
        dbuff[dataLen] = 0;
        Serial.printf("L437: dlen=%d dbuff=%s\n", dataLen, (char *) dbuff);
        Serial.print(" check all xdig: ");
        //for (int i=0; i<dataLen; i++) {
        //    Serial.printf(" i=%d c=%c isXDigit=%d\n", i, (char) dbuff[i], (int) isxdigit(dbuff[i]) );
        //    delay(100);
        //}

        if (IsCommandStr(dbuff, dataLen))
        {
            Serial.println("found a command string");
            ProcessCommand(macAddr, (uint8_t *)dbuff, dataLen);
        }
        else
        {
            Serial.printf("NON COMMAND: MAC=%d DATA=%s\n", macStr, (char *)dbuff);
            char buff[ESP_NOW_MAX_DATA_LEN + 1];
        }
    } // func

    static void InitESPNow()
    {
        WiFi.mode(WIFI_STA);
        esp_wifi_set_channel(IMP_CHANNEL, WIFI_SECOND_CHAN_NONE);
        Serial.print("STA MAC: ");
        esp_read_mac(IMP_MAC, ESP_MAC_WIFI_STA );        
        Serial.println();
        Serial.print("STA CHANNEL ");
        Serial.println(WiFi.channel());
        pinMode(PAIR_BUTTON, INPUT_PULLUP);

        formatMac((char *)IMP_SBuff,IMP_BroadcastAddress);
        Serial.printf("broadcast MAC=%s ", IMP_SBuff);

        Serial.print("AP MAC: ");
        Serial.println(WiFi.softAPmacAddress());
        Serial.println("L81: before memcpy");

        // Load our Broadcast address into our default
        // broadaster. All instances use the same IMP_Broadcaster
        memcpy(&IMP_Broadcaster.peer_addr,IMP_BroadcastAddress, 6);

        WiFi.disconnect();
        if (esp_now_init() == ESP_OK)
        {
            Serial.println("ESPNow Init Success");
        }
        else
        {
            Serial.println("ESPNow Init Failed - Restarting");
            delay(5000);
            ESP.restart();
        }
        Serial.println("after InitESPNow");
        esp_now_register_recv_cb(OnDataRecv);
        Serial.println("after onRec register");
        esp_now_register_send_cb(OnDataSent);
        Serial.println("after onSend register");
        if (!esp_now_is_peer_exist(IMP_BroadcastAddress))
        {
            int stat = esp_now_add_peer(&IMP_Broadcaster);
            if (stat != ESP_OK)
            {
                Serial.printf("L278: failed IMP_Broadcaster add res=%d %s\n", stat, EspNowError(stat));
            } else {
                Serial.println("L288: Added IMP_Broadcaster as peer");
            }
        }
        // WiFi.softAPdisconnect(true);
        // WiFi.mode(WIFI_STA);
        IMP_isStaticInitComplete=true;
    } // func

    // App Handler is an instance of the msg Proxy class which 
    // most direclty manages the connections to underlying communicaiton
    // class.   Most of the actual message processing work is proxied
    // to the message processors.
    // register a handler instance of a proxy to process messages
    // targeted to a matching App ID. Handlers are treated as a
    // single list where the static method points at first registered
    // handler and each one points at the next one registered until
    // we hit a null and know that we have hit the end.
    static void RegisterAppHandler(IoTMeshProxy *proxy)
    {
        if (IMP_FirstAppHandler == NULL)
        {
            IMP_FirstAppHandler = proxy;
        }
        else
        {
            IoTMeshProxy *handler = IMP_FirstAppHandler;
            while (handler->nextHandler != NULL)
            {
                handler = handler->nextHandler;
            }
            handler->nextHandler = proxy;
        }
    } // func


    int nextMsgId() {
        currMsgId += 1;
        if (currMsgId >= IMP_MAX_MSG_ID) {
            currMsgId = 0;
        }
        return currMsgId;
    }

    void service() {
       //Serial.print("L465: in service()");
        bool isPairing = this->isInPairMode();
        //Serial.printf("L458: isPairing=%d\n", isPairing);
        if (isPairing) {
            int sinceLastPair = elapMs(this->lastPairSent);
            //Serial.printf("L461: sinceLastPair=%ld\n", sinceLastPair);
            if ( sinceLastPair > 5000) {   // don't know the other guys key yet 
                // unit65 initKey = makeInitKey(KEYS_KEY1, myMAC, myMAC, currOnBoardRNum);
                Serial.printf("L481: send pair request\n");
                // Sends the pairing request with my current random number 
                // which we will use to compute the key for the initial 
                // connection in the Reponse record.
                Serial.printf("L485: appId=%d\n", appId);
                Serial.flush();
                sprintf((char *)IMP_SBuff, "%03X:%03X:%017llX ...", MSGPREF, 0, COMMAND_PAIR, currOnBoardRNum);
                Serial.println("L487: after sprintf");
                Serial.flush();
                //uint64_t key = makeInitKey(KEYS_KEY1, IMP_MAC, IMP_MAC);
                //char keyBuff[17];
                //formatKey64(keyBuff, key);
                
                Serial.println("L492: before SendBroadcast");
                Serial.flush();
                //        int sendMsg(uint8_t *mac,                  short appId, short destId,              short msgType,       uint16_t msgId, char *data, short dtaSize) 
                SendBroadcast((const uint8_t *)IMP_SBuff, strlen((char *)IMP_SBuff));
                Serial.println("L495: after broadcast");
                Serial.flush();
                int sres = sendMsg((uint8_t *) IMP_BroadcastAddress, appId,      (int) IMP_BROAD_TARG_ID, (int) COMMAND_PAIR, nextMsgId(),    (char *)"PAIR",        (int)   4);  
                Serial.println("L497: after sendMsg");
                Serial.flush();
                this->lastPairSent = millis();
            }
        }
    }

//const short IMP_APP_ID=1;
//const short IMP_BROAD_TARG_ID=UINT16_MAX;

    void loadConfig(char *fiName)
    {
    }


    void loadConfig()
    {
        loadConfig((char *) IMP_DefaultConfigName);
    }


    static void RegisterMessageHandlers(IoTMeshProxy *mph) {
       mph->msgHandlers[1] = (ImpMsgHand *) new ImpMsgPair();       
       // TODO: Add instances for default message handlers
       // here. 

    }

}; // class

#endif
