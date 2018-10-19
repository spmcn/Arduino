#include <SPI.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_SPIFlash_FatFs.h>
#include <SparkFunMPU9250-DMP.h>

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

#define WAIT 3 // seconds to wait until formatting

boolean r = false; // TRUE: READING, FALSE: WRITING
boolean imu_present = false;
unsigned int count = 0;
unsigned int count_max = 5;
unsigned int total = 0;
unsigned int total_max = 2;

MPU9250_DMP imu;

void setup() {
  // Initialize serial port and wait for it to open before continuing.
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

  // format and mount flash file system
  if(format()) Serial.println("\n\nFormatting has been stopped...");

  // Call imu.begin() to verify communication with and
  // initialize the MPU-9250 to it's default values.
  // Most functions return an error code - INV_SUCCESS (0)
  // indicates the IMU was present and successfully set up

  while(1)
  {
    Serial.println("Do you have an IMU present? (Y/N)");
    while(Serial.available() == 0) delay(100);
    if(Serial.available() > 0)
    {
      char a = Serial.read();
      if(a == 'Y') imu_present = true;
      else if (a == 'N') imu_present = false;
      else 
      {
        Serial.println("Input not accepted, try again ...");
        continue;
      }
      break;
    }
  }

  
  if(imu_present)
  {
    Serial.print("Turning on IMU...");
    if(imu.begin() != INV_SUCCESS)
    {
      Serial.println("There has been an error initializing the IMU");
      while(1) delay(100);
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
}

void loop() {

  if(total > total_max)
  {
    Serial.println("\nCompleted sensor storage/reading");
    while(1) delay(100);
  }
  
  else{    
    if(++count > 5)
    {
      total++;
      count = 0;
      File dataFile = fatfs.open(FILE_NAME, FILE_READ);
      if (!dataFile) {
        Serial.println("Error, failed to open file for reading!");
        while(1);
      }
  
      // And finally to read all the data and print it out a character at a time
      // (stopping when end of file is reached):
      Serial.println("\nEntire contents of file:");
      while (dataFile.available()) {
        char c = dataFile.read();
        Serial.print(c);
      }

      Serial.print("Total size of file (bytes): ");
      Serial.print(dataFile.size());
      Serial.print(" (");
      Serial.print((float)(dataFile.size())/(1024*1024),DEC);
      Serial.println(" MB)");
      Serial.println();

      dataFile.close();
    }
    // copied from fatfs_datalogging.ino example
  
    else{
          
      File dataFile = fatfs.open(FILE_NAME, FILE_WRITE);
      if (dataFile) {
        int16_t accelX, accelY, accelZ;
        if(imu_present && imu.dataReady())
        {
          imu.update(UPDATE_ACCEL | UPDATE_GYRO | UPDATE_COMPASS);
          accelX = imu.ax;
          accelY = imu.ay;
          accelZ = imu.az; 
        }
        else
        {
          accelX = random(-100,100);
          accelY = random(-100,100);
          accelZ = random(-100,100);           
        }

        // Write a line to the file. 
        // Write comma separated values
        dataFile.print("Accel Reading");
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

int format(){
  
  // Initialize flash library and check its chip ID.
  if (!flash.begin(FLASH_TYPE)) {
    Serial.println("Error, failed to initialize flash chip!");
    while(1);
  }
  Serial.print("Flash chip JEDEC ID: 0x"); Serial.println(flash.GetJEDECID(), HEX);

  int countdown = WAIT;
  Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  Serial.println("This sketch will ERASE ALL DATA on the flash chip and format it with a new filesystem!");
  Serial.print("Flash will be formated in ");
  Serial.print(WAIT);
  Serial.println(" seconds... Type 'STOP' to prevent formatting.");
  Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  Serial.setTimeout(1000);
  while(countdown){
    if(Serial.find("STOP")) return 1;
    Serial.print(countdown--);
    Serial.print("... ");
  }
  
  fatfs.activate();

  // Partition the flash with 1 partition that takes the entire space.
  Serial.println("\nPartitioning flash with 1 primary partition...");
  DWORD plist[] = {100, 0, 0, 0};  // 1 primary partition with 100% of space.
  uint8_t buf[512] = {0};          // Working buffer for f_fdisk function.
  FRESULT r = f_fdisk(0, plist, buf);
  if (r != FR_OK) {
    Serial.print("Error, f_fdisk failed with error code: "); Serial.println(r, DEC);
    while(1);
  }
  Serial.println("Partitioned flash!");

  // Make filesystem.
  Serial.println("Creating and formatting FAT filesystem (this takes ~60 seconds)...");
  r = f_mkfs("", FM_ANY, 0, buf, sizeof(buf));
  if (r != FR_OK) {
    Serial.print("Error, f_mkfs failed with error code: "); Serial.println(r, DEC);
    while(1);
  }
  Serial.println("Formatted flash!");

  // Test that the file system is mounted
  if (!fatfs.begin()) {
    Serial.println("Error, failed to mount newly formatted filesystem!");
    while(1);
  }
  Serial.println("Flash chip successfully formatted with new empty filesystem!");

  return 0;
}
