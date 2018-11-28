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

#ifndef RTC_M_ZERO_H
#define RTC_M_ZERO_H

#include "Arduino.h"

typedef void(*voidFuncPtr)(void);

class RTC_MZero {
public:


  RTC_MZero();

  void begin(bool resetTime = false);

  void attachInterrupt(voidFuncPtr callback);
  void detachInterrupt();
  
  void standbyMode();
  
  /* Get Functions */
  uint32_t getMillis();
  uint8_t getSeconds();
  uint8_t getMinutes();
  uint8_t getHours();

  bool isConfigured() {
    return _configured;
  }

private:
  bool _configured;
  uint16_t _millis;
  uint8_t _second;
  uint8_t _minute;
  uint8_t _hour;

  void config32kOSC(void);
  void configureClock(void);
  void RTCreadRequest();
  bool RTCisSyncing(void);
  void RTCdisable();
  void RTCenable();
  void RTCreset();
  void RTCresetRemove();
};

#endif // RTC_M_ZERO_H
