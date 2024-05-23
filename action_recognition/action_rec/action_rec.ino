
#include "Button2.h"; //  https://github.com/LennartHennigs/Button2
#include "Rotary.h";

#define ROTARY_PIN1	2
#define ROTARY_PIN2	3
#define BUTTON_PIN	4
#define CLICKS_PER_STEP   4   // this number depends on your rotary encoder 
#include <Arduino_LSM9DS1.h>


Rotary r;
Button2 b;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  delay(50);
  
  r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);

  b.begin(BUTTON_PIN);
  b.setTapHandler(click);
  b.setLongClickHandler(resetPosition);
}

void loop() {

  
  r.loop();
  b.loop();

  
}

/////////////////////////////////////////////////////////////////

// on change
void rotate(Rotary& r) {
int position =  r.getPosition();
   Serial.println(position);
}

// on left or right rotation
void showDirection(Rotary& r) {
 Serial.println(r.directionToString(r.getDirection()));

}
 
// single click
void click(Button2& btn) {
 Serial.println("Play / Pause"); 
}

// long click
void resetPosition(Button2& btn) {
  r.resetPosition();
  Serial.println("Reset!");
}

/////////////////////////////////////////////////////////////////
