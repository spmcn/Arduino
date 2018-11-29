/******************************
    INCLUDE FILES
 ******************************/
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

/* Include the SD.h library */
#include <SD.h>

/* Include the MPU9250 library */
#include <SparkFunMPU9250-DMP.h>

// Include the FatFs library header to use its low level functions
// directly.  Specifically the f_fdisk and f_mkfs functions are used
// to partition and create the filesystem.
#include "utility/ff.h"

// User created functions
// (Currently only has the format() function
#include "src/flash_funcs.h"

/******************************
    DEFINITIONS
 ******************************/
// Configuration of the flash chip pins and flash fatfs object.
// You don't normally need to change these if using a Feather/Metro
// M0 express board.
#define FLASH_TYPE     SPIFLASHTYPE_W25Q16BV  // Flash chip type.
#define FLASH_SS       SS1                    // Flash chip SS pin.
#define FLASH_SPI_PORT SPI1                   // What SPI port is Flash on?
#define NEOPIN         40
#define FILE_NAME      "data.csv"
#define SD_FILE_BASE_NAME  "data"
Adafruit_SPIFlash flash(FLASH_SS, &FLASH_SPI_PORT);     // Use hardware SPI
Adafruit_W25Q16BV_FatFs fatfs(flash);

#define F_SAMPLE 20
#define M F_SAMPLE // downsample to 1Hz frequency
#define OH_ANGLE 10

#define CHIP_SELECT_PIN 10
#define INTERRUPT_PIN 9
/******************************
    INITIALIZE VARIABLES
 ******************************/

// for variables used by both ISR and main program
// declare them as 'volatile'

char SD_FILE_NAME[13];
unsigned long startTime;    // time that the program has started
unsigned long oldTime;      // storage variable for last time stored
FlashFile flashFile;        // file on internal Flash
const int bufLen = 504;     // buffer length of 504B (21 data reads x 24B)
uint8_t buf[bufLen] = {0};  // buffer to hold our data
int i = 0;                  // variable to hold index of buffer

const int analogOutPin = A1; // Analog input for sEMG

MPU9250_DMP imu;
bool inOverhead = false;
unsigned char mcount = 0;
int ohCount = 0;
void setup()
{
  Serial.begin(9600);
  //while (!Serial);

  //Set up LED
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
  // format and mount flash file system
  if (format(&flash, FLASH_TYPE, &fatfs, 1)) Serial.println("\n\nFormatting has been stopped...");

  if (!SD.begin(CHIP_SELECT_PIN)) {
    Serial.println("SD Card initialization failed!");
    while (1);
  }

  /* initialize SD card */

  // first, find an unused filename
  int BASE_NAME_SIZE = sizeof(SD_FILE_BASE_NAME) - 1;
  char sdFileName[13] = SD_FILE_BASE_NAME "00.csv";
  if (BASE_NAME_SIZE > 6) {
    Serial.println("FILE_BASE_NAME too long");
    while(1);
  } 
  
  while (SD.exists(sdFileName)) {
    if (sdFileName[BASE_NAME_SIZE + 1] != '9') {
      sdFileName[BASE_NAME_SIZE + 1]++;
    } else if (sdFileName[BASE_NAME_SIZE] != '9') {
      sdFileName[BASE_NAME_SIZE + 1] = '0';
      sdFileName[BASE_NAME_SIZE]++;
    } else {
      Serial.println("Can't create file name (max has been reached).");
    }
  }

  for(int j = 0; j < 13; j++) SD_FILE_NAME[j] = sdFileName[j];

  SDFile sdFile = SD.open(SD_FILE_NAME, FILE_WRITE);
  // print headers
  sdFile.println("Time,Pitch,GyroX,GyroY,GyroZ,sEMG");
  sdFile.close();
  /* imu setup */
  setupIMU();

  /* setup for imu interrupt service routine */
  /*
    imu.enableInterrupt();
    imu.setIntLevel(INT_ACTIVE_LOW);
    imu.setIntLatched(INT_LATCHED);

    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), imuISR, LOW);
  */
  startTime = millis();

  // open flashFile to begin storage of data
  flashFile = fatfs.open(FILE_NAME, FLASH_WRITE);

  Serial.println("Start: " + String(startTime));
  oldTime = startTime;
  digitalWrite(LED_BUILTIN,LOW);
}

