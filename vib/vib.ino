//https://techzeero.com/arduino-tutorials/vibration-motor-with-arduino/

using namespace std; 
int motorPin = 3; //motor transistor is connected to pin 3
int before;
const int length = 11; 
void setup()
{
  Serial.begin(9600);
  pinMode(motorPin, OUTPUT);
  before = millis();
}
float v[length][3] = {
{0.1,4,1},
{0.6,2,1},
{0.3,2,1},
{0.8,4,1},
{0, 2, 1},
{0.7, 0.1, 1},
{0, 2, 1},
{0.7, 0.1, 1},
{0, 2, 1},
{0.7, 0.1, 1},
{0, 0.1, 0}
};

int i = 0; 

void loop()
{
  
  //If element exceeded beyond vector range
  if(length+1 == i)
  {
    return; 
  }
	//If the intensity value is between 0 and 1
	if (v[i][0] >= 0 && v[i][0] <= 1)
	{
    //Map intensity value to 0-255 and write to vib motor
    	if (v[i][2] == 0) 
        {
          analogWrite(motorPin, v[i][0]*255);
          Serial.println(v[i][0]*255);
        }
      	else if(v[i][2] == 1 && i-1 != length)
        {
         
          float secLen = v[i][1];
          float perc = (millis()-before)/(secLen*1000);
          float change = (v[i+1][0]-v[i][0])* perc;
          
          analogWrite(motorPin, (v[i][0]+change)*255);
          Serial.println((v[i][0]+change)*255);
        }
		
	}
	else
	{
    //Otherwise set it to none
		digitalWrite(motorPin, LOW);
     	Serial.println(0.0); 
	}
  //If the time has exceeded beyond the range allocated for v[i] then continue to next 
  if(millis()-before > v[i][1]*1000)
  {
    before = millis();
    i++; 
  }
}
