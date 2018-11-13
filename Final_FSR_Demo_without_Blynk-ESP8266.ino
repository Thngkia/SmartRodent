// Libraries
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>
#include <stdbool.h>

extern "C" {
#include "user_interface.h" //this is for the RTC memory read/write function
}

/*
 *********************EDIT HERE IF NEEDED**********************
*/

// You should get Auth Token in the Blynk App
char auth[] = "78cac8078ab54116b584cf8d501d06bd";

// Your WiFi credentials.
// Set password to "" for open networks (no password)
char ssid[] = "killem wifi mesh";
char pass[] = "williamtan";

// Email Address to send notifications to
char emailadd[] = "SmartRodent2018@gmail.com";

// IFTTT Key can be obtained from IFTTT account on IFTTT website
char MakerIFTTT_Key[] = "d1bgn89CimZ7PHgyPGebwE_6j4XtIFukTAoZwbtXaja";

// Trap Serial Number
char serialnum[] = "FSR Glue Trap Demo";

/*
 ***********************EDIT UP TO HERE*************************
*/

// Predefined Constants
#define Rodent_Weight 40           /* 1/5 of the weight of 200g Rodent to be caught */

// RTC Structure
typedef struct {
  boolean count = false;
  int bootcount = 0;
  int TrapWeight = 0;
} rtcStore;

// RTC Variable Declaration
rtcStore rtcMem;

// Assignment of pins used
const int fsrAnalogPin = A0;  // Pin A0 to read analog input (FSR)
const int calibrationPin = 0;  // Calibrate Button to Pin D3

// Constants Declared to convert FSR PWM values to grams(g)
const float VCC = 3.28;       // Exact input voltage through 3V3 pin
const float R_DIV = 9710.0;   //Measured exact resistance of 10k resistor

//Variables
int fsrReading;
int weight;

WiFiClient client;

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

// Functions

// Connect to WiFi
void ConnectWiFi() {
  WiFi.disconnect();
  delay(3000);
  Serial.println("Starting to connect to " + String(ssid));
  WiFi.begin(ssid, pass);
  while ((!(WiFi.status() == WL_CONNECTED))) {
    delay(300);
  }
  Serial.println("Connected!");
  Serial.println();
}

// Send Email through IFTTT
/*
   For MakerIFTTT_Event input, the different FSR will give you different messages to be sent as notifications
   FSR  = Starting to calibrate!
   FSR0 = Calibrated!
   FSR1 = FSR Trap Activated!
   FSR2 = FSR Rodent Caught!
   FSR3 = FSR Rodent Escaped!
   FSR4 = Still Running!

   The above messages can be edited under settings for each applets
*/

void IFTTTEmail(String MakerIFTTT_Event) {
  if (client.connect("maker.ifttt.com", 80)) {
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
    Serial.println("Email sent through IFTTT!");
    Serial.println();
  }
}

// Send Push Notifications and Email through Blynk
void BlynkNotif(String Notif) {
  Blynk.run();
  delay(500);
  Blynk.email(emailadd, serialnum, Notif);
  Serial.println("Email sent through Blynk!");
  Blynk.notify(Notif);
  Serial.println("Mobile Notification sent through Blynk!");
  Serial.println();
}

// Read FSR and check weight
int CheckWeight() {
  fsrReading = analogRead(fsrAnalogPin);         //Read and store analog value from Force Sensitive Resistance

  // Use ADC reading to calculate voltage:
  float fsrV = fsrReading * VCC / 1023.0;
  // Use voltage and static resistor value to
  // calculate FSR resistance:
  float fsrR = R_DIV * (VCC / fsrV - 1.0);
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

  return force;
}

// Main Body
void setup() {
  Serial.begin(9600);       // Begin serial communication
  Serial.println();
  Serial.println("--------------------------------------------------");
  Serial.println();
  Serial.println(String(serialnum) + " is awake now!");
  Serial.println();
}