void loop() {

  unsigned long newTime = millis();
  
  // after every 5 seconds,
  // store data onto the SD card
  if(newTime-oldTime > 5000)
  {
    flashFile.close();
    // print contents from flash to SD
    print2sd(FILE_NAME);
    
    // after 60 seconds, stop program altogether
    if(newTime-startTime > 30000){
      // print out all data from SD Card
      printSDContents(SD_FILE_NAME);
      digitalWrite(LED_BUILTIN,HIGH);
      while(1) delay(10);
    }

    // otherwise, keep storing data onto internal flash
    else
    {
      // overwrite old data on flash
      flashFile = fatfs.open(FILE_NAME,FLASH_WRITE);
      flashFile.seek(0);
      
      oldTime = millis();
    }
  }

  // check the IMU if data is available to read
  if (imu.fifoAvailable())
  {
    if (imu.dmpUpdateFifo() == INV_SUCCESS)
    {
      imu.computeEulerAngles();
      float p;
      unsigned long t;
      float g[3];
      calcIMUData(&p, g, &t);
      unsigned int semg = analogRead(analogOutPin);; // RAW sEMG data
      {
        if (inOverhead || mcount == M)
        {
          mcount = 0;
          store2buf(semg,p,g,t);
          //printIMUData(p, g, t);
        }
      }

      if (!inOverhead) mcount++;
    }
  }
}

void store2buf(int semg, float p, float* g, unsigned long t)
{
  unsigned long dt = millis();
  flashFile.println(
    String(t) + ',' +
    String(p) + ',' +
    String(g[0]) + ',' +
    String(g[1]) + ',' +
    String(g[2]) + ',' +
    String(semg));
  Serial.println("Storing set of data, taking " + String(millis()-dt) + " ms");
  

  /*
  // store sEMG data byte-by-byte
  buf[i++] = (uint8_t)(0xFF & (semg >> 24));
  buf[i++] = (uint8_t)(0xFF & (semg >> 16));
  buf[i++] = (uint8_t)(0xFF & (semg >> 8));
  buf[i++] = (uint8_t)(0xFF & (semg));
    
  // store pitch byte-by-byte
  buf[i++] = (uint8_t)(0xFF & (p >> 24));
  buf[i++] = (uint8_t)(0xFF & (p >> 16));
  buf[i++] = (uint8_t)(0xFF & (p >> 8));
  buf[i++] = (uint8_t)(0xFF & (p));

  // store gyro x-axis data byte-by-byte
  buf[i++] = (uint8_t)(0xFF & (g[0] >> 24));
  buf[i++] = (uint8_t)(0xFF & (g[0] >> 16));
  buf[i++] = (uint8_t)(0xFF & (g[0] >> 8));
  buf[i++] = (uint8_t)(0xFF & (g[0]));

  // store gyro y-axis data byte-by-byte
  buf[i++] = (uint8_t)(0xFF & (g[1] >> 24));
  buf[i++] = (uint8_t)(0xFF & (g[1] >> 16));
  buf[i++] = (uint8_t)(0xFF & (g[1] >> 8));
  buf[i++] = (uint8_t)(0xFF & (g[1]));

  // store gyro z-axis data byte-by-byte
  buf[i++] = (uint8_t)(0xFF & (g[2] >> 24));
  buf[i++] = (uint8_t)(0xFF & (g[2] >> 16));
  buf[i++] = (uint8_t)(0xFF & (g[2] >> 8));
  buf[i++] = (uint8_t)(0xFF & (g[2]));

  // store time
  buf[i++] = (uint8_t)(0xFF & (*t >> 24));
  buf[i++] = (uint8_t)(0xFF & (*t >> 16));
  buf[i++] = (uint8_t)(0xFF & (*t >> 8));
  buf[i++] = (uint8_t)(0xFF & (*t));

  if(i >= bufLen)
  {
    dt = millis();
    flashFile.write(buf,bufLen);
    i=0;
    Serial.println("Filled buffer and stored it, taking " + String(millis()-dt) + " ms");
  }

  */
  
}

