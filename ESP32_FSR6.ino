#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <esp_sleep.h>
#include <esp_wifi.h>

#define uS_TO_s_FACTOR 1000000      /* Conversion factor for micro seconds to seconds */
#define uS_TO_mins_FACTOR 60000000  /* Conversion factor for micro seconds to mins */
#define uS_TO_hr_FACTOR 3600000000  /* Conversion factor for micro seconds to hours */

#define WAKE_INTERVAL  1           /* Periodic Check Time(in hours) */
#define TIME_AFTER_ACTIVATION 15            /* Time Between Trap Activation and Double Checking (in mins) */
#define Rodent_Weight 40           /* 1/5  of the weight of 200g   Rodent to be caught */
#define Trap_Weight 150              /* Weight of Trap itself without anything on it */

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "e916a53c8c204c799a7e0edec0498c4e";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "killem wifi mesh";
char pass[] = "williamtan";

char serialnum[] = "FSR Glue Trap 6";

char IFTTTKey[] = "d1bgn89CimZ7PHgyPGebwE_6j4XtIFukTAoZwbtXaja";

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

//Constants:
const float VCC = 3.28; // Measured voltage of Sparkfun 3.3v
const float R_DIV = 9788.0; // Measured resistance of 10k resistor

//Variables:
int fsrAnalogPin = 32; // FSR is connected to analog D4
int fsrReading;      // the analog reading from the FSR resistor divider

RTC_DATA_ATTR int checker = 0;

/*
  void WIFI_Connect()
  {
  digitalWrite(26, HIGH);
  WiFi.disconnect();
  Serial.println("Reconnecting WiFi...");
  Blynk.begin(auth, ssid, pass);
  // Wait for connection
  for (int i = 0; i < 50; i++)
  {
    if (WiFi.status() != WL_CONNECTED) {
      delay ( 500 );
      digitalWrite(26, 0);
      Serial.print ( "." );
      delay ( 500 );
      digitalWrite(26, 1);
    }
  }
  digitalWrite(26, 0);
  }
*/

void setup() {
  Serial.begin(9600);       // Begin serial communication
  /*
    if (WiFi.status() != WL_CONNECTED)
    {
    digitalWrite(26, 0);
    WIFI_Connect();
    } else {
    digitalWrite(26, 1);
    }
  */
}

void loop() {

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

    // SETUP the Trap Trigger and Send EMAIL
    // and PUSH Notification

    if (force > (Rodent_Weight + Trap_Weight)) {
      WiFi.disconnect();
      delay(3000);
      Serial.println("Starting to connect");
      WiFi.begin(ssid, pass);
      Blynk.begin(auth, ssid, pass);
      while ((!(WiFi.status() == WL_CONNECTED))) {
        delay(300);
      }

      if (checker == 0) {
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

          Blynk.run();
          Blynk.virtualWrite(V1, force);
          delay(500);
          Blynk.email("SmartRodent2018@gmail.com", serialnum, "FSR Trap Activated!");
          Blynk.notify("FSR Trap Activated!");
          checker = 1;
          esp_sleep_enable_timer_wakeup(TIME_AFTER_ACTIVATION * uS_TO_mins_FACTOR);
          esp_wifi_disconnect;
          esp_wifi_stop;
          Serial.println("Going to sleep now!");
          esp_deep_sleep_start();
          Serial.println("ERROR! NOT SLEEPING!");
        }
      } else {
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

          Blynk.run();
          Blynk.virtualWrite(V1, force);
          delay(500);
          Blynk.email("SmartRodent2018@gmail.com", serialnum, "FSR Rodent Caught!");
          Blynk.notify("FSR Rodent Caught!");
          esp_wifi_disconnect;
          esp_wifi_stop;
          Serial.println("Going to sleep now!");
          esp_deep_sleep_start();
          Serial.println("ERROR! NOT SLEEPING!");
        }
      }
    } else {
      if (checker == 1) {
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

            Blynk.run();
            Blynk.virtualWrite(V1, force);
            delay(500);
            Blynk.email("SmartRodent2018@gmail.com", serialnum, "FSR Rodent Escaped!");
            Blynk.notify("FSR Rodent Escaped!");
            checker = 0;
          }
        }
      }
    }

    esp_sleep_enable_timer_wakeup(WAKE_INTERVAL * uS_TO_hr_FACTOR);

    esp_wifi_disconnect;
    esp_wifi_stop;
    esp_deep_sleep_start();
  }
