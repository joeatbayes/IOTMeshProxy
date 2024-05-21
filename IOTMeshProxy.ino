/* espnow_broadcast_serial_bridge
*
*  See: readme.txt for protocol design and 
*   detailed design notes
*
*  
*/

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h> // only for esp_wifi_set_channel()
#include <stdlib.h>
#include <string.h>
#include <esp_random.h>
#include <ctype.h>
#include <esp_crc.h>
#include <Base64.h>
// SEE: https://github.com/espressif/esp-idf/blob/4523f2d6/components/esp_wifi/include/esp_now.h
#define CHANNEL 1

esp_now_peer_info_t broadcaster = {};
uint8_t broadcastAddress[] = {0xFF,0XFF,0XFF,0xFF,0xFF,0xFF};
#define PAIR_BUTTON 0
#define PAIR_FOR_MS 30000
#define MSGPREF 01
#define COMMAND_PAIR 001
unsigned long long privKey = 0;
unsigned long long pubKey = 0;
unsigned long lastPairPress=-10000;
// Select a large prime number (replace with a suitable prime for real use)
unsigned long long DHPRIME_P = 329565970448322901398560964842595861041;
unsigned long long DHPRIME_G = 329565970448322901398560964842595860927;


// Function to generate a random number within a given range (inclusive)
uint64_t random_in_range(uint64_t min, uint64_t max) {
  uint64_t tr;
  esp_fill_random(&tr, 8);  
  //esp_random(&tr);  
  while (tr < min) {    
    uint8_t adjv;
    esp_fill_random(&adjv, 1);  
    tr = tr >> 8;
    *(uint8_t *)tr = adjv;
  }
  while (tr > max) {
    tr = tr << 1;
  }
  return (long long) tr;
}


uint64_t genPrivateKey() {
  return random_in_range(1, DHPRIME_P - 2);
}

uint64_t genPublicKey(uint64_t privKey) {
  return (uint64_t) pow(DHPRIME_G, privKey) % DHPRIME_P;
}

uint64_t calcSharedSecret(uint64_t otherPubKey, uint64_t myPrivKey) {
  return (uint64_t) pow(otherPubKey, myPrivKey) % DHPRIME_P;
}



void formatMac(char *dest, const uint8_t *mac) {
  sprintf(dest, "%X:%X:%X:%X:%X:%X",  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ;
}

unsigned long elapMs(unsigned long stop) {
  return millis() - stop;
}

bool isInPairMode() {  
  if (elapMs(lastPairPress) < PAIR_FOR_MS) {
    return true;
  } else {
    return false;
  }
}


const char *espNowError(int errNum) {  
  switch(errNum) {
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
    //case  ESP_ERR_ESPNOW_CHAN:
    //  return "ESP_ERR_ESPNOW_CHAN";
    default:
      return "ESP UNKNOWN ERROR";
  }
}


#include <cstring>

class ESPNowStream {
private:
  // Buffer to store received data
  uint8_t buffer[128]; // Adjust buffer size as needed
  int bufferIndex;

  // Buffer to store data to be transmitted
  uint8_t txBuffer[128]; // Adjust buffer size as needed
  int txBufferIndex;

public:
  ESPNowStream() : bufferIndex(0), txBufferIndex(0) {}

  // Function to handle received ESPNow packets (replace with your actual receive logic)
  void onReceive(const uint8_t *data, size_t len) {
    // Check for buffer overflow
    if (bufferIndex + len >= sizeof(buffer)) {
      // Handle buffer overflow (e.g., discard data or throw exception)
      return;
    }

    // Copy received data to buffer
    memcpy(buffer + bufferIndex, data, len);
    bufferIndex += len;
  }

  // Emulates Serial.readline, waits for a complete packet (ending with newline)
  std::string readline() {
    std::string result;

    // Loop until a newline is found or buffer is empty
    while (bufferIndex > 0 && result.find('\n') == std::string::npos) {
      // Extract character from buffer
      char c = static_cast<char>(buffer[0]);

      // Append character to result string
      result += c;

      // Remove character from buffer
      memmove(buffer, buffer + 1, bufferIndex - 1);
      bufferIndex--;
    }

    // Check if a newline was found
    if (result.find('\n') != std::string::npos) {
      // Remove trailing newline from result (optional)
      result.pop_back();
    }

    return result;
  }

  // Simulates writing data to ESPNow (replace with your actual transmit logic)
  size_t write(uint8_t data) {
    // Check for buffer overflow
    if (txBufferIndex >= sizeof(txBuffer)) {
      // Handle buffer overflow (e.g., return error or wait for space)
      return 0;
    }

    // Add data to transmit buffer
    txBuffer[txBufferIndex++] = data;

    // Replace with your actual ESPNow transmit logic (e.g., call a send function)
    // This example simulates successful transmission by returning the data size
    return 1;
  }
};


// Init ESP Now with fallback
void InitESPNow() {
 
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    Serial.println("ESP NOW INIT FAILED restarting");
    delay(5000);
    ESP.restart();
  }
}


void sendBroadcast(char *msg, int size) {
  Serial.println("sending broadcast");
  esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t *) msg, size );
  if (result != ESP_OK) {
    const char *errStr = espNowError(result);
    Serial.printf("failed send res=%d %s\n", result, errStr);
  }   
}

