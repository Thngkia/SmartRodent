#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <SparkFun_ADXL345.h> 
#include <SPI.h>

#define  DEVID   0x00  //Device ID Register
#define THRESH_TAP  0x1D  //Tap Threshold
#define OFSX    0x1E  //X-axis offset
#define OFSY    0x1F  //Y-axis offset
#define OFSZ    0x20  //Z-axis offset
#define DURATION  0x21  //Tap Duration
#define LATENT    0x22  //Tap latency
#define WINDOW    0x23  //Tap window
#define THRESH_ACT  0x24  //Activity Threshold
#define THRESH_INACT  0x25  //Inactivity Threshold
#define TIME_INACT  0x26  //Inactivity Time
#define ACT_INACT_CTL 0x27  //Axis enable control for activity and inactivity detection
#define THRESH_FF 0x28  //free-fall threshold
#define TIME_FF   0x29  //Free-Fall Time
#define TAP_AXES  0x2A  //Axis control for tap/double tap
#define ACT_TAP_STATUS  0x2B  //Source of tap/double tap
#define BW_RATE   0x2C  //Data rate and power mode control
#define POWER_CTL 0x2D  //Power Control Register
#define INT_ENABLE  0x2E  //Interrupt Enable Control
#define INT_MAP   0x2F  //Interrupt Mapping Control
#define INT_SOURCE  0x30  //Source of interrupts
#define DATA_FORMAT 0x31  //Data format control
#define DATAX0    0x32  //X-Axis Data 0
#define DATAX1    0x33  //X-Axis Data 1
#define DATAY0    0x34  //Y-Axis Data 0
#define DATAY1    0x35  //Y-Axis Data 1
#define DATAZ0    0x36  //Z-Axis Data 0
#define DATAZ1    0x37  //Z-Axis Data 1
#define FIFO_CTL  0x38  //FIFO control
#define FIFO_STATUS 0x39  //FIFO status

#define BUTTON_PIN_BITMASK 0x10 // 2^4 in hex
RTC_DATA_ATTR int bootCount = 0;


char ssid[] = "Joseph's Black Brick";
char pass[] = "loolalala";


ADXL345 adxl = ADXL345(10);



/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
WiFiClient client;

//const int ledPin = 2; //LED connected to D4
int tolerance = 10; //Sensitivity of the Alarm
boolean calibrated = false; //When accelerometer is calibrated- changes to true
boolean moveDetected = false; //When motion is detected-changes to true
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
char post_rqst[256]; char *p; char *content_length_here; char *json_start; int compi;

void displaySensorDetails(void)
{
  sensor_t sensor;
  accel.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" m/s^2");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" m/s^2");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" m/s^2");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}


void displayDataRate(void)
{
  Serial.print  ("Data Rate:    ");

  switch (accel.getDataRate())
  {
    case ADXL345_DATARATE_3200_HZ:
      Serial.print  ("3200 ");
      break;
    case ADXL345_DATARATE_1600_HZ:
      Serial.print  ("1600 ");
      break;
    case ADXL345_DATARATE_800_HZ:
      Serial.print  ("800 ");
      break;
    case ADXL345_DATARATE_400_HZ:
      Serial.print  ("400 ");
      break;
    case ADXL345_DATARATE_200_HZ:
      Serial.print  ("200 ");
      break;
    case ADXL345_DATARATE_100_HZ:
      Serial.print  ("100 ");
      break;
    case ADXL345_DATARATE_50_HZ:
      Serial.print  ("50 ");
      break;
    case ADXL345_DATARATE_25_HZ:
      Serial.print  ("25 ");
      break;
    case ADXL345_DATARATE_12_5_HZ:
      Serial.print  ("12.5 ");
      break;
    case ADXL345_DATARATE_6_25HZ:
      Serial.print  ("6.25 ");
      break;
    case ADXL345_DATARATE_3_13_HZ:
      Serial.print  ("3.13 ");
      break;
    case ADXL345_DATARATE_1_56_HZ:
      Serial.print  ("1.56 ");
      break;
    case ADXL345_DATARATE_0_78_HZ:
      Serial.print  ("0.78 ");
      break;
    case ADXL345_DATARATE_0_39_HZ:
      Serial.print  ("0.39 ");
      break;
    case ADXL345_DATARATE_0_20_HZ:
      Serial.print  ("0.20 ");
      break;
    case ADXL345_DATARATE_0_10_HZ:
      Serial.print  ("0.10 ");
      break;
    default:
      Serial.print  ("???? ");
      break;
  }
  Serial.println(" Hz");
}

void displayRange(void)
{
  Serial.print  ("Range:         +/- ");

  switch (accel.getRange())
  {
    case ADXL345_RANGE_16_G:
      Serial.print  ("16 ");
      break;
    case ADXL345_RANGE_8_G:
      Serial.print  ("8 ");
      break;
    case ADXL345_RANGE_4_G:
      Serial.print  ("4 ");
      break;
    case ADXL345_RANGE_2_G:
      Serial.print  ("2 ");
      break;
    default:
      Serial.print  ("?? ");
      break;
  }
  Serial.println(" g");
}

/****************** INTERRUPT ******************/
/*      Uncomment If Attaching Interrupt       */
int interruptPin = 4;                 // Setup d4 to be the interrupt pin

