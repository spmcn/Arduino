#ifndef Sampling_h
#define Sampling_h

#include "Arduino.h"

float tc_config(uint32_t sampleRate, uint32_t prescale);
void tc_start();
void tc_reset();
void tc_disable();
float tc_setFreq(int freq);
void tc_clearIntFlag();
#endif
