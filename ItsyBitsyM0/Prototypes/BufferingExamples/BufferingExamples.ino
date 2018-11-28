/******************************
 *  INCLUDE FILES
 ******************************/
#include <SPI.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_SPIFlash_FatFs.h>
/* 
 *  Must distinguish between Adafruit SPI
 *  and SD.h library classes
 */
typedef File FlashFile;
#undef FILE_READ
#undef FILE_WRITE
#define FLASH_READ FA_READ
#define FLASH_WRITE (FA_READ | FA_WRITE | FA_OPEN_APPEND)

/* Include the SD.h library */
#include <SD.h>

// Include the FatFs library header to use its low level functions
// directly.  Specifically the f_fdisk and f_mkfs functions are used
// to partition and create the filesystem.
#include "utility/ff.h"

// User created functions
// (Currently only has the format() function
#include "src/flash_funcs.h"

/******************************
 *  DEFINITIONS
 ******************************/
// Configuration of the flash chip pins and flash fatfs object.
// You don't normally need to change these if using a Feather/Metro
// M0 express board.
#define FLASH_TYPE     SPIFLASHTYPE_W25Q16BV  // Flash chip type.
#define FLASH_SS       SS1                    // Flash chip SS pin.
#define FLASH_SPI_PORT SPI1                   // What SPI port is Flash on?
#define NEOPIN         40
#define FILE_NAME      "data.csv"
#define SD_FILE_NAME   "data2.csv"
Adafruit_SPIFlash flash(FLASH_SS, &FLASH_SPI_PORT);     // Use hardware SPI
Adafruit_W25Q16BV_FatFs fatfs(flash);

/******************************
 *  INITIALIZE VARIABLES
 ******************************/
 
unsigned long startTime;    // time that the program has started
unsigned long dt;           // time it takes to write to flash file
FlashFile flashFile;        // file on internal Flash
SDFile sdFile;              // file on SD card
const int bufLen = 128;     // buffer length of 128B
uint8_t buf[bufLen] = {0};  // buffer to hold our data
int bufIndex = 0;                  // variable to hold index of buffer
void setup()
{
  Serial.begin(9600);
  while(!Serial);

  // format and mount flash file system
  if (format(&flash, FLASH_TYPE, &fatfs, 1)) Serial.println("\n\nFormatting has been stopped...");

  if (!SD.begin(10)) {
    Serial.println("SD Card initialization failed!");
    while (1);
  }
  
  startTime = millis();

  // open flashFile to begin storage of data
  flashFile = fatfs.open(FILE_NAME,FLASH_WRITE);
  
  Serial.println("Start: " + String(startTime));

}

void loop()
{

  // after 5 seconds, stop collecting
  // and store all data from Flash 
  // onto the SD card
  if(millis() - startTime > 5000)
  {

    // close the flashFile
    flashFile.close();

    // print file contents onto SD card
    printFileContents(FILE_NAME);

    // stop program
    while(1);
  }

  // if we have not filled the buffer yet,
  // fill the next space with collected data
  if(bufIndex < bufLen)
  {
    buf[bufIndex++] = (uint8_t)(random(65,123));
  }

  // if our buffer has been filled,
  // copy contents from our buffer to the
  // buffer of the internal flash
  else
  {
    // simply for easier readability when verifying correct operation
    buf[bufLen-1] = '\n';

    // start time to measure
    // how long it takes to write buffer into flash memory

    // when flash memory buffer has been filled (512B),
    // it will take significantly longer to actually write
    // the buffer
    dt = millis();
    flashFile.write(buf,bufLen);
    Serial.println("Filled buffer and stored it, taking " + String(millis()-dt) + " ms");

    /* 
     * it's important NOT to close the flashfile,
     * otherwise it will write the buffer into memory,
     * regardless of how much data was in it
     * 
     * if we keep flashfile open, then the data is still waiting in the buffer
     * Potential Issues: loss of power means loss of data in buffer
     * 
     */

  }
  
}

void printFileContents(String fileName) {

  // flashFile should have been closed prior to printing file contents
  FlashFile dataFile = fatfs.open(fileName, FLASH_READ);
  if (!dataFile) {
    Serial.println("Error, failed to open file for reading!");
    while (1);
  }

  // open the SD card file
  SDFile sdFile = SD.open(SD_FILE_NAME,FILE_WRITE);
  if(!sdFile)
  {
    Serial.println("Error, failed to open file on SD card for writing!");
    while(1);
  }
  Serial.println("\nPrint contents to SD Card");
  
  // track time spent writing data
  dt = millis();

  // read all the data and print it out a character (1B) at a time
  // (stopping when end of file is reached)
  while (dataFile.available()) {
    char c;
    dataFile.read(&c,1);
    sdFile.write(&c,1);
    
  }

  // depending on our data, would it be faster to read
  // data 1B at a time, or 4B at a time? Something to test.
  
  Serial.println("It took " + String(millis() - dt) + " ms to write to SD");

  
  // Print the total size of file on flash memory
  Serial.print("Total size of flash file (bytes): ");
  Serial.print(dataFile.size());
  Serial.print(" (");
  Serial.print((float)(dataFile.size()) / (1024 * 1024), DEC);
  Serial.println(" MB)");
  Serial.println();

  // the files can now be closed
  dataFile.close();
  sdFile.close();
}
