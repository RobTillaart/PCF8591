//
//    FILE: PCF8591.cpp
//  AUTHOR: Rob Tillaart
//    DATE: 2020-03-12
// VERSION: 0.0.2
// PURPOSE: I2C PCF8591 library for Arduino
//     URL: https://github.com/RobTillaart/PCF8591
//
// HISTORY:
// 0.0.1    2020-03-12 initial version
// 0.0.2    2020-07-22 testing, refactor, documentation and examples 

#include "PCF8591.h"

PCF8591::PCF8591(const uint8_t address)
{
  _address = address;
  _control = 0;
  _dac = 0;
  for (uint8_t i = 0; i < 4; i++) _adc[i] = 0;
  _error = PCF8591_OK;
}


#if defined (ESP8266) || defined(ESP32)
void PCF8591::begin(uint8_t sda, uint8_t scl, uint8_t val)
{
  Wire.begin(sda, scl);
  analogWrite(val);
}
#endif

void PCF8591::begin(uint8_t val)
{
  Wire.begin();
  analogWrite(val);
}

bool PCF8591::isConnected()
{
  Wire.beginTransmission(_address);
  _error = Wire.endTransmission();  // default == 0 == PCF8591_OK
  return( _error == PCF8591_OK);
}

// ADC PART
uint8_t PCF8591::analogRead(uint8_t channel, uint8_t mode)
{
  if (mode > 3)
  {
    _error = PCF8591_MODE_ERROR;
    return 0;
  }
  _control &= 0b01000100;         // clear all except flags
  _control |= (mode << 4);

  _error = PCF8591_CHANNEL_ERROR;
  switch(mode)
  {
    case 0:
      if (channel > 3) return 0;
      _control |= channel;
      break;
    case 1:
      if (channel > 2) return 0;
      _control |= (channel << 4);
      break;
    case 2:
      if (channel > 2) return 0;
      _control |= (channel << 4);
      break;
    case 3:
      if (channel > 1) return 0;
      _control |= (channel << 4);
      break;
    default:
      return 0;
  }
  _error = PCF8591_OK;

  // NOTE: one must read two values to get an up to date value. 
  //       Page 8 datasheet.
  Wire.beginTransmission(_address);
  Wire.write(_control);
  _error = Wire.endTransmission();  // default == 0 == PCF8591_OK
  if (_error != 0) return PCF8591_I2C_ERROR;
  if (Wire.requestFrom(_address, (uint8_t)2) != 2)
  {
    _error = PCF8591_I2C_ERROR;
    return _adc[channel];          // known last value
  }
  Wire.read();
  _adc[channel] = Wire.read();
  return _adc[channel];
}


void PCF8591::analogRead4()
{
  _control &= 0b01000100;         // clear all except flags
  uint8_t channel = 0;
  _control |= channel;

  
  enableINCR();
  Wire.beginTransmission(_address);
  Wire.write(_control);
  _error = Wire.endTransmission();  // default == 0 == PCF8591_OK
  if (_error != 0) 
  {
    disableINCR();
    return PCF8591_I2C_ERROR;
  }
  if (Wire.requestFrom(_address, (uint8_t)5) != 5)
  {
    _error = PCF8591_I2C_ERROR;
    disableINCR();
    return;
  }

  Wire.read();
  for (uint8_t i = 0; i < 4; i++)
  {
    _adc[i] = Wire.read();
  }
  _error = PCF8591_OK;
  disableINCR();
  return;
}


// DAC PART
void PCF8591::analogWrite(uint8_t value)
{
  _dac = value;
  Wire.beginTransmission(_address);
  Wire.write(_control);
  Wire.write(_dac);
  _error = Wire.endTransmission();
  return;
}

// -- END OF FILE --
