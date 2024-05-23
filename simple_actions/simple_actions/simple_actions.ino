/////////////////////////////////////////////////////////////////

#include "Button2.h"; //  https://github.com/LennartHennigs/Button2
#include "Rotary.h";
#include <Keyboard.h>

/////////////////////////////////////////////////////////////////

#define ROTARY_PIN1	2
#define ROTARY_PIN2	3
#define BUTTON_PIN	4
#define KEY_SPACE 0x2C

#define CLICKS_PER_STEP   4   // this number depends on your rotary encoder 


/////////////////////////////////////////////////////////////////

Rotary r;
Button2 b;

/////////////////////////////////////////////////////////////////

void setup() {
    pinMode(5, OUTPUT);
  Serial.begin(9600);
  delay(50);
  Serial.println("\n\nSimple Counter");
  
  r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);

  b.begin(BUTTON_PIN);
  b.setTapHandler(click);
  b.setLongClickHandler(resetPosition);
  Keyboard.begin();
}

void loop() { 
  r.loop();
  b.loop();
}

/////////////////////////////////////////////////////////////////

// on change
void rotate(Rotary& r) {
   Serial.println(r.getPosition());
}

// on left or right rotation
void showDirection(Rotary& r) {
  Serial.print("direção");
  int direction = r.getDirection();
  Serial.println(direction);

 if(direction  == 1){
    Keyboard.press(KEY_LEFT_GUI); 
    Keyboard.press(KEY_LEFT_ARROW);
    Keyboard.releaseAll();  
    shortVibration();
   
  delay(100);                      
  digitalWrite(LED_BUILTIN, LOW);    
  } else if(direction == 255){
    Keyboard.press(KEY_LEFT_GUI); 
    Keyboard.press(KEY_RIGHT_ARROW);
    Keyboard.releaseAll();
    shortVibration();
    
  }
}
 
// single click
void click(Button2& btn) {
  Serial.println("Click!");
Keyboard.write(' ');
shortVibration();
 
                    
}
 


// long click
void resetPosition(Button2& btn) {
  r.resetPosition();
  Serial.println("Reset!");
}

/////////////////////////////////////////////////////////////////

void shortVibration(){
   analogWrite(5, 255); 
  delay(80);                      
  analogWrite(5, 0);
}
