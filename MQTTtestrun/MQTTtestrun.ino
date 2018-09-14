/////////////////////////////////
// Generated with a lot of love//
// with TUNIOT FOR ESP32     //
// Website: Easycoding.tn      //
/////////////////////////////////
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFi.h>

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME  "JeremyThng"
#define AIO_KEY  "6af1270a351a4ec3a851cff735520535"
WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

int  i;
boolean MQTT_connect();

boolean MQTT_connect() {  int8_t ret; if (mqtt.connected()) {    return true; }  uint8_t retries = 3;  while ((ret = mqtt.connect()) != 0) { mqtt.disconnect(); delay(2000);  retries--;if (retries == 0) { return false; }} return true;}

Adafruit_MQTT_Publish mydata = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/mydata");
void setup()
{
i = 0;
Serial.begin(9600);

  WiFi.disconnect();
  delay(3000);
  Serial.println("START");
  WiFi.begin("SINGTEL-DC6B","0015785067");
  while ((!(WiFi.status() == WL_CONNECTED))){
    delay(300);
    Serial.print("..");

  }
  Serial.println("Connected");
  Serial.println("Your IP is");
  Serial.println((WiFi.localIP()));

}


void loop()
{

    i = (random(100,200));
    if (MQTT_connect()) {
      if (mydata.publish(i)) {
        Serial.println("Random data sent");
        Serial.println(i);

      } else {
        Serial.println("Problem to send the random data");

      }

    } else {
      Serial.println("Problem connect to the site");

    }
    delay(60000);

}
