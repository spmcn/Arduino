#include <SPI.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_SPIFlash_FatFs.h>
/*
    Must distinguish between Adafruit SPI
    and SD.h library classes
*/
typedef File FlashFile;
#undef FILE_READ
#undef FILE_WRITE
#define FLASH_READ FA_READ
#define FLASH_WRITE (FA_READ | FA_WRITE | FA_OPEN_APPEND)

#include <SD.h>

// Include the FatFs library header to use its low level functions
// directly.  Specifically the f_fdisk and f_mkfs functions are used
// to partition and create the filesystem.
#include "utility/ff.h"

// Configuration of the flash chip pins and flash fatfs object.
// You don't normally need to change these if using a Feather/Metro
// M0 express board.
#define FLASH_TYPE     SPIFLASHTYPE_W25Q16BV  // Flash chip type.
#define FLASH_SS       SS1                    // Flash chip SS pin.
#define FLASH_SPI_PORT SPI1                   // What SPI port is Flash on?
#define NEOPIN         40
#define FILE_NAME      "data.csv"
Adafruit_SPIFlash flash(FLASH_SS, &FLASH_SPI_PORT);     // Use hardware SPI
Adafruit_W25Q16BV_FatFs fatfs(flash);

#define WAIT 1 // seconds to wait until formatting


struct // 16B each
{
  short aX, aY, aZ; // accelerometer
  short gX, gY, gZ; // gyroscope
  uint32_t ms;      // time (in ms)
} data;

/*
   Use a 1KB buffer
   Can store 64 data reads
*/

uint8_t buf[1024] = {0};
uint8_t* bufPtr = buf;
uint8_t* bufTail = buf+1024;
uint16_t total = 0; // number of buffer writes to flash
uint16_t total_max = 3;


void setup() {
  // Initialize serial port and wait for it to open before continuing.
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

  Serial.println("Size of data: " + String(sizeof(data)));
  Serial.println("Buffer Addr: " + String((int)(bufPtr), HEX) + " (" + String((unsigned long)(&buf[0]), HEX) + ")");
  
  // generate random number to simulate sensor reading

  data.aX = random(-100,100);
  data.aY = random(-100,100);
  data.aZ = random(-100,100);

  data.gX = random(-100,100);
  data.gY = random(-100,100);
  data.gZ = random(-100,100);

  data.ms = millis();

  // store into buffer
  *bufPtr++ = data.aX >> 8;
  *bufPtr++ = 0xFF & data.aX;
  *bufPtr++ = data.aY >> 8;
  *bufPtr++ = 0xFF & data.aY;
  *bufPtr++ = data.aZ >> 8;
  *bufPtr++ = 0xFF & data.aZ;
  *bufPtr++ = data.gX >> 8;
  *bufPtr++ = 0xFF & data.gX;
  *bufPtr++ = data.gY >> 8;
  *bufPtr++ = 0xFF & data.gY;
  *bufPtr++ = data.gZ >> 8;
  *bufPtr++ = 0xFF & data.gZ;
  *bufPtr++ = (0xFF000000 & data.ms) >> 24;
  *bufPtr++ = (0x00FF0000 & data.ms) >> 16;
  *bufPtr++ = (0x0000FF00 & data.ms) >> 8;
  *bufPtr++ = (0x000000FF & data.ms);


  // read buffer to be sure
  uint8_t *p = buf;
  while(p < bufPtr)
  {
    Serial.println("0x" + String((int)(p),HEX) + ":\t" + String(*p));
    p++;
  }

  while(1);


  // 
  
  // format and mount flash file system
  //if (format()) Serial.println("\n\nFormatting has been stopped...");


}

