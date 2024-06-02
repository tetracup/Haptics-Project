#include <Arduino_LSM9DS1.h>
#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>
#include "model.h"
#include <EncoderStepCounter.h>
#include "mbed.h"
#include "USBKeyboard.h"


#define ROTARY_PIN1 2
#define ROTARY_PIN2 3
#define BUTTON_PIN 4
#define VIB_PIN 5
#define CLICKS_PER_STEP 4  // this number depends on your rotary encoder

int lastButtonState = LOW;
int lastaXt = 0;
int lastaYt = 0;

const int buttonPin = 4;
int debounceDelay = 5;
int degreesY = 0;
int degreesX = 0;
float aXt = 0;
float aYt = 0;
float aZy = 0;

//For Vibration code
int rotateFactor = 0;
bool tiltReset = false;

// rotary encoder setting
EncoderStepCounter encoder(ROTARY_PIN1, ROTARY_PIN2);
int oldPosition = 0;

const float accelerationThreshold = 2.5;  // threshold of significant in G's
const int numSamples = 119;

int samplesRead = numSamples;

// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

// array to map gesture index to a name
const char* GESTURES[] = {
  "shuffle",
  "up",
  "tilt_left",
  "tilt_right"
};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))


// USBkeyboard setting
USBKeyboard key;
int atexit(void (* /*func*/)()) {
  return 0;
}
void initVariant() __attribute__((weak));
void initVariant() {}
void setupUSB() __attribute__((weak));
void setupUSB() {}


void setup() {

  pinMode(4, INPUT_PULLUP);
  pinMode(VIB_PIN, OUTPUT);
  // Serial.begin(9600);

  // initialize the IMU
  if (!IMU.begin()) {
    // Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }
  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    // Serial.println("Model schema mismatch!");
    while (1)
      ;
  }
  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);
  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();
  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);

  // encoder previous position:
  encoder.begin();

  // Serial.println("setup end");
}

float aX, aY, aZ, gX, gY, gZ, aSum, gSum;
float maxASum = 1.5;
float maxGSum = 15.2;

void loop() {

  // wait for significant motion
  while (samplesRead == numSamples) {

    IMU.readAcceleration(aXt, aYt, aZy);
    IMU.readAcceleration(aX, aY, aZ);
    IMU.readGyroscope(gX, gY, gZ);

    aSum = fabs(aX) + fabs(aY) + fabs(aZ);
    gSum = fabs(gX) + fabs(gY) + fabs(gZ);


    //Tilt Vibration
    detectVibrations();

    // encoder reading
    encoder.tick();
    int buttonState = digitalRead(buttonPin);
    //  // if the button has changed:
    if (buttonState != lastButtonState) {
      // debounce the button:
      delay(debounceDelay);
      // if button is pressed:
      if (buttonState == LOW) {
        // Serial.println("Play / Pause");
        key.media_control(KEY_PLAY_PAUSE);
        shortVibration();
      }
    }
    lastButtonState = buttonState;

    int position = encoder.getPosition();
    if (position != oldPosition && position < oldPosition) {
      // Serial.println(position);
      // Serial.println("back");
      rotateFactor = rotateFactor + (position - oldPosition);
      RotateVibration((float)rotateFactor / 4.0f);
      oldPosition = position;

    } else if (position != oldPosition && position > oldPosition) {
      // Serial.println(position);
      // Serial.println("forward");
      rotateFactor = rotateFactor + (position - oldPosition);
      RotateVibration((float)rotateFactor / 4.0f);
      oldPosition = position;
    }
    // check if it's above the threshold
    if (aSum >= accelerationThreshold) {
      // reset the sample read count
      samplesRead = 0;
      break;
    }
  }
  // check if the all the required samples have been read since
  // the last time the significant motion was detected
  while (samplesRead < numSamples) {
    // check if new acceleration AND gyroscope data is available
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      // read the acceleration and gyroscope data
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);


      // normalize the IMU data between 0 to 1 and store in the model's
      // input tensor
      tflInputTensor->data.f[samplesRead * 6 + 0] = (aX + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead * 6 + 1] = (aY + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead * 6 + 2] = (aZ + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead * 6 + 3] = (gX + 2000.0) / 4000.0;
      tflInputTensor->data.f[samplesRead * 6 + 4] = (gY + 2000.0) / 4000.0;
      tflInputTensor->data.f[samplesRead * 6 + 5] = (gZ + 2000.0) / 4000.0;

      samplesRead++;

      if (samplesRead == numSamples) {
        // Run inferencing
        TfLiteStatus invokeStatus = tflInterpreter->Invoke();
        if (invokeStatus != kTfLiteOk) {
          // Serial.println("Invoke failed!");
          while (1)
            ;
          return;
        }
        // Loop through the output tensor values from the model
        int max_index = 0;
        float max_value = 0;
        for (int i = 0; i < NUM_GESTURES; i++) {
          // Serial.print(GESTURES[i]);
          // Serial.print(": ");
          // Serial.println(tflOutputTensor->data.f[i], 6);
          if (tflOutputTensor->data.f[i] > max_value) {
            max_index = i;
            max_value = tflOutputTensor->data.f[i];
          }
        }
        // Serial.println(GESTURES[max_index]);
        //Choose which vibration pattern to perform on gesture
        switch (max_index) {
          case 0:
            shuffleVibration();
            break;

          case 1:
            upVibration();
            break;
        }
        //Do Vibration patterns according to index
      }
    }
  }
}