void print2sd(String fileName) {
  FlashFile dataFile = fatfs.open(fileName, FLASH_READ);
  if (!dataFile) {
    Serial.println("Error, failed to open file for reading!");
    while (1);
  }

  SDFile sdFile = SD.open(SD_FILE_NAME,FILE_WRITE);
  if(!sdFile)
  {
    Serial.println("Error, failed to open file on SD card for writing!");
    while(1);
  }
  // read all the data and print it out a character at a time
  // (stopping when end of file is reached):
  Serial.println("\nPrint contents to SD Card");
  unsigned long dt = millis();
  int dataCount = 0;
  while (dataFile.available()) {
    char c;
    dataCount++;
    dataFile.read(&c,1);
    sdFile.write(&c,1);
  }

  Serial.println("It took " + String(millis() - dt) + " ms to write to SD");

  
  // Print the total size of file
  Serial.print("Total size of flash file (bytes): ");
  Serial.print(dataFile.size());
  Serial.print(" (");
  Serial.print((float)(dataFile.size()) / (1024 * 1024), DEC);
  Serial.println(" MB)");
  Serial.println();

  dataFile.close();
  sdFile.close();
}
/*
  void imuISR()
  {
  }
*/
void setupIMU()
{

  // Call imu.begin() to verify communication and initialize
  if (imu.begin() != INV_SUCCESS)
  {
    while (1)
    {
      Serial.println("Unable to communicate with MPU-9250");
      Serial.println("Check connections, and try again.");
      Serial.println();
      delay(5000);
    }
  }

  imu.setSensors(INV_XYZ_ACCEL | INV_XYZ_GYRO);
  imu.setGyroFSR(2000); // Set gyro to 2000 dps
  imu.setAccelFSR(2);
  //imu.setLPF(5);
  imu.dmpBegin(DMP_FEATURE_GYRO_CAL | DMP_FEATURE_SEND_CAL_GYRO | DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP, F_SAMPLE);
  // DMP_FEATURE_LP_QUAT can also be used. It uses the
  // accelerometer in low-power mode to estimate quat's.
  // DMP_FEATURE_LP_QUAT and 6X_LP_QUAT are mutually exclusive
}

void calcIMUData(float* p, float* g, unsigned long* t)
{
  // After calling dmpUpdateFifo() the ax, gx, mx, etc. values
  // are all updated.
  // Quaternion values are, by default, stored in Q30 long
  // format. calcQuat turns them into a float between -1 and 1

  g[0] = imu.calcGyro(imu.gx);
  g[1] = imu.calcGyro(imu.gy);
  g[2] = imu.calcGyro(imu.gz);

  *p = imu.pitch;
  bool currentCheck = checkOverhead(*p);
  if (currentCheck && !inOverhead) ohCount++; // rising edge
  inOverhead = currentCheck;

  *t = imu.time;
}

// determine if in overhead work
bool checkOverhead(float p)
{
  if (p > 270 && p < 360 - OH_ANGLE) return true;
  else return false;
}

void printIMUData(float p, float* g, unsigned long t)
{
  unsigned long t1 = t/1000;
  unsigned long t2 = t%1000;
  
  if(inOverhead)
  {
    Serial.println("------ OVERHEAD WORK (" + String(ohCount) + ") ------");
  }
  Serial.println("Time: " + String(t1) + "." + ( t2 < 100 ? "0" : "") + String(t2) + " s");
  Serial.println("Pitch: " + String(p) + " degrees");
  Serial.println("Gyro: (" + String(g[0]) + ", " + String(g[1]) + ", " + String(g[2]) + ") dps");
  Serial.println();
}

void printSDContents(String fileName)
{
  SDFile sdFile = SD.open(SD_FILE_NAME,FILE_READ);
  if(!sdFile)
  {
    Serial.println("Error, failed to open file on SD card for reading!");
    while(1);
  }
  
  while (sdFile.available()){
    char c;
    sdFile.read(&c,1);
    Serial.print(c);
  }
  
}
