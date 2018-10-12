#include "Arduino.h"
#include <Sampling.h>

// private function
bool isSyncing();
int div_value;

float tc_config(uint32_t sampleRate, uint32_t prescale)
{
	float realFreq = 0;
	// can only have 8 options for prescaling
	switch(prescale)
	{
		case 0:
			div_value = 1;
			break;
		case 1:
			div_value = 2;
			break;
		case 2:
			div_value = 4;
			break;
		case 3:
			div_value = 8;
			break;
		case 4:
			div_value = 16;
			break;			
		case 5:
			div_value = 64;
			break;
		case 6:
			div_value = 256;
			break;
		case 7:
			div_value = 1024;
			break;
	}

	// Enable GCLK for TCC2 and TC5 (timer counter input clock)
	GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5)) ;
	while (GCLK->STATUS.bit.SYNCBUSY);

	tc_reset(); //reset TC5

	// Set Timer counter Mode to 16 bits
	TC5->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
	// Set TC5 mode as match frequency
	TC5->COUNT16.CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
	//set prescaler
	TC5->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1 + (prescale << TC_CTRLA_PRESCALER_Pos);
	//set TC5 timer counter based off of the system clock and the user defined sample rate or waveform
	realFreq = tc_setFreq(sampleRate);
	while (isSyncing());

	// Configure interrupt request
	NVIC_DisableIRQ(TC5_IRQn);
	NVIC_ClearPendingIRQ(TC5_IRQn);
	NVIC_SetPriority(TC5_IRQn, 0);
	NVIC_EnableIRQ(TC5_IRQn);

	// Enable the TC5 interrupt request
	TC5->COUNT16.INTENSET.bit.MC0 = 1;
	while (isSyncing()); //wait until TC5 is done syncing 
	return realFreq;
} 


//Function that is used to check if TC5 is done syncing
//returns true when it is done syncing
bool isSyncing()
{
	return TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY;
}

//This function enables TC5 and waits for it to be ready
void tc_start()
{
	TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE; //set the CTRLA register
	while (isSyncing()); //wait until snyc'd
}

	//Reset TC5 
void tc_reset()
{
	TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
	while (isSyncing());
	while (TC5->COUNT16.CTRLA.bit.SWRST);
}

	//disable TC5
void tc_disable()
{
	TC5->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
	while (isSyncing());
}
float tc_setFreq(int freq)
{
	uint16_t cc = SystemCoreClock/div_value/freq - 1;
  TC5->COUNT16.CC[0].reg = cc;
  float realFreq = SystemCoreClock/div_value/(cc+1);
	while(isSyncing());

	return realFreq;
}

// Clear Interrupt Flag for Match or Capture Channel 0
void tc_clearIntFlag()
{
  TC5->COUNT16.INTFLAG.bit.MC0 = 1;
}
