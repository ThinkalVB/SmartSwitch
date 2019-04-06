#include <BH1750FVI.h>
#include <HM10.h>

class PIRsensor
{
	/* 
	foundHuman		- A bool flag to detect if a human is thier or not
	signalPin		- The pin number to which the PIR signal pin is connected with 
	lastLowTrigger	- The last time the system detected a low trigger in millis()

	PIRsensor(signalPinNumber)		- The pin to which the singal pin from PIR sensor is conneceted with 
	foundHumanPresensce				- Return true if Human motion detected 
	lastLowTriggerTime()			- Returns the last time when a low trigger was detected 
	update()						- Must be called at every iteration in loop 
	*/

	bool foundHuman;				
	const uint8_t signalPin;
	unsigned long lastLowTrigger;
public:
	PIRsensor(uint8_t signalPinNumber) : signalPin(signalPinNumber)
	{
		foundHuman = false;
		lastLowTrigger = 0;
		pinMode(signalPin, INPUT);
	}

	unsigned long lastLowTriggerTime()
	{
		return lastLowTrigger;
	}

	bool foundHumanPresence()
	{
		return foundHuman;
	}

	void update()
	{
		if (digitalRead(signalPin) == HIGH)
		{
			foundHuman = true;
			lastLowTrigger = 0;
		}
		else if(lastLowTrigger == 0)
		{
			foundHuman = false;
			lastLowTrigger = millis();
		}
	}
};

class Switch
{
	/*
	minTolerableIntensity			- Minimum intensity to which the switch reamins off 
	timeDelay						- After PIR goes LOW, for how long it must wait before turning the switch off 
	singalPin						- Pin number to which the signal pin of the relay module is connected with 

	Switch(signalPinNumber)			- The pin to which the singal pin from the realy module is conneceted with
	setMinimumIntensity(intensity)	- Set the minimum tolerable intensity for that particular switch
	setTimeDelay(dealy)				- Set the time delay
	turnOn()						- Turn on the switch
	turnOff()						- Turn off the switch
	tryturnOn(lightIntensity)		- Turn on the switch when the intensity exceeds the threshold
	tryTurnOff(lastLowTrigger)		- Turn off only after the time delay
	*/

	uint16_t minTolerableIntensity;
	unsigned long timeDelay;
	const uint8_t signalPin;

public:

	void turnOn()
	{
		digitalWrite(signalPin, HIGH);
	}

	void turnOff()
	{
		digitalWrite(signalPin, LOW);
	}

	Switch(uint8_t signalPinNumber) : signalPin(signalPinNumber)
	{
		minTolerableIntensity = 10;
		timeDelay = 10000;
		pinMode(signalPin, OUTPUT);
	}

	void setTimeDelay(unsigned long delay)
	{
		timeDelay = delay;
	}

	void setMinimumIntensity(uint16_t intensity)
	{
		minTolerableIntensity = intensity;
	}

	void tryTurnOn(uint16_t lightIntensity)
	{
		if (digitalRead(signalPin) == LOW && lightIntensity < minTolerableIntensity)
			turnOn();
	}

	void tryTurnOff(unsigned long lastLowTrigger)
	{
		if (digitalRead(signalPin) == HIGH && lastLowTrigger + timeDelay < millis())
			turnOff();
	}
};

  PIRsensor pirSensor(2);
  Switch led(13);
  BH1750FVI LightSensor(BH1750FVI:: k_DevModeContHighRes2);
  HM10 Bluetooth(10,11);
bool isOverriding = false;

void setup() {
  Serial.begin(9600);
	LightSensor.begin();
}

void loop() {


  Serial.println(LightSensor.GetLightIntensity());
  Serial.println(digitalRead(2));
    
  if(!isOverriding)
	{
		pirSensor.update();
		if (pirSensor.foundHumanPresence())
			led.tryTurnOn(LightSensor.GetLightIntensity());
		else
			led.tryTurnOff(pirSensor.lastLowTriggerTime());
	}

	Bluetooth.readData();
	if(!Bluetooth.bufferEmpty())
	{
		if (Bluetooth.data == "ON")
		{
			led.turnOn();
			isOverriding = true;
		}
		else if(Bluetooth.data == "OFF")
		{
			led.turnOff();
			isOverriding = false;
		}
		Bluetooth.clearBuffer();
	}
  delay(250); 
}
