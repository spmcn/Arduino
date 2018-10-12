/*
This sketch illustrates how to set a timer on an SAMD21 based board in Arduino
*/

#include <Sampling.h>
#define LED_PIN 13 //just for an example
bool stateLED = 0; //just for an example
int count = 0;
uint16_t sampleRate = 10; // sample rate (Hz)

void setup() {
  Serial.begin(9600);
  
  // configure LED pin
  pinMode(LED_PIN,OUTPUT);
  
  // configure timer to run at <sampleRate>Hertz
  // ignore the second argument (7) for now...
  tc_config(sampleRate,7); //configure the timer to run at <sampleRate>Hertz

  // start the timer
  tc_start();
}

void loop() {
  //tc_disable(); //This function can be used anywhere if you need to stop/pause the timer
  //tc_reset(); //This function should be called everytime you stop the timer
}

//this function gets called by the interrupt at <sampleRate>Hertz
void TC5_Handler (void) {

  /* begin code */
  // in Serial Monitor, check "Show timestamp" to verify
  String s = "\nInterrupt Request...";
  Serial.print("\nInterrupt Request...");
  if(stateLED == true) {
    digitalWrite(LED_PIN,HIGH);
  } else {
    digitalWrite(LED_PIN,LOW);
  }
  
  stateLED = !stateLED;

  // after 10 cycles, change frequency to 20 Hz
  if(count > 10) tc_setFreq(20);

  // after 30 cycles, change back to 10 Hz
  if(count > 30)
  {
    tc_setFreq(10);
    count = 0;
  }
  count++;
  /* end code */
  
  tc_clearIntFlag(); // REQUIRED at end of TC5_Handler.
}
