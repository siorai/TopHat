/*
  Tilt compensated HMC5883L + ADXL345 (GY-80). Output for HMC5883L_compensation_processing.pde
  Read more: http://www.jarzebski.pl/arduino/czujniki-i-sensory/3-osiowy-magnetometr-hmc5883l.html
  GIT: https://github.com/jarzebski/Arduino-HMC5883L
  Web: http://www.jarzebski.pl
  (c) 2014 by Korneliusz Jarzebski
*/
#include <TinyGPS++.h>
#include <Wire.h>
#include <HMC5883L.h>
#include <ADXL345.h>
#include <SPI.h>
#include <SD.h>

const int chipSelect = 10; //
File myLocationFile;
char curLocationFile[16];
double doubleDestLong;
double doubleDestLat;
unsigned long last = 0UL;

HMC5883L compass;
ADXL345 accelerometer;
TinyGPSPlus gps;

float heading1;
float heading2;

void setup()
{
  Serial.begin(9600);

  // Initialize ADXL345

  if (!accelerometer.begin())
  {
    delay(500);
  }

  accelerometer.setRange(ADXL345_RANGE_2G);

  // Initialize Initialize HMC5883L
  while (!compass.begin())
  {
    delay(500);
  }

  // Set measurement range
  compass.setRange(HMC5883L_RANGE_1_3GA);

  // Set measurement mode
  compass.setMeasurementMode(HMC5883L_CONTINOUS);

  // Set data rate
  compass.setDataRate(HMC5883L_DATARATE_30HZ);

  // Set number of samples averaged
  compass.setSamples(HMC5883L_SAMPLES_8);

  // Set calibration offset. See HMC5883L_calibration.ino
  compass.setOffset(86, -351); 

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
  double doubleDestLong = atof(DestLong);
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
  Serial.println(doubleDestLat);
  myLocationFile.close();
  delay(5000);//close file
}

// No tilt compensation

float noTiltCompensate(Vector mag)
{
  float heading = atan2(mag.YAxis, mag.XAxis);
  return heading;
}
 
// Tilt compensation
float tiltCompensate(Vector mag, Vector normAccel)
{
  // Pitch & Roll 

  float roll;
  float pitch;

  roll = asin(normAccel.YAxis);
  pitch = asin(-normAccel.XAxis);

  if (roll > 0.78 || roll < -0.78 || pitch > 0.78 || pitch < -0.78)
  {
    return -1000;
  }

  // Some of these are used twice, so rather than computing them twice in the algorithem we precompute them before hand.
  float cosRoll = cos(roll);
  float sinRoll = sin(roll);  
  float cosPitch = cos(pitch);
  float sinPitch = sin(pitch);

  // Tilt compensation
  float Xh = mag.XAxis * cosPitch + mag.ZAxis * sinPitch;
  float Yh = mag.XAxis * sinRoll * sinPitch + mag.YAxis * cosRoll - mag.ZAxis * sinRoll * cosPitch;

  float heading = atan2(Yh, Xh);

  return heading;
}

// Correct angle
float correctAngle(float heading)
{
  if (heading < 0) { heading += 2 * PI; }
  if (heading > 2 * PI) { heading -= 2 * PI; }

  return heading;
}

void loop()
{
  // Read vectors
  Vector mag = compass.readNormalize();
  Vector acc = accelerometer.readScaled();

  // Calculate headings
  heading1 = noTiltCompensate(mag);
  heading2 = tiltCompensate(mag, acc);

  if (heading2 == -1000)
  {
    heading2 = heading1;
  }

  // Set declination angle on your location and fix heading
  // You can find your declination on: http://magnetic-declination.com/
  // (+) Positive or (-) for negative
  // For Bytom / Poland declination angle is 4'26E (positive)
  // Formula: (deg + (min / 60.0)) / (180 / M_PI);
  float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / M_PI);
  heading1 += declinationAngle;
  heading2 += declinationAngle;

  // Correct for heading < 0deg and heading > 360deg
  heading1 = correctAngle(heading1);
  heading2 = correctAngle(heading2);

  // Convert to degrees
  heading1 = heading1 * 180/M_PI; 
  heading2 = heading2 * 180/M_PI; 

  // Output
  Serial.print(heading1);
  Serial.print(":");
  Serial.println(heading2);
  delay(100);

{
  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial.available() > 0)
    if (gps.encode(Serial.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
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

      Serial.println("Correct raw GPS heading to compass data:");
  heading2 += gps.courseTo(gps.location.lat(), gps.location.lng(),CAMP_LAT, CAMP_LON);
  if (heading2 > 360) heading2 = heading2 - 360;
  Serial.print(heading2);
  Serial.println(" degrees*******");
    }
  }
}
