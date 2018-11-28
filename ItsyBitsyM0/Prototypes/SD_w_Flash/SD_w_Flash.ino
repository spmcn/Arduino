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

// Configuration of the flash chip pins and flash fatfs object.
// You don't normally need to change these if using a Feather/Metro
// M0 express board.
#define FLASH_TYPE     SPIFLASHTYPE_W25Q16BV
#define FLASH_SS       SS1                    // Flash chip SS pin.
#define FLASH_SPI_PORT SPI1                   // What SPI port is Flash on?
#define NEOPIN         40

Adafruit_SPIFlash flash(FLASH_SS, &FLASH_SPI_PORT);     // Use hardware SPI
Adafruit_W25Q16BV_FatFs fatfs(flash);

#define FLASH_FILE_NAME      "data.csv"
#define SD_FILE_NAME      "test.txt"

void setup() {
  // Initialize serial port and wait for it to open before continuing.
  Serial.begin(115200);
  Serial.println("Adafruit SPI Flash FatFs Simple Datalogging Example");

  // Initialize flash library and check its chip ID.
  if (!flash.begin(FLASH_TYPE)) {
    Serial.println("Error, failed to initialize flash chip!");
    while(1);
  }
  Serial.print("Flash chip JEDEC ID: 0x"); Serial.println(flash.GetJEDECID(), HEX);
  if (!fatfs.begin()) {
    Serial.println("Error, failed to mount newly formatted filesystem!");
    Serial.println("Was the flash chip formatted with the fatfs_format example?");
    while(1);
  }
  Serial.println("Mounted filesystem!");

  FlashFile dataFile1 = fatfs.open(FLASH_FILE_NAME);
  SDFile dataFile2 = SD.open(SD_FILE_NAME, FILE_WRITE);
  // Check that the file opened successfully and write a line to it.
  if (dataFile1 && dataFile2) {
    //write data to file
    dataFile1.close();
    dataFile2.close();
  }
  else {
    Serial.println("Failed to open data file for writing!");
  }
}

void loop() {
}
