/*
  Analog input, analog output, serial output

  Reads an analog input pin, maps the result to a range from 0 to 255 and uses
  the result to set the pulse width modulation (PWM) of an output pin.
  Also prints the results to the Serial Monitor.

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/AnalogInOutSerial
*/


// These constants won't change. They're used to give names to the pins used:
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 9; // Analog output pin that the LED is attached to

int sensorValue = 0;        // value read from the pot
float voltage = 0;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

void loop() {
  // read the analog in value:
  sensorValue = analogRead(analogInPin);
 
 // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  voltage = sensorValue * (5/1023);
 
  // print the results to the Serial Monitor:
   Serial.println(voltage);

  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  delay(2); 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add smoothing if useful 
// add sd card data transfer-- below example code to tweak for sEMG
// code from http://scholarslab.org/makerspace/saving-arduino-sensor-data/
/*
// SPI and SD libraries. SPI for connecting SD card to SPI bus.
#include <SPI.h>
#include <SD.h>
const int sdPin = 4;
 
// Temperature pin set to analog 0
const int temPin = 0;
 
// Delay time. How often to take a temperature reading, in miliseconds
// 20 minutes = 1200000 milliseconds
const int delayTime = 1200000;
 
// File variable
File tempsFile;
 
 
 
void setup() {
  // Serial output for when connected to computer
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
 
  Serial.print("Initializing SD card...");
  if(!SD.begin(sdPin)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("Initialization done.");
 
  tempsFile = SD.open("temps.txt", FILE_WRITE);
 
  if (tempsFile) {
    Serial.println("Printing temperatures");
    tempsFile.println("Printing temperatures:");
    tempsFile.close();
    Serial.println("Done.");
  } else {
    Serial.println("Error opening file in setup.");
  }
 
}
 /*
void loop() {
  /********************/
  // Open SD card for writing
  
  /*
  tempsFile = SD.open("rawEMG.txt", FILE_WRITE);
  
 
    // write temps to SD card
    tempsFile.print("Voltage: ");
    tempsFile.print(voltage);
 
 
    // close the file
    tempsFile.close();
  } else {
    Serial.println("Error opening file in loop.");
  }
 
 
  delay(delayTime);
 
}
 
float getVoltage(int pin)
{
  return (analogRead(pin) * 0.004882814);
} */
  



///////////////////////////////////////////////////////////////////////////////////////////////////////////
