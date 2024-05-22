/* IoT Mesh Proxy -
 *
 *  Securely carry data and serial emulation across a dynamically discoverd mesh of sensor nodes.
 *  See: readme.txt for protocol design and
 *   detailed design notes
 *
 */


#include "src/IoTMeshProxy.h"

IoTMeshProxy *proxy;
unsigned long lastSend = 0;
char *buff[200];

void setup()
{
  IMP_SBuff = (char *)buff; // want proxy to reuse our buffer for string formatting in static methods  
  delay(1200);
  Serial.begin(115200);
  Serial.println("BEGIN SETUP\n");
  pinMode(0, INPUT);
 
  proxy = new IoTMeshProxy();
  proxy->appId = 1;
 
  // proxy.privKey=genPrivateKey();
  // Serial.printf("privKey=%llX\n", proxy.privKey);
  // proxy.pubKey=genPublicKey(proxy.privKey);
  // Serial.printf("pubKeyKey=%llX\n", proxy.pubKey);
}

void loop()
{

  int pbs = digitalRead(PAIR_BUTTON);
  if (pbs == 0)
  {
    // Serial.println("pair button pushed");
    proxy->lastPairStarted = millis();
  }
  proxy->service();

  unsigned long sinceSend = elapMs(lastSend);
  if (sinceSend > 5000)
  {
    #ifdef SEND_PERIODIC_BROADCAST
      Serial.println("in print interval");
      Serial.println("L300: before sprintf");
      sprintf((char *)buff, "ms=%ld since=%ld\n", millis(), sinceSend);
      Serial.printf("L302: after sprintf");
      IoTMeshProxy::SendBroadcast((uint8_t *)buff, strlen((char *)buff));
      lastSend = millis();
    #endif
  }
}
