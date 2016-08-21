#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
TinyGPSPlus gps; // gps object

Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

// 12v LED Strip Light Definitions
#define REDPIN 5
#define GREENPIN 6
#define BLUEPIN 3
#define FADESPEED 8     // make this higher to slow down

// SD Card configuration
#include <SPI.h>
#include <SD.h>

const int chipSelect = 10; //
File myLocationFile;
char curLocationFile[16]; // array for file name using char types no more than 16 characters long
double doubleDestLong;
double doubleDestLat;
unsigned long last = 0UL;

int Mag_adjustment()
{
  int result;

  sensors_event_t event;
  mag.getEvent(&event);
  float heading = atan2(event.magnetic.x, event.magnetic.y);
  float declinationAngle = -0.19;
  heading += declinationAngle;

  //corrections
  if(heading < 0)
    heading += 2*PI;

  if(heading > 2*PI)
    heading -= 2*PI;

  float headingDegrees = heading * 180/M_PI;
  return headingDegrees;
}

void setup()
{
  Serial.begin(9600);
  Serial.print("Reading SD Card...");

  if (!SD.begin(10)) { // Notifies user if it can't start the SD card
    Serial.println("initialization failed!");
    Serial.println("Did you plug the card back in?");
    return;
  }
  Serial.println("Lets rock and roll!");

  // Get the longitude from a file and turn it into a double type
  char buff[16];  // may not need this
  char DestLong[16]; // array to fill up with bytes from the file for Longitude
  int i; // defines i as an integer type
  myLocationFile = SD.open("camplon.txt"); // opens the file itself


  /* for-loop for reading the first byte ( in the "0"th place) in the file 'camplon.txt' and
      assigning it as the first value in the array DestLong.
      As noted on line 14, DestLong was declared to an array (group)
      of values tied to that single name. [16] states that there are 16
      of the values.  Each value is of the character type, each only
      taking up a single byte. This for-loop is quite common in reading
      data from sources that 'stream' such as a file seen above.  Each time
      the 'myLocationFile.read() is executed, it moves to the next byte.
  */
  for (int i = 0; i < 12; i++) {
    DestLong[i] = myLocationFile.read();
  }
  char *lat2; // no idea why this is here
  doubleDestLong = atof(DestLong);
  Serial.print("Imported longitude from camplon.txt:   ");
  Serial.println(doubleDestLong, 6);
  myLocationFile.close();
  /* the other for-loop that now that I'm thinking about I can probably rewrite to eliminate the need for a second file
      simply by making sure all of my coordinates are the say, 12 characters long, that way I can just put
      for(int i=12; i<23; i++) etc etc
   *  ***********
      LAWLNO
  */
  myLocationFile = SD.open("camplat.txt");
  char DestLat[16]; // array to fill up with bytes from the file for Latitude
  for (int i = 0; i < 12; i++) {
    DestLat[i] = myLocationFile.read();
  }
  char *lon2; //still no idea... will see try to remove after the rewrite
  doubleDestLat = atof(DestLat);
  Serial.print("Imported latitude from camplat.txt:   ");
  Serial.println(doubleDestLat, 6);
  myLocationFile.close();
  delay(5000);//close file

  if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }

  // LED Setup
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
}

void loop()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial.available() > 0)
    if (gps.encode(Serial.read()))
      displayInfo();
      Serial.println(Mag_adjustment());
      delay(2500);

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }
}

void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  if (millis() - last > 1000)
  {
    Serial.println();
    if (gps.location.isValid())
    {
      static const double CAMP_LAT = doubleDestLat, CAMP_LON = doubleDestLong;

      
      Serial.println("Using TinyGPS++:");
      
      
      Serial.println(gps.courseTo(CAMP_LAT, CAMP_LON,gps.location.lat(), gps.location.lng()));
      Serial.println(gps.courseTo(gps.location.lat(), gps.location.lng(),CAMP_LAT, CAMP_LON));
      
      
      Serial.print(F(" degrees ["));
      Serial.println(F("]"));

      if (gps.charsProcessed() < 10)
        Serial.println(F("WARNING: No GPS data.  Check wiring."));

        last = millis();
        Serial.println();
      }
  }
}

