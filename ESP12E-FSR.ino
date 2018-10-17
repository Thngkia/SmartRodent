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
char auth[] = "76a2d7d3f7de4da5bee8c225dea0678e";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "SINGTEL-42C4";
char pass[] = "eegisaagio";

//RTC_DATA_ATTR
int checker = 0;

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

    if (force > (Rodent_Weight + Trap_Weight)) {
      if (checker == 0) {
        Blynk.begin(auth, ssid, pass);
        Blynk.run();
        Blynk.virtualWrite(V1, force);
        delay(500);
        Blynk.email("SmartRodent2018@gmail.com", "FSR Glue Trap", "FSR Trap Activated!");
        Blynk.notify("FSR Trap Activated!");
        checker = checker + 1;
        Serial.println("Going to sleep now!");
        ESP.deepSleep(30e6);
        Serial.println("ERROR! Should be sleeping!");
      } else {
        Blynk.begin(auth, ssid, pass);
        Blynk.run();
        Blynk.virtualWrite(V1, force);
        delay(500);
        Blynk.email("SmartRodent2018@gmail.com", "FSR Glue Trap", "FSR Rodent Caught!");
        Blynk.notify("FSR Rodent Caught!");
      }
    } else {
      if (checker > 0) {
        Blynk.begin(auth, ssid, pass);
        Blynk.run();
        Blynk.virtualWrite(V1, force);
        delay(500);
        Blynk.email("SmartRodent2018@gmail.com", "FSR Glue Trap", "FSR Rodent Escaped!");
        Blynk.notify("FSR Rodent Escaped!");
        checker = 0;
      }
    }
  }

  Serial.println("Going to sleep now!");
  ESP.deepSleep(60e6);
  Serial.println("ERROR! Should be sleeping!");
}
