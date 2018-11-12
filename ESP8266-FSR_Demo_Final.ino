#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>
#include <stdbool.h>

extern "C" {
#include "user_interface.h" //this is for the RTC memory read/write function
}

#define uS_TO_s_FACTOR 1000000      /* Conversion factor for micro seconds to seconds */
#define uS_TO_mins_FACTOR 60000000  /* Conversion factor for micro seconds to mins */
#define uS_TO_hr_FACTOR 3600000000  /* Conversion factor for micro seconds to hours */

#define WAKE_INTERVAL  1           /* Periodic Check Time(in hours) */
#define TIME_AFTER_ACTIVATION 30            /* Time Between Trap Activation and Double Checking (in mins) */
#define Rodent_Weight 40           /* 1/5  of the weight of 200g   Rodent to be caught */
#define Trap_Weight 150              /* Weight of Trap itself without anything on it */

typedef struct {
  boolean count = false;
  int bootcount = 0;
  int TrapWeight = 0;
} rtcStore;

rtcStore rtcMem;

//Constants:
const int fsrAnalogPin = A0;  // Pin A0 to read analog input
const int calibrationPin = 0;  // Pin D4 to calibrate
const float VCC = 3.28;       // Measured voltage of Sparkfun 3.3v
const float R_DIV = 9710.0;   //Measured resistance of 10k resistor

//Variables:
int fsrReading;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "78cac8078ab54116b584cf8d501d06bd";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "killem wifi mesh";
char pass[] = "williamtan";
char emailadd[] = "SmartRodent2018@gmail.com";

char IFTTTKey[] = "d1bgn89CimZ7PHgyPGebwE_6j4XtIFukTAoZwbtXaja";

char serialnum[] = "FSR Glue Trap Demo";

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
  Serial.println();
  Serial.println("Awake now!");
}

