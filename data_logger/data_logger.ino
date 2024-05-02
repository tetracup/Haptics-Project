
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <MPU6050.h>

#include "Button2.h"; //  https://github.com/LennartHennigs/Button2
#include "Rotary.h";

#define ROTARY_PIN1	5
#define ROTARY_PIN2	6
#define BUTTON_PIN	7

#define CLICKS_PER_STEP   4   // this number depends on your rotary encoder 


MPU6050 mpu;
Rotary r;
Button2 b;

const int chipSelect =10;

void setup() {
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);

  b.begin(BUTTON_PIN);
  b.setTapHandler(click);
  b.setLongClickHandler(resetPosition);

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
  File dataFile = SD.open("datalog.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.print("tiltX");
    dataFile.print(",");
    dataFile.print("tiltY");
    dataFile.print(",");
    dataFile.print("tiltZ");
     dataFile.print(",");
    dataFile.print("accAngleX");
    dataFile.print(",");
    dataFile.print("accAngleY");
    dataFile.print(",");
    dataFile.print("accAngleZ");
    dataFile.print(",");
    dataFile.print("gyroX");
    dataFile.print(",");
    dataFile.print("gyroY");
    dataFile.print(",");
    dataFile.println("gyroZ");
    dataFile.close();
  }
}

void loop() {

 r.loop();
  b.loop();
  
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
     dataFile.print(",");
    dataFile.print(accAngleX);
    dataFile.print(",");
    dataFile.print(accAngleY);
    dataFile.print(",");
    dataFile.print(accAngleZ);
    dataFile.print(",");
    dataFile.print(gyroX);
    dataFile.print(",");
    dataFile.print(gyroY);
    dataFile.print(",");
    dataFile.println(gyroZ);
    dataFile.close();
    // print to the serial port too:
    Serial.print(tiltX);
    Serial.print(tiltY);
    Serial.print(tiltZ);
    Serial.print(accAngleX);
    Serial.print(accAngleY);
    Serial.print(accAngleZ);
    Serial.print(gyroX);
    Serial.print(gyroY);
    Serial.println(gyroZ);

  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.csv");
  }
    }
  }
}

void rotate(Rotary& r) {
   Serial.println(r.getPosition());
    
     File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.println(r.getPosition());
    dataFile.close();
}

}

// on left or right rotation
void showDirection(Rotary& r) {
  Serial.println(r.directionToString(r.getDirection()));

       File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
     dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.println(r.directionToString(r.getDirection()));
    dataFile.close();
}
}
 
// single click
void click(Button2& btn) {
  Serial.println("Click!");
        File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
     dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.println("Click!");
    dataFile.close();
}
}

// long click
void resetPosition(Button2& btn) {
  r.resetPosition();
  Serial.println("Reset!");

          File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
     dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.print(" ");
    dataFile.print(",");
    dataFile.println("Long click");
    dataFile.close();
}
}