void setup(void)
{
  
  
 
    Serial.begin(9600);

    
//Create an interrupt that will trigger when a tap is detected.
    //  attachInterrupt(0,tap , RISING);
    


      
  
  /* Initialise the sensor */
  
  if (!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while (1);
  }
  
 
  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_16_G);
  // displaySetRange(ADXL345_RANGE_8_G);
  // displaySetRange(ADXL345_RANGE_4_G);
  // displaySetRange(ADXL345_RANGE_2_G);

  /* Display some basic information on this sensor */
  displaySensorDetails();

  /* Display additional settings (outside the scope of sensor_t) */
  displayDataRate();
  displayRange();
  Serial.println("");

  //Initialise LED Pin
 // pinMode(ledPin, OUTPUT);

  //calibrate accelerometer
  Serial.println("Calibrated!");
  calibrateAccel();
  
  delay(300);
  
btStop();
WiFi.mode(WIFI_OFF);

      Serial.println("Wifi off");
      



 esp_sleep_enable_ext0_wakeup(GPIO_NUM_4,0);



  
}
void loop(void)
{
 //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  
  /* Get a new sensor event */
  sensors_event_t event;
  accel.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
  // Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
  // Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
  //Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  "); Serial.println("m/s^2 ");
  // delay(500);

  // when restarted, initialises and arms
  if (checkMotion()) {
    moveDetected = true;
  }

  //If motion is detected- sound the alarm!
  if (moveDetected) {
    /*Serial.println("Alarm");
      ALARM();
      delay(1000);
    */

    
  
   
  WiFi.disconnect();
  delay(3000);
  Serial.println("Starting to connect");
  WiFi.begin(ssid, pass);
  while ((!(WiFi.status() == WL_CONNECTED))) {
    delay(300);
  }
  Serial.println("Connected!");
  Serial.println("Accelerometer Test"); Serial.println("");






    
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
      p = append_str(p, "Hello from esp32");
      p = append_str(p, "\",\"value3\":\"");

      if(bootCount=0){
      p = append_str(p, "Trap Armed");
      p = append_str(p, "\"}");

      compi = strlen(json_start);
      content_length_here[0] = '0' + (compi / 10);
      content_length_here[1] = '0' + (compi % 10);
      client.print(post_rqst);

      Serial.println("The email was sent!");
      }
      
      
      
      if(bootCount>0){
      p = append_str(p, "Trap Activated");
      p = append_str(p, "\"}");

      compi = strlen(json_start);
      content_length_here[0] = '0' + (compi / 10);
      content_length_here[1] = '0' + (compi % 10);
      client.print(post_rqst);

      Serial.println("The email was sent!");
      }

      
      delay(9000);
      WiFi.disconnect();
      Serial.println("Wifi disconnected");
      WiFi.mode(WIFI_OFF);
      Serial.println("Wifi off");
      moveDetected = false;
    




    
    }

delay(9000);
  Serial.println("Going to sleep now");
  delay(9000);
  
  esp_deep_sleep_start();  
  

  }

}



//Accelerometer limits
int xMin; //Minimum x Value
int xMax; //Maximum x Value
int xVal; //Current x Value

int yMin; //Minimum y Value
int yMax; //Maximum y Value
int yVal; //Current y Value

int zMin; //Minimum z Value
int zMax; //Maximum z Value
int zVal; //Current z Value

void calibrateAccel() {
  sensors_event_t event;
  accel.getEvent(&event);
  //reset alarm
  moveDetected = false;

  //initialise x,y,z variables
  xVal = event.acceleration.x;
  xMin = xVal;
  xMax = xVal;

  yVal = event.acceleration.y;
  yMin = yVal;
  yMax = yVal;

  zVal = event.acceleration.z;
  zMin = zVal;
  zMax = zVal;

  //calibration sequence initialisation - 3 led blinks
  //digitalWrite(ledPin, HIGH);

  for (int i = 0; i < 50; i++) {
    sensors_event_t event;
    accel.getEvent(&event);
    // Calibrate x Values
    xVal = event.acceleration.x;
    if (xVal > xMax) {
      xMax = xVal;
    } else if (xVal < xMin) {
      xMin = xVal;
    }
    // Calibrate y Values
    yVal = event.acceleration.y;
    if (yVal > yMax) {
      yMax = yVal;
    } else if (yVal < yMin) {
      yMin = yVal;
    }
    // Calibrate z Values
    zVal = event.acceleration.z;
    if (zVal > zMax) {
      zMax = zVal;
    } else if (zVal < zMin) {
      zMin = zVal;
    }
    delay(10);
  }
  //digitalWrite(ledPin, HIGH);

  calibrated = true;
}

//Function used to detect motio. Tolerance variable asjusts the sensitivity of movement detected.
boolean checkMotion() {
  sensors_event_t event;
  accel.getEvent(&event);
  boolean tempB = false;
  xVal = event.acceleration.x;
  yVal = event.acceleration.y;
  zVal = event.acceleration.z;

  if (xVal > (xMax + tolerance) || xVal < (xMin - tolerance)) {
    tempB = true;
    Serial.print("X Failed = ");
    Serial.println(xVal);
  }
  if (yVal > (yMax + tolerance) || yVal < (yMin - tolerance)) {
    tempB = true;
    Serial.print("Y Failed = ");
    Serial.println(yVal);
  }

  if (zVal > (zMax + tolerance) || zVal < (zMin - tolerance)) {
    tempB = true;
    Serial.print("Z Failed = ");
    Serial.println(zVal);
  }

  return tempB;
}

//Alarm Function
void ALARM() {

  //dont check for movement until recalibrated again
  calibrated = false;

//   blink LED
 // digitalWrite(ledPin, HIGH);
  //digitalWrite(ledPin, LOW);

}
