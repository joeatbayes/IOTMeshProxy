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
#include <esp_crc.h>
#include "keys.h"
#include "util.h"
#include "math.h"
#ifdef ESP32CRC
  #include "crc.h"
#else
  #include "CRC.h" // see: https://github.com/RobTillaart/CRC/
  #include "CRC16.h"
#endif


// SEE: https://github.com/espressif/esp-idf/blob/4523f2d6/components/esp_wifi/include/esp_now.h

//esp_now_peer_info_t broadcaster = {};
//uint8_t broadcastAddress[] = {0xFF, 0XFF, 0XFF, 0xFF, 0xFF, 0xFF};
#define PAIR_BUTTON 0
#define PAIR_FOR_MS 30000
#define MSGPREF 01
#define COMMAND_PAIR 001
#define MAXHEX3 4096
#define MAXHEX4 65535
class IoTMeshProxy
{

public:
    // Static Variables
    static inline esp_now_peer_info_t Broadcaster = {};
    static inline uint8_t BroadcastAddress[] = {0xFF, 0XFF, 0XFF, 0xFF, 0xFF, 0xFF};
    static inline IoTMeshProxy *FirstHandler = NULL;
    static inline int CHANNEL = 1;
    static inline char *SBuff = NULL;
    static inline char DefaultConfigName[] = "IoTMesh.cfg";
    static inline bool isStaticInitComplete= false;
    static const short MAX_MSG_LEN=250;
    static const short CHAR_USED_BY_HEADER=13;
    static const short CHAR_USED_BY_CRC=4;
    static const short CHAR_USED_BY_OVERHEAD= CHAR_USED_BY_HEADER + CHAR_USED_BY_CRC;
    static const short MAX_PAYLOAD_LEN= MAX_MSG_LEN - CHAR_USED_BY_OVERHEAD;
    static char myMAC[16];

    // Instance Variables
    uint64_t currOnBoardRNum = random_in_range(1, UINT64_MAX);
    unsigned long long privKey = 0;
    unsigned long long pubKey = 0;
    unsigned long long sharedKey = 0;
    unsigned long pair_for_ms = PAIR_FOR_MS;
    short appId = 0;
    unsigned long lastPairStarted = 0;
    unsigned long lastPairSent = 0;
    IoTMeshProxy *nextHandler = NULL;
    

