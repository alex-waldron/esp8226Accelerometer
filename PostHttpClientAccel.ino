/**
   PostHTTPClient.ino

    Created on: 21.11.2016

*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_LIS3DH.h>

/* this can be run with an emulated server on host:
        cd esp8266-core-root-dir
        cd tests/host
        make ../../libraries/ESP8266WebServer/examples/PostServer/PostServer
        bin/PostServer/PostServer
   then put your PC's IP address in SERVER_IP below, port 9080 (instead of default 80):
*/
//#define SERVER_IP "10.0.1.7:9080" // PC address with emulation on host
#define SERVER_IP "10.0.0.12:8000"

#ifndef STASSID
#define STASSID "Skatealextori"
#define STAPSK  "Alextori12"
#endif
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
bool collectData = false;
unsigned long startTime = 0;
String data = "";
void sendToCloud(String dataToSend) {
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, "http://" SERVER_IP "/postplain/"); //HTTP
    http.addHeader("Content-Type", "application/json");

    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    int httpCode = http.POST(dataToSend);

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();

  }
}


void setup() {
  pinMode(D0, OUTPUT);
  digitalWrite(D0, LOW);
  pinMode(D4, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  if (!lis.begin()) {
    Serial.println("Couldnt start accel");
    while (1);
  }
  lis.setRange(LIS3DH_RANGE_2_G);
}

void loop() {
  if (digitalRead(D4) == LOW) {
    if (collectData) {
      sendToCloud(data);
      sendToCloud("done");
      data = "";
    } else {
      startTime = millis();
    }

    collectData = !collectData;
    digitalWrite(D0, collectData);
    delay(1000);
  }
 


  if (collectData) {
    sensors_event_t event;
    lis.getEvent(&event);
    data += event.acceleration.x;
    data += " ";
    data += event.acceleration.y;
    data += " ";
    data += event.acceleration.z;
    data += " ";
    data += (millis() - startTime) / 1000.0;
    data += "/";
    // wait for WiFi connection
    Serial.println(data.length());
    if (data.length() > 1000) {
      sendToCloud(data);
      data = "";
    }
    delay(16);
  }


  Serial.println(collectData);
  
}