void loop() {
/*
  // generate random number to simulate sensor reading
  data.aX = random(-100,100);
  data.aY = random(-100,100);
  data.aZ = random(-100,100);

  data.gX = random(-100,100);
  data.gY = random(-100,100);
  data.gZ = random(-100,100);

  data.ms = millis();

  // store into buffer
  *bufPtr++ = data;

  // check it is in the buffer
  Serial.println(

  while(1);
    for (int n = 0; n < 511; n++)
  {
    // fill with random numbers
    short rnd = random(-100, 100);
    *bufPtr++ = rnd;
  }

  bufPtr = buf; // reset bufPtr

  for (int n = 0; n < 511; n++)
  {
    Serial.println("0x" + String((int)bufPtr, HEX) + ":\t" + String(*bufPtr));
    bufPtr++;
  }

  bufPtr = buf;
*/
}
/*
  // Stop storing/reading data after a certain number of reads
  if (total > total_max)
  {
    endTime = millis();
    Serial.println("\nCompleted sensor storage/reading");
    printTime();
    Serial.println(endTime - startTime);
    while (1) delay(100);
  }

  // Read sensor and write sensor data to buffer
  short accelX, accelY, accelZ;
  accelX = random(-100, 100);
  accelY = random(-100, 100);
  accelZ = random(-100, 100);

   bufPtr = accelX; bufPtr++;
   bufPtr = accelY; bufPtr++;
   bufPtr = accelZ; bufPtr++;

  // check if the end has been reached
  if (bufPtr > endPtr)
  {
    // store buffer into SPI Flash
    FlashFile dataFile = fatfs.open(FILE_NAME, FLASH_WRITE);
    dataFile.write(buf, bufsize);
    dataFile.close;

    // reset bufPtr
    bufPtr = buf;
  }
  }

  }
  FlashFile dataFile = fatfs.open(FILE_NAME, FLASH_WRITE);
  if (dataFile) {
  // stores 3 shorts per sensor reading
  short accelX, accelY, accelZ;
  accelX = random(-100, 100);
  accelY = random(-100, 100);
  accelZ = random(-100, 100);


  // Write a line to the file.
  // Write comma separated values
  if (rtc.getHours() < 10) dataFile.print("0");
  dataFile.print(rtc.getHours());
  dataFile.print(":");
  if (rtc.getMinutes() < 10) dataFile.print("0");
  dataFile.print(rtc.getMinutes());
  dataFile.print(":");
  if (rtc.getSeconds() < 10) dataFile.print("0");
  dataFile.print(rtc.getSeconds());
  dataFile.print(",");
  dataFile.print(accelX, DEC);
  dataFile.print(",");
  dataFile.print(accelY, DEC);
  dataFile.print(",");
  dataFile.print(accelZ, DEC);
  dataFile.println();

  // Finally close the file when done writing.  This is smart to do to make
  // sure all the data is written to the file.
  dataFile.close();
  Serial.print("Wrote new measurement to data file: ");
  Serial.println("(ax, ay, az): (" + String(accelX) + ", " + String(accelY) + ", " + String(accelZ) + ")");
  }
  else {
  Serial.println("Failed to open data file for writing!");
  }
  }

  // Wait 1 seconds.
  delay(1000L);
  }
  }

  int format() {

  // Initialize flash library and check its chip ID.
  if (!flash.begin(FLASH_TYPE)) {
    Serial.println("Error, failed to initialize flash chip!");
    while (1);
  }
  Serial.print("Flash chip JEDEC ID: 0x"); Serial.println(flash.GetJEDECID(), HEX);
  Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  Serial.println("This sketch will ERASE ALL DATA on the flash chip and format it with a new filesystem!");
  Serial.print("Flash will be formated in ");
  Serial.print(WAIT);
  Serial.println(" seconds... Type 'STOP' to prevent formatting.");
  Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  Serial.setTimeout(1000);

  int countdown = WAIT;
  while (countdown) {
    if (Serial.find("STOP")) return 1;
    Serial.print(countdown--);
    Serial.print("... ");
  }

  fatfs.activate();

  // Partition the flash with 1 partitio\n that takes the entire space.
  Serial.println("\nPartitioning flash with 1 primary partition...");
  DWORD plist[] = {100, 0, 0, 0};  // 1 primary partition with 100% of space.
  uint8_t buf[512] = {0};          // Working buffer for f_fdisk function.
  FRESULT r = f_fdisk(0, plist, buf);
  if (r != FR_OK) {
    Serial.print("Error, f_fdisk failed with error code: "); Serial.println(r, DEC);
    while (1);
  }
  Serial.println("Partitioned flash!");

  // Make filesystem.
  Serial.println("Creating and formatting FAT filesystem (this takes ~60 seconds)...");
  r = f_mkfs("", FM_ANY, 0, buf, sizeof(buf));
  if (r != FR_OK) {
    Serial.print("Error, f_mkfs failed with error code: "); Serial.println(r, DEC);
    while (1);
  }
  Serial.println("Formatted flash!");

  // Test that the file system is mounted
  if (!fatfs.begin()) {
    Serial.println("Error, failed to mount newly formatted filesystem!");
    while (1);
  }
  Serial.println("Flash chip successfully formatted with new empty filesystem!");

  return 0;
  }

  void printTime() {
  // Prints time in HH:MM:SS format
  Serial.print("Time: ");
  if (rtc.getHours() < 10) Serial.print("0");
  Serial.print(rtc.getHours());
  Serial.print(":");
  if (rtc.getMinutes() < 10) Serial.print("0");
  Serial.print(rtc.getMinutes());
  Serial.print(":");
  if (rtc.getSeconds() < 10) Serial.print("0");
  Serial.println(rtc.getSeconds());
  }

  void printFileContents(String fileName) {
  FlashFile dataFile = fatfs.open(fileName, FLASH_READ);
  if (!dataFile) {
    Serial.println("Error, failed to open file for reading!");
    while (1);
  }

  // read all the data and print it out a character at a time
  // (stopping when end of file is reached):
  Serial.println("\nEntire contents of file:");
  while (dataFile.available()) {
    char c = dataFile.read();
    Serial.print(c);
  }

  // Print the total size of file
  Serial.print("Total size of file (bytes): ");
  Serial.print(dataFile.size());
  Serial.print(" (");
  Serial.print((float)(dataFile.size()) / (1024 * 1024), DEC);
  Serial.println(" MB)");
  Serial.println();

  dataFile.close();
  }

*/
