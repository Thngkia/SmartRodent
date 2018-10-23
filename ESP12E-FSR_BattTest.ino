#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>

#define uS_TO_s_FACTOR 1000000      /* Conversion factor for micro seconds to seconds */
#define uS_TO_mins_FACTOR 60000000  /* Conversion factor for micro seconds to mins */
#define uS_TO_hr_FACTOR 3600000000  /* Conversion factor for micro seconds to hours */

#define WAKE_INTERVAL  1           /* Periodic Check Time(in hours) */
#define TIME_AFTER_ACTIVATION 30            /* Time Between Trap Activation and Double Checking (in mins) */
#define Rodent_Weight 40           /* 1/5  of the weight of 200g   Rodent to be caught */
#define Trap_Weight 150              /* Weight of Trap itself without anything on it */

//Constants:
const int fsrAnalogPin = A0;  // Pin A0 to read analog input
const float VCC = 3.28;       // Measured voltage of Sparkfun 3.3v
const float R_DIV = 9710.0;   //Measured resistance of 10k resistor

//Variables:
int fsrReading;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "e51eea1d1e594a3c8f115b699f605fb3";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "SINGTEL-42C4";
char pass[] = "eegisaagio";

char serialnum[] = "FSR Glue Trap 4";

WiFiClient client;

String MakerIFTTT_Key ;
String MakerIFTTT_Event;
char *append_str(char *here, String s) {
  int i = 0;
  while (*here++ = s[i]) {
    i++;
  };
  return here - 1;
}
char *append_ul(char *here, unsigned long u) {
  char buf[20];
  return append_str(here, ultoa(u, buf, 10));
}

char post_rqst[256];
char *p;
char *content_length_here;
char *json_start;
int compi;

void setup() {
  Serial.begin(9600);       // Begin serial communication
}

void loop() {
  fsrReading = analogRead(fsrAnalogPin);         //Read and store analog value from Force Sensitive Resistance

  // If the FSR has no pressure, the resistance will be
  // near infinite. So the voltage should be near 0.
  if (fsrReading != 0) // If the analog reading is non-zero
  {
    // Use ADC reading to calculate voltage:
    float fsrV = fsrReading * VCC / 1023.0;
    // Use voltage and static resistor value to
    // calculate FSR resistance:
    float fsrR = R_DIV * (VCC / fsrV - 1.0);
    Serial.println("Resistance: " + String(fsrR) + " ohms");
    // Guesstimate force based on slopes in figure 3 of
    // FSR datasheet:
    float force;
    float fsrG = 1.0 / fsrR; // Calculate conductance
    // Break parabolic curve down into two linear slopes:
    if (fsrR <= 600)
      force = (fsrG - 0.00075) / 0.00000032639;
    else
      force =  fsrG / 0.000000642857;
    Serial.println("Weight: " + String(force) + " g");
    Serial.println();

    // SETUP the Trap Trigger and Send EMAIL
    // and PUSH Notification

    WiFi.disconnect();
    delay(3000);
    Serial.println("Starting to connect");
    WiFi.begin(ssid, pass);
    Blynk.begin(auth, ssid, pass);
    while ((!(WiFi.status() == WL_CONNECTED))) {
      delay(300);
    }

    if (client.connect("maker.ifttt.com", 80)) {
      MakerIFTTT_Key = "d1bgn89CimZ7PHgyPGebwEXAGeNFE2XxcKtCYslRp0B";
      MakerIFTTT_Event = "email";
      p = post_rqst;
      p = append_str(p, "POST /trigger/");
      p = append_str(p, MakerIFTTT_Event);
      p = append_str(p, "/with/key/");
      p = append_str(p, MakerIFTTT_Key);
      p = append_str(p, " HTTP/1.1\r\n");
      p = append_str(p, "Host: maker.ifttt.com\r\n");
      p = append_str(p, "Content-Type: application/json\r\n");
      p = append_str(p, "Content-Length: ");
      content_length_here = p;
      p = append_str(p, "NN\r\n");
      p = append_str(p, "\r\n");
      json_start = p;
      p = append_str(p, "{\"value1\":\"");
      p = append_str(p, "smartrodent2018@gmail.com");
      p = append_str(p, "\",\"value2\":\"");
      p = append_str(p, serialnum);
      p = append_str(p, "\",\"value3\":\"");
      p = append_str(p, "Battery Test!");
      p = append_str(p, "\"}");

      compi = strlen(json_start);
      content_length_here[0] = '0' + (compi / 10);
      content_length_here[1] = '0' + (compi % 10);
      client.print(post_rqst);
      Serial.println("The email was sent!");

      Blynk.run();
      Blynk.virtualWrite(V1, force);
      delay(500);
      Blynk.email("SmartRodent2018@gmail.com", serialnum, "Battery Test!");
      Blynk.notify("Battery Test!");

      Serial.println("Going to sleep for 30 seconds!");
      ESP.deepSleep(300e6);
      Serial.println("ERROR! Should be sleeping!");
    }
  }
}