void loop() {
  delay(1000); //Time lag to press calibration button

  int calibrate = digitalRead(calibrationPin);
  Serial.println("Calibrate = " + String(calibrate));

  if (calibrate == 0) {
    delay(30000);
    delay(30000);

    fsrReading = analogRead(fsrAnalogPin);         //Read and store analog value from Force Sensitive Resistance

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

    rtcMem.bootcount = 0;
    rtcMem.count = false;
    rtcMem.TrapWeight = force;
    system_rtc_mem_write(65, &rtcMem, 20);
    yield();
    Serial.println(String(serialnum) + " calibrated!");

    WiFi.disconnect();
    delay(3000);
    Serial.println("Starting to connect");
    WiFi.begin(ssid, pass);
    Blynk.begin(auth, ssid, pass);
    while ((!(WiFi.status() == WL_CONNECTED))) {
      delay(300);
    }
    if (client.connect("maker.ifttt.com", 80)) {
      MakerIFTTT_Key = IFTTTKey;
      MakerIFTTT_Event = "FSR0";
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
      p = append_str(p, serialnum);
      p = append_str(p, "\"}");

      compi = strlen(json_start);
      content_length_here[0] = '0' + (compi / 10);
      content_length_here[1] = '0' + (compi % 10);
      client.print(post_rqst);
      Serial.println("The calibration email was sent!");
    }
    Blynk.run();
    delay(500);
    Blynk.email(emailadd, serialnum, "Calibrated!");
    Blynk.notify("Calibrated!");
    Serial.println("Calibration Blynk email was sent!");
  } else {
    fsrReading = analogRead(fsrAnalogPin);         //Read and store analog value from Force Sensitive Resistance

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

    system_rtc_mem_read(65, &rtcMem, sizeof(rtcMem));
    yield();

    //Read RTC Memory
    system_rtc_mem_read(65, &rtcMem, sizeof(rtcMem));
    yield();

    if (!rtcMem.count) {
      if (force > (Rodent_Weight + rtcMem.TrapWeight)) {
        WiFi.disconnect();
        delay(3000);
        Serial.println("Starting to connect");
        WiFi.begin(ssid, pass);
        Blynk.begin(auth, ssid, pass);
        while ((!(WiFi.status() == WL_CONNECTED))) {
          delay(300);
        }

        if (client.connect("maker.ifttt.com", 80)) {
          MakerIFTTT_Key = IFTTTKey;
          MakerIFTTT_Event = "FSR1";
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
          p = append_str(p, serialnum);
          p = append_str(p, "\"}");

          compi = strlen(json_start);
          content_length_here[0] = '0' + (compi / 10);
          content_length_here[1] = '0' + (compi % 10);
          client.print(post_rqst);
          Serial.println("The email was sent!");
        }
        Blynk.run();
        Blynk.virtualWrite(V1, force);
        delay(500);
        Blynk.email(emailadd, serialnum, "FSR Trap Activated!");
        Blynk.notify("FSR Trap Activated!");
        Serial.println("Blynk email was sent!");

        rtcMem.count = true;
        system_rtc_mem_write(65, &rtcMem, 20);
        yield();

        Serial.println("Going to sleep for 5 seconds!");
        ESP.deepSleep(5e6);
        Serial.println("ERROR! Should be sleeping!");
      }
    }

    if (rtcMem.count) {
      if (force > (Rodent_Weight + rtcMem.TrapWeight)) {
        WiFi.disconnect();
        delay(3000);
        Serial.println("Starting to connect");
        WiFi.begin(ssid, pass);
        Blynk.begin(auth, ssid, pass);
        while ((!(WiFi.status() == WL_CONNECTED))) {
          delay(300);
        }
        if (client.connect("maker.ifttt.com", 80)) {
          MakerIFTTT_Key = IFTTTKey;
          MakerIFTTT_Event = "FSR2";
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
          p = append_str(p, serialnum);
          p = append_str(p, "\"}");

          compi = strlen(json_start);
          content_length_here[0] = '0' + (compi / 10);
          content_length_here[1] = '0' + (compi % 10);
          client.print(post_rqst);
          Serial.println("The email was sent!");
        }
        Blynk.run();
        Blynk.virtualWrite(V1, force);
        delay(500);
        Blynk.email(emailadd, serialnum, "FSR Rodent Caught!");
        Blynk.notify("FSR Rodent Caught!");
        Serial.println("Blynk email was sent!");

        rtcMem.count = false;
        system_rtc_mem_write(65, &rtcMem, 20);
        yield();

        Serial.println("Going to sleep forever until reset!");
        ESP.deepSleep(0);
        Serial.println("ERROR! Should be sleeping!");
      } else {
        WiFi.disconnect();
        delay(3000);
        Serial.println("Starting to connect");
        WiFi.begin(ssid, pass);
        Blynk.begin(auth, ssid, pass);
        while ((!(WiFi.status() == WL_CONNECTED))) {
          delay(300);
        }
        if (client.connect("maker.ifttt.com", 80)) {
          MakerIFTTT_Key = IFTTTKey;
          MakerIFTTT_Event = "FSR3";
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
          p = append_str(p, serialnum);
          p = append_str(p, "\"}");

          compi = strlen(json_start);
          content_length_here[0] = '0' + (compi / 10);
          content_length_here[1] = '0' + (compi % 10);
          client.print(post_rqst);
          Serial.println("The email was sent!");
        }
        Blynk.run();
        Blynk.virtualWrite(V1, force);
        delay(500);
        Blynk.email(emailadd, serialnum, "FSR Rodent Escaped!");
        Blynk.notify("FSR Rodent Escaped!");
        Serial.println("Blynk email was sent!");

        rtcMem.count = false;
        system_rtc_mem_write(65, &rtcMem, 20);
        yield();
      }
    }

    //Increasing Boot Count
    rtcMem.bootcount = rtcMem.bootcount + 1;
    system_rtc_mem_write(65, &rtcMem, 20);
    yield();

    system_rtc_mem_read(65, &rtcMem, sizeof(rtcMem));
    yield();

    Serial.println("Boot Count:" + String(rtcMem.bootcount));

    //Daily Check if trap is still running when Rodent is not caught
    if (rtcMem.bootcount % 5 == 0) {
      WiFi.disconnect();
      delay(3000);
      Serial.println("Starting to connect");
      WiFi.begin(ssid, pass);
      Blynk.begin(auth, ssid, pass);
      while ((!(WiFi.status() == WL_CONNECTED))) {
        delay(300);
      }
      if (client.connect("maker.ifttt.com", 80)) {
        MakerIFTTT_Key = IFTTTKey;
        MakerIFTTT_Event = "FSR4";
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
        p = append_str(p, serialnum);
        p = append_str(p, "\"}");

        compi = strlen(json_start);
        content_length_here[0] = '0' + (compi / 10);
        content_length_here[1] = '0' + (compi % 10);
        client.print(post_rqst);
        Serial.println("The daily check email was sent!");
      }
      Blynk.run();
      delay(500);
      Blynk.email(emailadd, serialnum, "Still Running!");
      Blynk.notify("Still Running!");
      Serial.println("Daily check Blynk email was sent!");
    }

    Serial.println("Going to sleep for 15 seconds!");
    ESP.deepSleep(15e6);
    Serial.println("ERROR! Should be sleeping!");
  }
}
