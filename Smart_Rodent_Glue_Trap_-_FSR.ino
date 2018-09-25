#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <esp_sleep.h>
#include <esp_wifi.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "0df5a56a12aa42088733fad796023c12";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "HUAWEI nova 2i";
char pass[] = "xingliang";

//Constants:
const float VCC = 3.28; // Measured voltage of Sparkfun 3.3v
const float R_DIV = 9788.0; // Measured resistance of 10k resistor

//Variables:
int fsrAnalogPin = 32; // FSR is connected to analog D4
int fsrReading;      // the analog reading from the FSR resistor divider

void setup(){
  Serial.begin(9600);       // Begin serial communication
  Blynk.begin(auth, ssid, pass);
}

void loop(){
  
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
  
  if (force > 200) {
    Blynk.email("Smartrodent2018@gmail.com", "Smart Rodent Glue Trap", "Trap Activated!");
    Blynk.notify("Trap Activated!");
    
    delay(900000); // Wait 15minutes and check status again

    fsrReading = analogRead(fsrAnalogPin);         //Read and store analog value from Force Sensitive Resistance
  
    // If the FSR has no pressure, the resistance will be
    // near infinite. So the voltage should be near 0.
    if (fsrReading != 0) { // If the analog reading is non-zero
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

      if (force > 200) {
        Blynk.email("Smartrodent2018@gmail.com", "Smart Rodent Glue Trap", "Rodent Caught!");
        Blynk.notify("Rodent Caught!");
      } else {
        Blynk.email("Smartrodent2018@gmail.com", "Smart Rodent Glue Trap", "Rodent Escape!");
        Blynk.notify("Rodent Escaped!");
      }
    }
   }
 }
}