void OnDataSent(const uint8_t *macAddr, esp_now_send_status_t  status) {
  Serial.println("onDataSent");
  char macStr[18];
  formatMac((char *) macStr, macAddr);
  Serial.printf("last Sent mac=%s status=%d\n", macStr, status);
}

bool isCommandStr(const char *data, int dataLen) {
   if (dataLen < 6) { return false;}
   if ((data[4] == ':') && (isxdigit(data[0])) && (isxdigit(data[1])) && (isxdigit(data[2]))) {
    return true;
   }
   return false;
}

void processCommand(const uint8_t *macAddr, char *data, int dataLen) {
    data[3]=0;
    data[7]=0;
    short  id; // To store ID (3 hex digits + null terminator)
    short  cmd; // To store command (3 hex digits + null terminator)
    uint64_t pubKey; // To store public key (16 hex digits + null terminator)
    id = (short) strtoul(data,NULL,16);
    cmd = (short) strtoul(data+4,NULL,16);
    pubKey = strtoull(data+8,NULL,16);  
    
    // TODO: Pull last 8 bytes and compare to reste of message using
    //  hardware CRC.

    // TODO: Since we only have a single ESP32 Now onDataRecv 
    //  once we recognize that we have a command we look up 
    //  the class instance that will be processing it send the 
    //  command there. Add command dispatch to proper handlers to
    // those instances.   Have not made a decision about wether 
    // we have a separate handler by macAddr or not.
}


// Called by ESPNOW when messages are received.
void OnDataRecv(const uint8_t *macAddr, const uint8_t *data, int dataLen) {
  Serial.println("OnDataRecv");
  char macStr[18];
  char dbuff[ESP_NOW_MAX_DATA_LEN + 1];
  formatMac((char *) macStr, macAddr);  
  memcpy(dbuff, data, dataLen);
  dbuff[dataLen]=0;
  if (isCommandStr(data, dataLen)) {
    processCommand(mac, data, dataLen);
  } else {
    Serial.printf("NON COMMAND: MAC=%d DATA=%s\n", macStr, (char *) dbuff);
    char buff[ESP_NOW_MAX_DATA_LEN + 1];
  }
}


unsigned long lastSend=0;
char *buff[200];

#ifdef DELME
// config AP SSID
void configDeviceAP() {
  WiFi.mode(WIFI_AP);
  const char *SSID = "Slave_1";
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    Serial.print("AP CHANNEL "); Serial.println(WiFi.channel());
  }
}
#endif


void setup() {
  delay(1200);
  Serial.begin(115200);
  Serial.println("BEGIN SETUP\n");
  
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  Serial.print("STA CHANNEL "); Serial.println(WiFi.channel());
  pinMode(PAIR_BUTTON, INPUT_PULLUP);

  //configDeviceAP();

  formatMac((char *) buff, broadcastAddress);
  Serial.printf("broadcast MAC=%s", buff);
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  Serial.println("L81: before memcpy");
  // put your setup code here, to run once:
  memcpy(&broadcaster.peer_addr, broadcastAddress,6);
  Serial.println("after println");
  Serial.println("before InitESPNow");
  InitESPNow(); 
  Serial.println("after InitESPNow");
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("after onRec register");
  esp_now_register_send_cb(OnDataSent);
  Serial.println("after onSend register");
  if (!esp_now_is_peer_exist(broadcastAddress)) {
    int stat = esp_now_add_peer(&broadcaster);
    if (stat != ESP_OK) {      
      Serial.printf("failed send res=%d %s\n", stat, espNowError(stat));
    }  
  }
  //WiFi.softAPdisconnect(true);
  //WiFi.mode(WIFI_STA);
  privKey=genPrivateKey();
  Serial.printf("privKey=%llX\n", privKey);
  pubKey=genPublicKey(privKey);
  Serial.printf("pubKeyKey=%llX\n", pubKey);
}



void loop() {

  int pbs = digitalRead(PAIR_BUTTON);
  if (pbs == 0) {
      //Serial.println("pair button pushed");
      lastPairPress = millis();
  }

  unsigned long sinceSend = elapMs(lastSend);
  if (sinceSend > 5000) {
     Serial.println("in print interval");
     Serial.printf(" pair button state=%d sincPair=%lu\n", pbs, elapMs(lastPairPress));

     if (isInPairMode()) {
       Serial.printf("send pair request\n");
       sprintf((char *)buff, "%03X:%03X:%036llX",MSGPREF,COMMAND_PAIR,pubKey);
       Serial.println("after sprintf");
       sendBroadcast((char *) buff, strlen((char *)buff));
     } else {
       Serial.println("L300: before sprintf");
       sprintf((char *)buff, "ms=%ld since=%ld\n", millis(), sinceSend);
       Serial.printf("L302: after sprintf");
       sendBroadcast((char *) buff, strlen((char *)buff));
     }
     lastSend = millis();
  }

  // put your main code here, to run repeatedly:
  //sp_err_t addStatus = esp_now_add_peer(&slave);
  //    if (addStatus == ESP_OK) {
        // Pair success

}




