#include <SD.h>
#include <SPI.h>

File myFile;
uint8_t sckSpeed = SPI_HALF_SPEED;
uint8_t chipSelectPin = SS;   //pin 16
int8_t mosiPin = MOSI;        //pin 29
int8_t misoPin = MISO;        //pin 28
int8_t sckPin = 30;           //pin 30

int count = 0;
 
void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    delay(100);
  }
  Serial.print("Initializing SD card...");
  
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  //pinMode(SS, OUTPUT);
  pinMode(10, OUTPUT);

  if (!SD.begin(10)) {
  //if (!SD.begin(chipSelectPin,mosiPin,misoPin,sckPin)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
 
  // open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");
 
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
 
void loop()
{
  // nothing happens after setup
  count++;
  //Serial.println(count);
  if((count%2)==0) digitalWrite(13,HIGH);
  else digitalWrite(13,LOW);
  delay(1000);
}