    /* Default constructor */
    IoTMeshProxy()
    {
        if (!isStaticInitComplete) {
            // Moved here because we want to delay calling it
            // until after Serial.begin() has been completed
            // in setup():
            InitESPNow();
            Serial.println("InitESPNow is complete()");
        }
        privKey = genPrivateKey();
        pubKey = genPublicKey(privKey);
        IoTMeshProxy::RegisterHandler(this);
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

    #define MP_PAYLOAD_TOO_LONG 87

    static const char *EspNowError(int errNum)
    {
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
        case MP_PAYLOAD_TOO_LONG:
            return "PAYLOAD TOO LONG";
        // case  ESP_ERR_ESPNOW_CHAN:
        //   return "ESP_ERR_ESPNOW_CHAN";
        default:
            return "ESP UNKNOWN ERROR";
        } // switch
    } // func

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
    }

    // formats message and sends it to the target MAC ID.
    // returns ESP_IK or one of errors ESP_ERR values
    int sendMsg(uint8_t mac, short appId, short destId, short msgType, short msgId, char *data, int dtaSize) {
        char buff[MAX_MSG_LEN+1];
        if (dtaSize > MAX_PAYLOAD_LEN) {
            return MP_PAYLOAD_TOO_LONG;
        }
        dtaSize = max(0,min(dtaSize, MAX_PAYLOAD_LEN));
        data[dtaSize] = 0; // limit our buffer size sent. 
        int blen=sprintf(buff, "%3x%4x%3x%3x%s",
            min(appId,MAXHEX3), min(destId,MAXHEX4), min(msgType,MAXHEX3), 
            min(msgId,MAXHEX4), data);
        #ifdef ESP32CRC
          uint16_t crc16 = crc16_le(0, (const uint8_t *) buff, blen);
        #else
          uint16_t crc16 = calcCRC16((const uint8_t  *) buff, blen);
        #endif
        sprintf(buff+blen, "%4x", crc16);
        
    }

    static void SendBroadcast(const uint8_t *msg, int size)
    {
        Serial.println("sending broadcast");
        esp_err_t result = esp_now_send(BroadcastAddress, (const uint8_t *)msg, size);
        if (result != ESP_OK)
        {
            const char *errStr = EspNowError(result);
            Serial.printf("failed send res=%d %s\n", result, errStr);
        }        
    }

    static bool IsCommandStr(const uint8_t *data, int dataLen)
    {
        if (dataLen < 6)
        {
            return false;
        }
        if ((data[4] == ':') && (isxdigit(data[0])) && (isxdigit(data[1])) && (isxdigit(data[2])))
        {
            return true;
        }
        return false;
    } // func


    // return the first handler that matches the
    // requested app id or return NULL if no matching
    // handler is found.
    static IoTMeshProxy *GetHandler(short appId)
    {
        IoTMeshProxy *handler = FirstHandler;
        while (true)
        {
            if (handler == NULL)
            {
                return NULL;
            }

            if (handler->appId == appId)
            {
                return handler;
            }

            if (handler->nextHandler == NULL)
            {
                return NULL;
            }
            handler = handler->nextHandler;
        }
    } // func

    void processCommand(short appId, short targId, short msgType, short msgId, uint8_t *body)
    {
    }

    static void ProcessCommand(const uint8_t *macAddr, uint8_t *data, int dataLen)
    {
        short appId;   // To store ID (3 hex digits + null terminator)
        short msgType; // To store command (3 hex digits + null terminator)
        short targId;
        short msgId;
        long crc;
        char buff[16];
        char *body;
        const char *sdp = (const char *)data;
        const char *dp = (const char *)buff;
        strncpy(buff, sdp, 3);
        appId = (short)strtoul(dp, NULL, 16);
        strncpy(buff, sdp + 3, 4);
        targId = (short)strtoul(dp, NULL, 16);
        strncpy(buff, sdp + 7, 3);
        msgType = (short)strtoul(dp, NULL, 16);
        strncpy(buff + 10, sdp + 10, 3);
        msgId = (short)strtoul(dp, NULL, 16);
        //  Pull last 8 bytes and compare to reste of message using
        //  hardware CRC.
        #ifdef ESP32CRC
          crc = (long)strtol(sdp + (dataLen - 8), NULL, 16);
        #else
          crc = calcCRC16((uint8_t *)buff, dataLen - 4);
        #endif

        data[dataLen - 9] = 0;
        body = (char *)data - 8;

        // TODO: Compute CRC for all byes upto CRC.
        // if it fails reject it here

        IoTMeshProxy *handler = GetHandler(appId);
        if (handler != NULL)
        {
            //  Since we only have a single ESP32 Now onDataRecv
            //  once we recognize that we have a command we look up
            //  the class instance that will be processing it send the
            //  command there. Add command dispatch to proper handlers to
            // those instances.   Have not made a decision about wether
            // we have a separate handler by macAddr or not.
            // create a structure with all this data and pass it.
            handler->processCommand(appId, targId, msgType, msgId, (uint8_t *)data);
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
    } // func

    // Method called by Called by ESPNOW when messages are received.
    // This must be a static method due to the ESPNow Design
    static void OnDataRecv(const uint8_t *macAddr, const uint8_t *data, int dataLen)
    {
        Serial.println("OnDataRecv");
        char macStr[18];
        char dbuff[ESP_NOW_MAX_DATA_LEN + 1];
        formatMac((char *)macStr, macAddr);
        memcpy(dbuff, data, dataLen);
        dbuff[dataLen] = 0;
        if (IsCommandStr(data, dataLen))
        {
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
        esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
        Serial.print("STA MAC: ");
        myMAC = (char *) WiFi.macAddress().c_str();
        Serial.println();
        Serial.print("STA CHANNEL ");
        Serial.println(WiFi.channel());
        pinMode(PAIR_BUTTON, INPUT_PULLUP);

        formatMac((char *)SBuff, BroadcastAddress);
        Serial.printf("broadcast MAC=%s ", SBuff);

        Serial.print("AP MAC: ");
        Serial.println(WiFi.softAPmacAddress());
        Serial.println("L81: before memcpy");

        // Load our Broadcast address into our default
        // broadaster. All instances use the same broadcaster
        memcpy(&Broadcaster.peer_addr, BroadcastAddress, 6);

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
        if (!esp_now_is_peer_exist(BroadcastAddress))
        {
            int stat = esp_now_add_peer(&Broadcaster);
            if (stat != ESP_OK)
            {
                Serial.printf("L278: failed broadcaster add res=%d %s\n", stat, EspNowError(stat));
            } else {
                Serial.println("L288: Added broadcaster as peer");
            }
        }
        // WiFi.softAPdisconnect(true);
        // WiFi.mode(WIFI_STA);
        isStaticInitComplete=true;
    } // func

    // register a handler instance of a proxy to process messages
    // targeted to a matching App ID. Handlers are treated as a
    // single list where the static method points at first registered
    // handler and each one points at the next one registered until
    // we hit a null and know that we have hit the end.
    static void RegisterHandler(IoTMeshProxy *proxy)
    {
        if (FirstHandler == NULL)
        {
            FirstHandler = proxy;
        }
        else
        {
            IoTMeshProxy *handler = FirstHandler;
            while (handler->nextHandler != NULL)
            {
                handler = handler->nextHandler;
            }
            handler->nextHandler = proxy;
        }
    } // func

    void service()
    {
        if (this->isInPairMode())
        {
            if (elapMs(this->lastPairSent) > 5000)            
            {   // don't know the other guys key yet 
                // unit65 initKey = makeInitKey(KEYS_KEY1, myMAC, myMAC, currOnBoardRNum);
                Serial.printf("send pair request\n");
                // Sends the pairing request with my current random number 
                // which we will use to compute the key for the initial 
                // connection in the Reponse record.
                sprintf((char *)SBuff, "%03X:%03X:%017llX", MSGPREF, COMMAND_PAIR, currOnBoardRNum);
                Serial.println("after sprintf");
                SendBroadcast((const uint8_t *)SBuff, strlen((char *)SBuff));
                this->lastPairSent = millis();
            }
        }
    }

    void loadConfig(char *fiName)
    {
    }

    void loadConfig()
    {
        loadConfig(DefaultConfigName);
    }

}; // class

#endif
