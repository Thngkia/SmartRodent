// Libraries to include
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <esp_sleep.h>
#include <esp_wifi.h>

// Time Factors
#define uS_TO_s_FACTOR 1000000      /* Conversion factor for micro seconds to seconds */
#define uS_TO_mins_FACTOR 60000000  /* Conversion factor for micro seconds to mins */
#define uS_TO_hr_FACTOR 3600000000  /* Conversion factor for micro seconds to hours */
#define TIME_TO_SLEEP  1           /* Periodic Check Time(in hours) */
#define TIME_TO_CAUGHT 5            /* Time Between Trap Activation and Double Checking (in mins) */

// Calibrations
#define rodent_weight 200           /* Minimum Weight of Rodent to be caught */
#define trap_weight 120              /* Weight of Trap itself without anything on it */

#define BLYNK_PRINT Serial

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "76a2d7d3f7de4da5bee8c225dea0678e";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "SINGTEL-42C4";
char pass[] = "eegisaagio";

//Constants:
const float VCC = 3.28; // Measured voltage of Sparkfun 3.3v
const float R_DIV = 9788.0; // Measured resistance of 10k resistor

//Variables:
int fsrAnalogPin = 32; // FSR is connected to analog D4
int fsrReading;      // the analog reading from the FSR resistor divider
int checker = 0;

void setup(){
  Serial.begin(9600);       // Begin serial communication
  Blynk.begin(auth, ssid, pass);
}

void loop(){
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_hr_FACTOR);
  
  fsrReading = analogRead(fsrAnalogPin);         //Read and store analog value from Force Sensitive Resistance
  
  // If the FSR has no pressure, the resistance will be
  // near infinite. So the voltage should be near 0.
  if (fsrReading != 0) // If the analog reading is non-zero
  {
    // Use ADC reading to calculate voltage:
    float fsrV = fsrReading * VCC / 4094.0;
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
    Blynk.run();
    Blynk.virtualWrite(V1, force);
    delay(500);
  
    // SETUP the Trap Trigger and Send EMAIL
    // and PUSH Notification
  
    if ((force > (rodent_weight + trap_weight)) && (checker == 0)) {
      Blynk.email("Smartrodent2018@gmail.com", "Smart Rodent Glue Trap", "FSR Trap Activated!");
      Blynk.notify("FSR Trap Activated!");
      checker = checker + 1;
      //esp_sleep_enable_timer_wakeup(TIME_TO_CAUGHT * uS_TO_s_FACTOR);
      //esp_wifi_disconnect;
      //esp_wifi_stop;
      //esp_deep_sleep_start();
    }
    
    if (checker == 1) {
      if (force > (rodent_weight + trap_weight)) {
        Blynk.email("Smartrodent2018@gmail.com", "Smart Rodent Glue Trap", "Caught!");
        Blynk.notify("Rodent Caught!");
        checker = checker + 1;
      } else {
        checker = 0;
      }
    }
  }
    esp_wifi_disconnect;
    esp_wifi_stop;
    esp_deep_sleep_start();
}
