/* 
 *  Currently does not run. Uploading it to the ItysBitsy results in
 *  not being able to connect to it via serial com port until it's restarted
 *  in boot mode. Will run if lines 46 and 47 are commented out, so I'm
 *  thinking it could be something with setting the baud rate to 115200
 *  that conflicts with Sean's library.
 */

#include <Sampling.h>
#include <SPI.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_SPIFlash_FatFs.h>

#define FLASH_TYPE     SPIFLASHTYPE_W25Q16BV
#define FLASH_SS       SS1
#define FLASH_SPI_PORT SPI1
#define NEOPIN         40
#define LED_PIN 13

Adafruit_SPIFlash flash(FLASH_SS, &FLASH_SPI_PORT);
Adafruit_W25Q16BV_FatFs fatfs(flash);

#define FILE_NAME      "data.csv"
uint16_t sampleRate = 10;
bool stateLED = 0;
int count = 0;
int sample_cycles = 0;

unsigned long enterTime = 0;
unsigned long exitTime = 0;
void setup() {
  Serial.begin(115200);
  while(!Serial); // wait until Serial is ready
  pinMode(LED_PIN,OUTPUT);

  //initialize flash and mount fs
  if (!flash.begin(FLASH_TYPE)) {
    Serial.println("Error, failed to initialize flash");
    while(1);
  }
  Serial.println("Flash chip initialized");
  Serial.print("Flash chip JEDEC ID: 0x"); Serial.println(flash.GetJEDECID(), HEX);
  if (!fatfs.begin()) {
    Serial.println("Error, failed to mount filesystem");
    while(1);
  }
  Serial.println("Mounted filesystem");
  
  tc_config(sampleRate,7);    //Sean: changed from 0 to 7
  tc_start();                 //Sean: timer should be working now

  // Serial outputs to test TC5_Handler functions
  Serial.print("Starting time: ");
  Serial.print(millis());
  Serial.print(" ms");
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("Test");
  //delay(10000L);
}

void TC5_Handler (void) {
  enterTime = millis();
  Serial.print("It has been ");
  Serial.print(enterTime - exitTime);
  Serial.println(" ms");
  
  Serial.print("Entering TC5_Handler at: ");
  Serial.print(millis());
  Serial.println(" ms");
  /*
  File dataFile = fatfs.open(FILE_NAME,FILE_WRITE);
  if(dataFile){
    if(count>9 && count<109) tc_setFreq(10);
    else if(count>108){
      tc_setFreq(1);
      count = 0;
      sample_cycles++;
    }
    int reading = random(0,100);
    dataFile.print("Sensor #1");
    dataFile.print(",");
    dataFile.print(reading, DEC);
    dataFile.println();
    dataFile.close();
  }
  else {
    Serial.println("Failed to open file for write.");
  }
  if(stateLED == true) {
    digitalWrite(LED_PIN,HIGH);
  } else {
    digitalWrite(LED_PIN,LOW);
  }
  stateLED = !stateLED;
  count++;
  if(sample_cycles>5){
    tc_disable();
    Serial.println("Done sampling.");
  }
  */


  Serial.print("Exiting TC5_Handler at: ");
  Serial.print(millis());
  Serial.println(" ms");
  Serial.println();

  exitTime = millis();
  tc_clearIntFlag(); // REQUIRED at end of TC5_Handler.
}
