/*
  SD card datalogger

  This example shows how to log data from three analog sensors
  to an SD card using the SD library.

  The circuit:
   analog sensors on analog ins 0, 1, and 2
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created  24 Nov 2010
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <MPU6050.h>
MPU6050 mpu;

const int chipSelect =10;

void setup() {
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
}

void loop() {

    // Read accelerometer data
  int16_t accX, accY, accZ;
  mpu.getAcceleration(&accX, &accY, &accZ);
  
  // Read gyroscope data
  int16_t gyroX, gyroY, gyroZ;
  mpu.getRotation(&gyroX, &gyroY, &gyroZ);
  
    // Calculate tilt angles
  float tiltX = atan2(accY, accZ) * 180 / M_PI;
  float tiltY = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180 / M_PI;
  float tiltZ = atan2(accZ, accX) * 180 / M_PI; // Gyroscope data used for tiltZ
  
  // Check if tilt angles are finite
  if (!isnan(tiltX) && isfinite(tiltX) && !isnan(tiltY) && isfinite(tiltY) && !isnan(tiltZ) && isfinite(tiltZ)) {
    // Calculate acceleration in all three angles
    float accAngleX = atan2(accX, sqrt(accY * accY + accZ * accZ)) * 180 / M_PI;
    float accAngleY = atan2(accY, sqrt(accX * accX + accZ * accZ)) * 180 / M_PI;
    float accAngleZ = atan2(sqrt(accX * accX + accY * accY), accZ) * 180 / M_PI;
    
    // Check if acceleration angles are finite
    if (!isnan(accAngleX) && isfinite(accAngleX) && !isnan(accAngleY) && isfinite(accAngleY) && !isnan(accAngleZ) && isfinite(accAngleZ)) {
      // Calculate acceleration magnitude
      float accMagnitude = sqrt(accX * accX + accY * accY + accZ * accZ);



  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.print(tiltX);
    dataFile.print(",");
    dataFile.print(tiltY);
    dataFile.print(",");
    dataFile.print(tiltZ);
    dataFile.print(accAngleX);
    dataFile.print(",");
    dataFile.print(accAngleY);
    dataFile.print(",");
    dataFile.println(accAngleZ);
    dataFile.close();
    // print to the serial port too:
    Serial.print(tiltX);
    Serial.print(tiltY);
    Serial.print(tiltZ);
    Serial.print(accAngleX);
    Serial.print(accAngleY);
    Serial.println(accAngleZ);

  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
    }
  }
}
