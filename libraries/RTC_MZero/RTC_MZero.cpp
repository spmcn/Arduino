/*
  RTC library for Arduino Zero.
  Copyright (c) 2015 Arduino LLC. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <time.h>

#include "RTC_MZero.h"

voidFuncPtr RTC_callBack = NULL;

RTC_MZero::RTC_MZero()
{
  _configured = false;
  _millis = 0;
  _second = 0;
  _minute = 0;
  _hour = 0;
}

void RTC_MZero::begin(bool resetTime)
{
  //SYSCTRL->OSC8M.bit.PRESC = 3;

  uint16_t tmp_reg = 0;
  
  config32kOSC();

  // Setup clock GCLK2 with OSC8M
  configureClock();

  RTCdisable();

  RTCreset();

  RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_MODE_COUNT32; // set clock operating mode
  RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_PRESCALER_DIV32;

  // RTC->MODE0.COUNT increments on every 0-1 transition, so count frequency is actually 16kHz
  // MAX COUNT is (2^32) x (1us) is ~ 4,295 seconds (or 72 minutes)
  // overflow will happen too soon, so use the CMP0 interrupt to match and clear
  // to a nice and even number, instead of at 2^32 microseconds.


  //RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_MATCHCLR; // enable clear on match
  RTC->MODE0.READREQ.reg |= RTC_READREQ_RCONT; // enable continuously read mode
  //RTC->MODE0.COMP[0].reg = 16; // 1000us = 1ms
  while (RTCisSyncing())
    ;

  // enable the interrupt for compare and match/clear
  //RTC->MODE0.INTENSET.reg = RTC_MODE0_INTENSET_CMP0;
  
  NVIC_EnableIRQ(RTC_IRQn); // enable RTC interrupt 
  NVIC_SetPriority(RTC_IRQn, 0x00);


  while (RTCisSyncing())
    ;

  RTCenable();
  RTCresetRemove();

 
  _configured = true;
}

void RTC_Handler(void)
{
  
  if(RTC_callBack != NULL)
  {
    RTC_callBack();
  }
  
  RTC->MODE0.INTFLAG.reg = 0xFF; // must clear flag at end
}

void RTC_MZero::attachInterrupt(voidFuncPtr callback)
{
  RTC_callBack = callback;
}

void RTC_MZero::detachInterrupt()
{
  RTC_callBack = NULL;
}

uint32_t RTC_MZero::getMillis()
{
  RTCreadRequest();
  return (RTC->MODE0.COUNT.reg);
}

uint8_t RTC_MZero::getSeconds()
{
  return _second;
}

uint8_t RTC_MZero::getMinutes()
{
  return _minute;
}

uint8_t RTC_MZero::getHours()
{
  return _hour;
}

/*
 * Private Utility Functions
 */

/* Attach peripheral clock */
void RTC_MZero::configureClock() {
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(2);
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
    ;
  GCLK->GENCTRL.reg = (GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSC32K | GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_DIVSEL );
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
    ;
  GCLK->CLKCTRL.reg = (uint32_t)((GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2 | (RTC_GCLK_ID << GCLK_CLKCTRL_ID_Pos)));
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;
}

inline void RTC_MZero::RTCreadRequest() {
  if (_configured) {
    RTC->MODE0.READREQ.reg = RTC_READREQ_RREQ;
    while (RTCisSyncing())
      ;
  }
}

/* Configure the 32768Hz Oscillator */
void RTC_MZero::config32kOSC() 
{
  SYSCTRL->OSC32K.reg = SYSCTRL_OSC32K_ONDEMAND |
                         SYSCTRL_OSC32K_RUNSTDBY |
                         //SYSCTRL_OSC32K_EN1K |
                         SYSCTRL_OSC32K_EN32K |
                         SYSCTRL_OSC32K_STARTUP(6) |
                         SYSCTRL_OSC32K_ENABLE;
}


/* Wait for sync in write operations */
inline bool RTC_MZero::RTCisSyncing()
{
  return (RTC->MODE0.STATUS.bit.SYNCBUSY);
}

void RTC_MZero::RTCdisable()
{
  RTC->MODE0.CTRL.reg &= ~RTC_MODE0_CTRL_ENABLE; // disable RTC
  while (RTCisSyncing())
    ;
}

void RTC_MZero::RTCenable()
{
  RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_ENABLE; // enable RTC
  while (RTCisSyncing())
    ;
}

void RTC_MZero::RTCreset()
{
  RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_SWRST; // software reset
  while (RTCisSyncing())
    ;
}

void RTC_MZero::RTCresetRemove()
{
  RTC->MODE0.CTRL.reg &= ~RTC_MODE0_CTRL_SWRST; // software reset remove
  while (RTCisSyncing())
    ;
}