void loop() {
  delay(1000); //Time lag to press calibration button

  int calibrate = digitalRead(calibrationPin);

  if (calibrate == 0) {
    Serial.println("Starting to calibrate now!");
    Serial.println();

    // Connect WiFi & Blynk
    ConnectWiFi();
    //Blynk.begin(auth, ssid, pass);

    // Send Notifications
    IFTTTEmail("FSR");
    //BlynkNotif("Starting to calibrate!");

    // Time delay to set trap (1 second = 1000)
    Serial.println("You have 30 seconds to set the trap!");

    Serial.print("... ");
    delay(5000);
    Serial.print("... ");
    delay(5000);
    Serial.print("... ");
    delay(5000);
    Serial.print("... ");
    delay(5000);
    Serial.print("... ");
    delay(5000);
    Serial.print("... ");
    delay(5000);
    Serial.print("... ");
    Serial.println();

    // Check Weight on FSR
    weight = CheckWeight();

    // Initialising RTC Variables
    rtcMem.bootcount = 0;
    rtcMem.count = false;
    rtcMem.TrapWeight = weight;
    system_rtc_mem_write(65, &rtcMem, 20);
    yield();

    Serial.println(String(serialnum) + " calibrated!");
    Serial.println();

    // Send Notifications
    IFTTTEmail("FSR0");
    //BlynkNotif("Calibrated!");

  } else {
    // Check Weight on FSR
    weight = CheckWeight();

    //Read RTC Memory
    system_rtc_mem_read(65, &rtcMem, sizeof(rtcMem));
    yield();

    if (!rtcMem.count) {
      if (weight > (Rodent_Weight + rtcMem.TrapWeight)) {
        Serial.println("Trap Activated!");
        Serial.println();

        // Connect WiFi & Blynk
        ConnectWiFi();
        //Blynk.begin(auth, ssid, pass);

        // Send Notifications
        IFTTTEmail("FSR1");
        //BlynkNotif("FSR Trap Activated!");

        // Change status in RTC
        rtcMem.count = true;
        system_rtc_mem_write(65, &rtcMem, 20);
        yield();

        Serial.println("Going to sleep and check the trap again in 5 seconds!");
        Serial.println();
        Serial.println("--------------------------------------------------");
        Serial.println();
        ESP.deepSleep(5e6); // Number before 'e' is the duration (in seconds) to be in deep sleep
        Serial.println("ERROR! Should be sleeping!");
      }
    }

    if (rtcMem.count) {
      if (weight > (Rodent_Weight + rtcMem.TrapWeight)) {
        Serial.println("Rodent Caught!");
        Serial.println();

        // Connect WiFi & Blynk
        ConnectWiFi();
        //Blynk.begin(auth, ssid, pass);

        // Send Notifications
        IFTTTEmail("FSR2");
        //BlynkNotif("FSR Rodent Caught!");

        Serial.println("Going to sleep until reset!");
        Serial.println();
        Serial.println("--------------------------------------------------");
        Serial.println();
        ESP.deepSleep(0); // 0 means that it will be in deep sleep till reset button is pressed
        Serial.println("ERROR! Should be sleeping!");
      } else {
        Serial.println("Rodent Escaped!");
        Serial.println();

        // Connect WiFi & Blynk
        ConnectWiFi();
        //Blynk.begin(auth, ssid, pass);

        // Send Notifications
        IFTTTEmail("FSR3");
        //BlynkNotif("FSR Rodent Escaped!");

        // Reset trap status in RTC
        rtcMem.count = false;
        system_rtc_mem_write(65, &rtcMem, 20);
        yield();
      }
    }

    // Increasing Boot Count every time it wakes up in RTC
    rtcMem.bootcount = rtcMem.bootcount + 1;
    system_rtc_mem_write(65, &rtcMem, 20);
    yield();

    // Read RTC Memory
    system_rtc_mem_read(65, &rtcMem, sizeof(rtcMem));
    yield();

    //Serial.println("Boot Count:" + String(rtcMem.bootcount));

    //Daily Check if trap is still running and Rodent is not caught yet
    if (rtcMem.bootcount % 5 == 0) {
      Serial.println("Daily Status Check!");
      Serial.println();

      // Connect WiFi & Blynk
      ConnectWiFi();
      //Blynk.begin(auth, ssid, pass);

      // Send Notifications
      IFTTTEmail("FSR4");
      //BlynkNotif("Still Running!");
    }

    Serial.println("No Rodent Detected! Going to sleep for 15 seconds!");
    Serial.println();
    Serial.println("--------------------------------------------------");
    Serial.println();
    ESP.deepSleep(15e6); // Number before 'e' is the duration (in seconds) to be in deep sleep
    Serial.println("ERROR! Should be sleeping!");
  }
}
