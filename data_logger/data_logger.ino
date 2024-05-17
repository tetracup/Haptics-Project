
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

  Wire.begin();
  mpu.initialize();
  
r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);

  b.begin(BUTTON_PIN);
  b.setTapHandler(click);
  b.setLongClickHandler(resetPosition);

 

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {

    // don't do anything more:
    while (1);
  }
  SD.remove("datalog.csv");
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
    dataFile.print("gyroZ");
    dataFile.print(",");
    //Added
    dataFile.print("Click");
    dataFile.print(",");
    dataFile.print("RotDir");
    dataFile.print(",");
    dataFile.print("RotVal");
    dataFile.print(",");
    dataFile.println("Time");
    //
    dataFile.close();
  }
}
bool clickBool = false; 
String rotDir = "NA"; 
int rotVal = 0; 
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
  
  int csvI = 0;
  while(SD.exists("datalog" + String(csvI) + ".csv"))
  {
    csvI++;
  }
  File dataFile = SD.open("datalog" + String(csvI) + ".csv", FILE_WRITE);
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
    dataFile.print(gyroZ);
    //Added
    dataFile.print(",");
    dataFile.print(clickBool);
    dataFile.print(",");
    dataFile.print(rotDir);
    dataFile.print(",");
    dataFile.print(rotVal);
    dataFile.print(",");
    dataFile.println(millis());
    dataFile.close();
    clickBool = false; 
  }
  // if the file isn't open, pop up an error:
  else {

  }
    }
  }
}

void rotate(Rotary& r) {

  rotVal = r.getPosition(); 
}

// on left or right rotation
void showDirection(Rotary& r) {

  rotDir = r.directionToString(r.getDirection()); 
}
 
// single click
void click(Button2& btn) {

  clickBool = true; 
}

// long click
void resetPosition(Button2& btn) {
  r.resetPosition();
  clickBool = true; 
}
