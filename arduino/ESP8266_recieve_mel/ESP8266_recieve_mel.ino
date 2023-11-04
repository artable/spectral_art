/*
  Recieve int array and print to console
  Configure:
  STASSID, STAPSK to the wifi the signal processor is on
  port = 25000
*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PolledTimeout.h>
#include <algorithm>  // std::min

#ifndef STASSID
#define STASSID "spectral_fi"
#define STAPSK "1234567890"
#endif

constexpr int port = 25000;

WiFiServer server(port);
WiFiClient client;

constexpr size_t sizes[] = { 0, 512, 384, 256, 128, 64, 16, 8, 4 };
constexpr uint32_t breathMs = 200;
esp8266::polledTimeout::oneShotFastMs enoughMs(breathMs);
esp8266::polledTimeout::periodicFastMs test(2000);
int t = 1;  // test (1, 2 or 3, see below)
int s = 0;  // sizes[] index

void setup() {

  Serial.begin(115200);
  Serial.println(ESP.getFullVersion());

  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  Serial.print("\nConnecting to ");
  Serial.println(STASSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println();
  Serial.print("connected, address=");
  Serial.println(WiFi.localIP());

  server.begin();

  MDNS.begin("echo23");

  Serial.printf("Ready!\n"
                "- Use 'telnet/nc echo23.local %d' to try echo\n\n"
                "- Use 'python3 echo-client.py' bandwidth meter to compare transfer APIs\n\n"
                "  and try typing 1, 1, 1, 2, 2, 2, 3, 3, 3 on console during transfers\n\n",
                port);
}


void loop() {

  MDNS.update();

  static uint32_t tot = 0;
  static uint32_t cnt = 0;
  // if (test && cnt) {
  //   Serial.printf("measured-block-size=%u min-free-stack=%u", tot / cnt, ESP.getFreeContStack());
  //   if (t == 2 && sizes[s]) { Serial.printf(" (blocks: at most %d bytes)", sizes[s]); }
  //   if (t == 3 && sizes[s]) { Serial.printf(" (blocks: exactly %d bytes)", sizes[s]); }
  //   if (t == 3 && !sizes[s]) { Serial.printf(" (blocks: any size)"); }
  //   Serial.printf("\n");
  // }

  // check if there are any new clients
  if (server.hasClient()) {
    client = server.accept();
    Serial.println("New client");
  }

  if (Serial.available()) {
    s = (s + 1) % (sizeof(sizes) / sizeof(sizes[0]));
    // switch (Serial.read()) {
    //   case '1':
    //     if (t != 1) s = 0;
    //     t = 1;
    //     Serial.println("byte-by-byte (watch then press 2, 3 or 4)");
    //     break;
    //   case '2':
    //     if (t != 2) s = 1;
    //     t = 2;
    //     Serial.printf("through buffer (watch then press 2 again, or 1, 3 or 4)\n");
    //     break;
    //   case '3':
    //     if (t != 3) s = 0;
    //     t = 3;
    //     Serial.printf("direct access (sendAvailable - watch then press 3 again, or 1, 2 or 4)\n");
    //     break;
    //   case '4':
    //     t = 4;
    //     Serial.printf("direct access (sendAll - close peer to stop, then press 1, 2 or 3 before restarting peer)\n");
    //     break;
    // }
    tot = cnt = 0;
    ESP.resetFreeContStack();
  }

  enoughMs.reset(breathMs);
  uint8_t client_buffer [8];
  while (client.available() && !enoughMs) {
    client.read(client_buffer, 8);
    for (uint8_t band : client_buffer) {
      Serial.print(band);
      Serial.print(" ");
    }
    Serial.print("\n");
    cnt++;
  }
}
