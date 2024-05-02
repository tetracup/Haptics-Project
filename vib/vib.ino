using namespace std; 
int motorPin = 3; //motor transistor is connected to pin 3
int before;
const int length = 6; 
void setup()
{
  Serial.begin(9600);
  pinMode(motorPin, OUTPUT);
  before = millis();
  analogWrite(motorPin, 0);
}
float v[length][2] = {
{0.5,1},
{0.6,1},
{0.7,1},
{0.8,1}
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
	if (v[i][0] > 0 && v[i][0] <= 1)
	{
    //Map intensity value to 0-255 and write to vib motor
    
		analogWrite(motorPin, v[i][0]*255);
	}
	else
	{
    //Otherwise set it to none
		analogWrite(motorPin, 0);
	}
  //If the time has exceeded beyond the range allocated for v[i] then continue to next 
  if(millis()-before > v[i][1]*1000)
  {
    before = millis();
    i++; 
  }
}
