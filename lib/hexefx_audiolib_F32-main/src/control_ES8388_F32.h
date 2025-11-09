/**
 * @file control_ES8388_F32.h
 * @author your name (you@domain.com)
 * @brief driver for the ES8388 codec chip
 * @version 0.1
 * @date 2024-06-14
 * 
 * @copyright Copyright (c) 2024 www.hexefx.com
 * This program is free software: you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software Foundation, 
 * either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. 
 * If not, see <https://www.gnu.org/licenses/>."
 */
#ifndef _CONTROL_ES8388_F32_H_
#define _CONTROL_ES8388_F32_H_

#include <Arduino.h>
#include <Wire.h>
#include "AudioControl.h"

#define ES8388_I2C_ADDR_L	(0x10) 		// CS/ADD pin low
#define ES8388_I2C_ADDR_H	(0x11) 		// CS/ADD pin high

class AudioControlES8388_F32 : public AudioControl
{
public:
	AudioControlES8388_F32(void){};
	~AudioControlES8388_F32(void){};
	typedef enum
	{
		ES8388_CFG_LINEIN_SINGLE_ENDED = 0,
		ES8388_CFG_LINEIN_DIFF,
	}config_t;	


	bool enable()
	{
		return enable(&Wire, ES8388_I2C_ADDR_L, ES8388_CFG_LINEIN_SINGLE_ENDED);
	}
	bool enable(TwoWire *i2cBus, uint8_t addr,  config_t cfg);
	bool disable(void) { return false; }
	bool volume(float n);
	bool inputLevel(float n) {return true;} // range: 0.0f to 1.0f
	bool inputSelect(int n) {return true;}

	void set_noiseGate(float thres);
	
	void volume(uint8_t vol);
	uint8_t getOutVol();

	bool setInGain(uint8_t gain);
	uint8_t getInGain();

	bool analogBypass(bool bypass);
	bool analogSoftBypass(bool bypass);
private:
	static bool configured;
	TwoWire *ctrlBus;
	uint8_t i2cAddr;
	uint8_t dacGain;

	bool writeReg(uint8_t addr, uint8_t val);
	bool readReg(uint8_t addr, uint8_t* valPtr);
	uint8_t modifyReg(uint8_t reg, uint8_t val, uint8_t iMask);
	void optimizeConversion(uint8_t range);
};

#endif // _CONTROL_ES8388_F32_H_