//Vibrations
// You should change the number to adjust the sensitibity here
void detectVibrations() {
  if (aYt > 0.3 && aYt != lastaYt && gSum < maxGSum && aSum < maxASum) {
    tiltReset = false;
    aYt = 100 * aYt;
    degreesY = map(aYt, 0, 97, 0, 90);
    tiltVibration(degreesY);
    lastaYt = aYt;
    key.media_control(KEY_VOLUME_UP);
    delay(10);
  } else if (aYt < -0.3 && gSum < maxGSum && aSum < maxASum) {
    tiltReset = false;
    aYt = 100 * aYt;
    degreesY = map(aYt, 0, -100, 0, 90);
    tiltVibration(degreesY);
    lastaYt = aYt;
    key.media_control(KEY_VOLUME_DOWN);
    delay(10);

  }

  if (aYt > -0.3 && aYt < 0.3 && !tiltReset) {
    tiltReset = true;
    analogWrite(VIB_PIN, 0);
  }
}
void shortVibration() {
  analogWrite(VIB_PIN, 255);
  delay(130);
  analogWrite(VIB_PIN, 0);
}

void RotateVibration(float inputIntensity) {
  // Serial.println(Intensity);
  float Intensity = abs(inputIntensity);
  if (Intensity > 0 && Intensity < 1) {
    analogWrite(VIB_PIN, 255 * Intensity);
    //analogWrite(VIB_PIN, 255);
    //delay(70); 
    //analogWrite(VIB_PIN, 0); 
    //delay(70); 
  } else if (Intensity >= 1) {
    if(inputIntensity > 0)
    {
      key.media_control(KEY_PREVIOUS_TRACK);  
      delay(10);
      key.media_control(KEY_PREVIOUS_TRACK);  
    }
    else if(inputIntensity < 0)
    {
      key.media_control(KEY_NEXT_TRACK);
    }
    rotateFactor = 0;
    analogWrite(VIB_PIN, 0);
  }
}

void tiltVibration(float degrees) {
  if (degrees > -45 || degrees < 45) {
    //analogWrite(VIB_PIN, 255 * (abs(degrees) / 45));
    analogWrite(VIB_PIN, 255);
    delay(70); 
    analogWrite(VIB_PIN, 0); 
    delay(70);
  }
}

void shuffleVibration() {
  key.key_code('s', 0x01);
  delay(10);
  key.media_control(KEY_NEXT_TRACK);
  delay(10);
  key.key_code('s', 0x01);
  analogWrite(VIB_PIN, 255);
  delay(140);
  analogWrite(VIB_PIN, 0);
  delay(70);
  analogWrite(VIB_PIN, 255);
  delay(140);
  analogWrite(VIB_PIN, 0);
  delay(70);
  analogWrite(VIB_PIN, 255);
  delay(140);
  analogWrite(VIB_PIN, 0);
}

void upVibration() {
  analogWrite(VIB_PIN, 255);
  delay(400);
  analogWrite(VIB_PIN, 0);
}


// This is needed for USBkeyboard
int main(void) {
  init();
  initVariant();
#if defined(USBCON)
  USBDevice.attach();
#endif
  setup();
  for (;;) {
    loop();
    if (serialEventRun) serialEventRun();
  }
  return 0;
}