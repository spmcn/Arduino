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

#include <SD.h>
#include <SparkFunMPU9250-DMP.h>
#include <RTCZero.h>

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


#include "src/flash_funcs.h"

#define WAIT 1 // seconds to wait until formatting

boolean r = false; // TRUE: READING, FALSE: WRITING
boolean imu_present = false;
unsigned int count = 0;
unsigned int count_max = 5;
unsigned int total = 0;
unsigned int total_max = 2;
unsigned long startTime = 0;
unsigned long endTime = 0;


MPU9250_DMP imu;
RTCZero rtc;

void setup() {
  // Initialize serial port and wait for it to open before continuing.
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

  // Set up the real-time counter
  rtc.begin(); // initialize rtc
  rtc.setTime(0, 0, 0);
  rtc.setDate(0, 0, 0);
  printTime();
  startTime = millis();

  // format and mount flash file system
  if (format(&flash, FLASH_TYPE, &fatfs, WAIT)) Serial.println("\n\nFormatting has been stopped...");

  // ask if IMU is present
  while (1) 
  {
    Serial.println("Do you have an IMU present? (Y/N)");
    while (Serial.available() == 0) delay(100);
    if (Serial.available() > 0)
    {
      char a = Serial.read();
      if (a == 'Y') imu_present = true;
      else if (a == 'N') imu_present = false;
      else
      {
        Serial.println("Input not accepted, try again ...");
        continue;
      }
      break;
    }
  }


  if (imu_present)
  {
    // Call imu.begin() to verify communication with and
    // initialize the MPU-9250 to it's default values.
    // Most functions return an error code - INV_SUCCESS (0)
    // indicates the IMU was present and successfully set up

    Serial.print("Turning on IMU...");
    if (imu.begin() != INV_SUCCESS)
    {
      Serial.println("There has been an error initializing the IMU");
      while (1) delay(100);
    }
    else Serial.println(" Done!");

    Serial.print("Enabling sensors ");
    // Use setSensors to turn on or off MPU-9250 sensors.
    // Any of the following defines can be combined:
    // INV_XYZ_GYRO, INV_XYZ_ACCEL, INV_XYZ_COMPASS,
    // INV_X_GYRO, INV_Y_GYRO, or INV_Z_GYRO
    // Enable all sensors:
    imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
    Serial.println(" Done!");

    Serial.print("Setting sensor options...");
    // Gyro options are +/- 250, 500, 1000, or 2000 dps
    imu.setGyroFSR(250); // Set gyro to 2000 dps
    Serial.print("... Gyro Done! ");
    // Accel options are +/- 2, 4, 8, or 16 g
    imu.setAccelFSR(2); // Set accel to +/-2g
    Serial.println("... Accel Done! ");
    // Note: the MPU-9250's magnetometer FSR is set at
    // +/- 4912 uT (micro-tesla's)

    // setLPF() can be used to set the digital low-pass filter
    // of the accelerometer and gyroscope.
    // Can be any of the following: 188, 98, 42, 20, 10, 5
    // (values are in Hz).
    imu.setLPF(5); // Set LPF corner frequency to 5Hz

    // The sample rate of the accel/gyro can be set using
    // setSampleRate. Acceptable values range from 4Hz to 1kHz
    imu.setSampleRate(10); // Set sample rate to 10Hz

    // Likewise, the compass (magnetometer) sample rate can be
    // set using the setCompassSampleRate() function.
    // This value can range between: 1-100Hz
    imu.setCompassSampleRate(10); // Set mag rate to 10Hz
    Serial.println("IMU is ready to make measurements!\n");
  }

  else Serial.println("Sensor readings will be assigned random numbers.");

  // Write headers to file
  FlashFile dataFile = fatfs.open(FILE_NAME, FLASH_WRITE);
  if (dataFile)
  {
    dataFile.println("Time,Ax,Ay,Az");
    dataFile.close();
  }
}

void loop() {

  // Stop storing/reading data after a certain number of reads

  if (total > total_max)
  {
    endTime = millis();
    Serial.println("\nCompleted sensor storage/reading");
    printTime();
    Serial.println(endTime - startTime);
    while (1) delay(100);
  }


  else {

    // After storing 5 sensor readings,
    // read the file to verify correct operation
    if (++count > 5)
    {
      total++;
      count = 0;
      printFileContents(FILE_NAME);
    }

    else {

      // Read sensor and write sensor data to file
      FlashFile dataFile = fatfs.open(FILE_NAME, FLASH_WRITE);
      if (dataFile) {
        int16_t accelX, accelY, accelZ;
        if (imu_present && imu.dataReady())
        {
          imu.update(UPDATE_ACCEL | UPDATE_GYRO | UPDATE_COMPASS);
          accelX = imu.ax;
          accelY = imu.ay;
          accelZ = imu.az;
        }
        else
        {
          accelX = random(-100, 100);
          accelY = random(-100, 100);
          accelZ = random(-100, 100);
        }

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
