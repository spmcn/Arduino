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
  exitTime = millis();
  Serial.print("Starting time: ");
  Serial.print(exitTime);
  Serial.println(" ms");
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
  
  Serial.print("--- Entering TC5_Handler at: ");
  Serial.print(millis());
  Serial.println(" ms ---");

  Serial.print("Count: ");
  Serial.println(count);

  // begin modifying data
  File dataFile = fatfs.open(FILE_NAME,FILE_WRITE);
  
  if(dataFile){
    Serial.println("Datafile exists.");
    if(count>9 && count<109) tc_setFreq(10);
    else if(count == 109)
    {
      tc_setFreq(1);
      count = 0;
      sample_cycles++;
    }

    Serial.print("Writing to file... ");
    // put the current time into the reading
    int reading = millis();
    dataFile.print("Sensor #1"); Serial.print("1... ");
    dataFile.print(","); Serial.print("2... ");
    dataFile.print(reading, DEC); Serial.print("3... ");
    dataFile.println();
    Serial.println("Done!");
    
    Serial.print("Closing file... ");
    dataFile.close();
    Serial.println("Done!");
  }
  else {
    Serial.println("Failed to open file for write.");
    Serial.println("Please fix . . .");
    while(1);
  }

  Serial.println("Toggling LED . . .");
  if(stateLED == true) digitalWrite(LED_PIN,HIGH);
  else digitalWrite(LED_PIN,LOW);

  
  stateLED = !stateLED;
 
  if(sample_cycles>5){
    tc_disable();
    Serial.println("Done sampling.");
  }
  
  count++;
  exitTime = millis();
  
  Serial.print("--- Exiting TC5_Handler at: ");
  Serial.print(exitTime);
  Serial.println(" ms ---");
  Serial.print("Total time spent in handler: ");
  Serial.print(exitTime - enterTime);
  Serial.println(" ms\n\n");
  exitTime = millis();
  
  tc_clearIntFlag(); // REQUIRED at end of TC5_Handler.
}
