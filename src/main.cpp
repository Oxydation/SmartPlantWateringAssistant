#include <Arduino.h>
// This example shows how to connect to Cayenne using an ESP32 and send/receive sample data.
// Make sure you install the ESP32 Board Package and select the correct ESP32 board before compiling.
// To install the ESP32 Board Package follow the instructions here: https://github.com/espressif/arduino-esp32/blob/master/README.md#installation-instructions.

//#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP32.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi network info.
char ssid[] = "Gate";
char wifiPassword[] = "E60035CF";

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "d6913b20-8ad5-11e9-94e9-493d67fd755e";
char password[] = "b2bfdf6fa920c6d2befce9ef776a13c487a7734e";
char clientID[] = "f59b7560-8b63-11e9-be3b-372b0d2759ae";

unsigned long lastMillis = 0;

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 27
#define PUMP_PIN 14
#define CURRENT_SENS_PIN 34

const int led2PwmPin = 12;
const int ledChannel = 0;
const int resolution = 8;
const int freq = 5000;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress thermometer;

void setup() {
	Serial.begin(115200);
	Cayenne.begin(username, password, clientID, ssid, wifiPassword);
  sensors.begin();

  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(led2PwmPin, ledChannel);

  pinMode(PUMP_PIN, OUTPUT);  
  digitalWrite(PUMP_PIN, 0);

 if (!sensors.getAddress(thermometer, 0)) Serial.println("Unable to find address for Device 0"); 
}

void loop() {
	Cayenne.loop();
}

void readTempSensor(){     
  sensors.requestTemperatures();
  double temp = sensors.getTempCByIndex(0);
  if(!(temp == 85.00 || temp == (-127.0))){
 	Cayenne.celsiusWrite(1, sensors.getTempCByIndex(0)); 
  }
}

// Default function for sending sensor data at intervals to Cayenne.
// You can also use functions for specific channels, e.g CAYENNE_OUT(1) for sending channel 1 data.
CAYENNE_OUT_DEFAULT()
{
	// Write data to Cayenne here. This example just sends the current uptime in milliseconds on virtual channel 0.
	//Cayenne.virtualWrite(0, millis());
	// Some examples of other functions you can use to send data.
	readTempSensor();
  Cayenne.virtualWrite(4, analogRead(CURRENT_SENS_PIN), UNIT_MILLIVOLTS);

	//Cayenne.luxWrite(2, 700);
	//Cayenne.virtualWrite(3, 50, TYPE_PROXIMITY, UNIT_CENTIMETER);
}

// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT()
{
	CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
	//Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}

CAYENNE_IN(2){
  digitalWrite(PUMP_PIN, !getValue.asInt());
}

CAYENNE_IN(3){
  ledcWrite(ledChannel, getValue.asInt());
}