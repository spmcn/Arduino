#ifndef _flash_funcs_
#define _flash_funcs_

#include <Adafruit_SPIFlash.h>
#include <Adafruit_SPIFlash_FatFs.h>
#include "utility/ff.h"
#include <Arduino.h>


int format(Adafruit_SPIFlash* spiFlash, spiflash_type_t flashType, Adafruit_W25Q16BV_FatFs* fatFs, int waitTime);


#endif