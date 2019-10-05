/*

Arduino library for Texas Instruments OPT3001 Digital Ambient Light Sensor
Written by AA for ClosedCube
---

The MIT License (MIT)

Copyright (c) 2015 ClosedCube Limited

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include "ClosedCube_OPT3001.h"
#include "api_debug.h"
#include "api_os.h"

ClosedCube_OPT3001::ClosedCube_OPT3001(uint8_t address) : _address(address) {}

OPT3001_ErrorCode ClosedCube_OPT3001::begin(I2C_ID_t i2c)
{
  	_i2c = i2c;
  	_i2cConfig.freq = I2C_FREQ_100K;
  	I2C_Init(_i2c, _i2cConfig);
	return NO_ERROR;
}

uint16_t ClosedCube_OPT3001::readManufacturerID() {
	uint16_t result = 0;
	OPT3001_ErrorCode error = writeData(MANUFACTURER_ID);
	if (error == NO_ERROR)
		error = readData(&result);
	return result;
}

uint16_t ClosedCube_OPT3001::readDeviceID() {
	uint16_t result = 0;
	OPT3001_ErrorCode error = writeData(DEVICE_ID);
	if (error == NO_ERROR)
		error = readData(&result);
	return result;
}

OPT3001_Config ClosedCube_OPT3001::readConfig() {
	OPT3001_Config config;
	OPT3001_ErrorCode error = writeData(CONFIG);
	if (error == NO_ERROR)
		error = readData(&config.rawData);
	return config;
}

OPT3001_ErrorCode ClosedCube_OPT3001::writeConfig(OPT3001_Config config) {
	uint8_t data[3] = {
		CONFIG,					// Register
		config.rawData >> 8,	// Upper 8-bits
		config.rawData & 0x00FF // Lower 8-bits
	};
	I2C_Error_t error = I2C_Transmit(_i2c, _address, &data[0], 3, I2C_DEFAULT_TIME_OUT);
	if (error != I2C_ERROR_NONE)
	{
		Trace(1, "ClosedCube_OPT3001::writeConfig transmit error: 0X%02x", error);
		return WIRE_I2C_UNKNOW_ERROR;
	}

	return NO_ERROR;
}

OPT3001 ClosedCube_OPT3001::readResult() {
	return readRegister(RESULT);
}

OPT3001 ClosedCube_OPT3001::readHighLimit() {
	return readRegister(HIGH_LIMIT);
}

OPT3001 ClosedCube_OPT3001::readLowLimit() {
	return readRegister(LOW_LIMIT);
}

OPT3001 ClosedCube_OPT3001::readRegister(OPT3001_Commands command) {
	OPT3001_ErrorCode error = writeData(command);
	if (error == NO_ERROR) {
		OPT3001 result;
		result.lux = 0;
		result.error = NO_ERROR;

		OPT3001_ER er;
		error = readData(&er.rawData);
		if (error == NO_ERROR) {

			switch (er.Exponent)
			{
				case 0:
					result.lux = 0.01 * er.Result * 0.01;
					break;
				case 1:
					result.lux = 0.01 * er.Result * 0.02;
					break;
				case 2:
					result.lux = 0.01 * er.Result * 0.04;
					break;
				case 3:
					result.lux = 0.01 * er.Result * 0.08;
					break;
				case 4:
					result.lux = 0.01 * er.Result * 0.16;
					break;
				case 5:
					result.lux = 0.01 * er.Result * 0.32;
					break;
				case 6:
					result.lux = 0.01 * er.Result * 0.64;
					break;
				case 7:
					result.lux = 0.01 * er.Result * 1.28;
					break;
				case 8:
					result.lux = 0.01 * er.Result * 2.56;
					break;
				case 9:
					result.lux = 0.01 * er.Result * 5.12;
					break;
				case 10:
					result.lux = 0.01 * er.Result * 10.24;
					break;
				case 11:
					result.lux = 0.01 * er.Result * 20.48;
					break;
			}
			result.raw = er;
		}
		else {
			result.error = error;
		}

		return result;
	}
	else {
		return returnError(error);
	}
}

OPT3001_ErrorCode ClosedCube_OPT3001::writeData(OPT3001_Commands command)
{
	uint8_t reg = command;
	I2C_Error_t error = I2C_Transmit(_i2c, _address, &reg, 1, I2C_DEFAULT_TIME_OUT);
	if (error != I2C_ERROR_NONE)
	{
		Trace(1, "ClosedCube_OPT3001::writeData transmit error: 0X%02x", error);
		return WIRE_I2C_UNKNOW_ERROR;
	}

	return NO_ERROR;
}

OPT3001_ErrorCode ClosedCube_OPT3001::readData(uint16_t* data)
{
	uint8_t buf[2];

	int counter = 0;
	I2C_Error_t error = I2C_Receive(_i2c, _address, &buf[0], 2, I2C_DEFAULT_TIME_OUT);
	while (error != I2C_ERROR_NONE)
	{
		counter++;
		OS_Sleep(10);
		if (counter > 250) {
			Trace(1, "ClosedCube_OPT3001::readData recieve error: 0X%02x", error);
			return TIMEOUT_ERROR;
		}
	}

	*data = (buf[0] << 8) | buf[1];

	return NO_ERROR;
}


OPT3001 ClosedCube_OPT3001::returnError(OPT3001_ErrorCode error) {
	OPT3001 result;
	result.lux = 0;
	result.error = error;
	return result;
}
