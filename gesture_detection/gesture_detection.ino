
#include <Arduino_LSM9DS1.h>

#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>

#include "model.h"

#include <EncoderStepCounter.h>


#define ROTARY_PIN1 2
#define ROTARY_PIN2 3
#define BUTTON_PIN 4
#define VIB_PIN 5
#define CLICKS_PER_STEP 4  // this number depends on your rotary encoder
int lastButtonState = LOW;
const int buttonPin = 4;  
int debounceDelay = 5; 

bool isVib = false; 
int before;
int vibIndex = 0; 

const int length = 5;
// Intensity 0-1, Length 0-, Gradual 0 or 1
float v[length][3] = {
{0.0,   2, 1},
{0.7, 0.1, 0},
{0.2, 0.1, 0},
{0.7, 0.1, 0},
{0.0, 0.1, 0}
};

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

void setup() {
  pinMode(4, INPUT_PULLUP);
  pinMode(VIB_PIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial)
    ;

  // initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }

  // print out the samples rates of the IMUs
  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");

  Serial.println();

  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
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

  Serial.println("setup end");
}

void loop() {
  float aX, aY, aZ, gX, gY, gZ;

  /*
  if(isVib)
  {
    patternVibration(); 
  }
  */
  // wait for significant motion
  while (samplesRead == numSamples) {

    // encoder reading
    encoder.tick();
  int buttonState = digitalRead(buttonPin);
  //  // if the button has changed:
  if (buttonState != lastButtonState) {
    // debounce the button:
    delay(debounceDelay);
    // if button is pressed:
    if (buttonState == LOW) {
      Serial.println("Play / Pause");
      shortVibration();
    }
  }
  lastButtonState = buttonState;
     
     
    

    int position = encoder.getPosition();
    if (position != oldPosition && position < oldPosition) {
      Serial.println(position);
      Serial.println("back");

      shortVibration();

      oldPosition = position;
    }else if (position != oldPosition && position > oldPosition){
      Serial.println(position);
      Serial.println("foward");

      shortVibration(); 

      oldPosition = position;
    }

    if (IMU.accelerationAvailable()) {
      // read the acceleration data
      IMU.readAcceleration(aX, aY, aZ);

      // sum up the absolutes
      float aSum = fabs(aX) + fabs(aY) + fabs(aZ);

      // check if it's above the threshold
      if (aSum >= accelerationThreshold) {
        // reset the sample read count
        samplesRead = 0;
        break;
      }
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
          Serial.println("Invoke failed!");
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
        Serial.println(GESTURES[max_index]);
        if(max_index == 0)
        {
          //isVib = true; 
          confirmVibration(); 
        }
        //Do Vibration patterns according to index 
        Serial.println();
      }
    }
  }
}

void shortVibration() {
  analogWrite(VIB_PIN, 255);
  delay(130);
  analogWrite(VIB_PIN, 0);
}

void confirmVibration() {
  analogWrite(VIB_PIN, 170);
  delay(130);
  analogWrite(VIB_PIN, 0);
  delay(130);
  analogWrite(VIB_PIN, 255);
  delay(130);
  analogWrite(VIB_PIN, 0);
}
/*
void patternVibration()
{
  //If element exceeded beyond vector range
  if(length+1 == vibIndex)
  {
    isVib = false; 
    return; 
  }
	//If the intensity value is between 0 and 1
	if (v[vibIndex][0] >= 0 && v[vibIndex][0] <= 1)
	{
    //Map intensity value to 0-255 and write to vib motor
    if (v[vibIndex][2] == 0) 
    {
      analogWrite(VIB_PIN, v[vibIndex][0]*255);
      Serial.println("test: " + String(v[vibIndex][0]*255));
    }
    else if(v[vibIndex][2] == 1 && vibIndex-1 != length)
    {
      
      float secLen = v[vibIndex][1];
      float perc = (millis()-before)/(secLen*1000);
      float change = (v[vibIndex+1][0]-v[vibIndex][0])* perc;
      
      analogWrite(VIB_PIN, (v[vibIndex][0]+change)*255);
      Serial.println((v[vibIndex][0]+change)*255);
    }
		
	}
	else
	{
    //Otherwise set it to none
		digitalWrite(VIB_PIN, LOW);
    Serial.println(0.0); 
	}
  //If the time has exceeded beyond the range allocated for v[i] then continue to next 
  if(millis()-before > v[vibIndex][1]*1000)
  {
    before = millis();
    vibIndex++; 
  }
}
*/
// // on change
// void rotate(Rotary& r) {
//   Serial.println(r.getPosition());
// }

// // on left or right rotation
// void showDirection(Rotary& r) {
//   Serial.print("direção");
//   int direction = r.getDirection();
//   Serial.println(direction);

//   if (direction == 1) {
//     Serial.println("left_arrow");
//     shortVibration();

//     delay(100);
//     digitalWrite(LED_BUILTIN, LOW);
//   } else if (direction == 255) {
//     Serial.println("right_arrow");
//     shortVibration();
//   }
// }

// // single click
// void click(Button2& btn) {
//   Serial.println("Click!");
//   shortVibration();
// }



// // long click
// void resetPosition(Button2& btn) {
//   r.resetPosition();
//   Serial.println("Reset!");
// }

