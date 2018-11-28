#include "flash_funcs.h"

int format(Adafruit_SPIFlash* spiFlash, spiflash_type_t flashType, Adafruit_W25Q16BV_FatFs* fatFs, int waitTime) {

  // Initialize flash library and check its chip ID.
  if (!(spiFlash->begin(flashType))) {
    Serial.println("Error, failed to initialize flash chip!");
    while (1);
  }
  Serial.print("Flash chip JEDEC ID: 0x"); Serial.println(spiFlash->GetJEDECID(), HEX);
  Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  Serial.println("This sketch will ERASE ALL DATA on the flash chip and format it with a new filesystem!");
  Serial.print("Flash will be formated in ");
  Serial.print(waitTime);
  Serial.println(" seconds... Type 'STOP' to prevent formatting.");
  Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  Serial.setTimeout(1000);

  int countdown = waitTime;
  while (countdown) {
    if (Serial.find("STOP")) return 1;
    Serial.print(countdown--);
    Serial.print("... ");
  }

  fatFs->activate();

  // Partition the flash with 1 partition that takes the entire space.
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
  if (!(fatFs->begin())) {
    Serial.println("Error, failed to mount newly formatted filesystem!");
    while (1);
  }
  Serial.println("Flash chip successfully formatted with new empty filesystem!");

  return 0;
}