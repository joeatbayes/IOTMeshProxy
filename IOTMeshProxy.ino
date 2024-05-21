/* espnow_broadcast_serial_bridge
*
*  See: readme.txt for protocol design and 
*   detailed design notes
*
*  
*/

#include "src/diffie.h"
#include "src/IoTMeshProxy.h"


IoTMeshProxy proxy = IoTMeshProxy();
unsigned long lastSend=0;
char *buff[200];


void setup() {
  IoTMeshProxy::SBuff=(char *)buff; // want proxy to reuse our buffer for string formatting in static methods
  delay(1200);
  Serial.begin(115200);
  Serial.println("BEGIN SETUP\n");
  pinMode(0,INPUT);
  IoTMeshProxy::InitESPNow();
  
  proxy.appId = 1;
  IoTMeshProxy::RegisterHandler(&proxy);
  //proxy.privKey=genPrivateKey();
  //Serial.printf("privKey=%llX\n", proxy.privKey);
  //proxy.pubKey=genPublicKey(proxy.privKey);
  //Serial.printf("pubKeyKey=%llX\n", proxy.pubKey);
}


void loop() {

  int pbs = digitalRead(PAIR_BUTTON);
  if (pbs == 0) {
      //Serial.println("pair button pushed");      
      proxy.lastPairStarted = millis();
  }
  proxy.service();


  unsigned long sinceSend = elapMs(lastSend);
  if (sinceSend > 5000) {
     Serial.println("in print interval");
     Serial.println("L300: before sprintf");
     sprintf((char *)buff, "ms=%ld since=%ld\n", millis(), sinceSend);
     Serial.printf("L302: after sprintf");
     IoTMeshProxy::SendBroadcast((uint8_t *) buff, strlen((char *)buff));
     lastSend = millis();
  }


}




