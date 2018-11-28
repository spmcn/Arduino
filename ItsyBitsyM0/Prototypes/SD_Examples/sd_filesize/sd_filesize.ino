/*
 * Currently will not compile. In the process of figuring out how to make
 * this work by looking at the included libraries. Line 38 is the one that
 * has the error, but there seems to be an init function that matches what
 * I currently have, so I'm not sure why it won't compile.
 */

#include <SPI.h>
#include <SD.h>

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
File myFile;
char toWrite;

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// MKRZero SD: 
const int chipSelect = 10;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(chipSelect,OUTPUT);

  Serial.print("\nInitializing SD card...");
  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  //if (!card.init(sckSpeed, chipSelectPin, mosiPin, misoPin, sckPin)) {
  if (!card.init(SPI_HALF_SPEED,chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    while (1);
  } else {
    Serial.println("Card initialized.");
  }
  
  if (!SD.begin(chipSelect)){
    Serial.println("could not write to file");
  } else {
    if(SD.exists("test.txt")){
      SD.remove("test.txt");
    }
    myFile = SD.open("test.txt", FILE_WRITE);
    if(myFile){
      Serial.print("Writing to test.txt...");
      for(int i=0; i<200000; i++){
        if((i%26)==0){
          myFile.print("\n");
        }
        toWrite = (char)((i%26)+65);
        myFile.print(toWrite);
      }
      myFile.close();
      Serial.println("done.");
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
    
    if (!volume.init(card)) {
      Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
      while (1);
    }
  }
  
  Serial.println();
  Serial.print("Clusters:          ");
  Serial.println(volume.clusterCount());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(volume.blocksPerCluster());

  Serial.print("Total Blocks:      ");
  Serial.println(volume.blocksPerCluster() * volume.clusterCount());
  Serial.println();

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("Volume type is:    FAT");
  Serial.println(volume.fatType(), DEC);

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
}

void loop(void) {
}
