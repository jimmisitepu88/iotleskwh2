#include "iotleskwh.h"

iotleskwh::iotleskwh()
{
}

void iotleskwh::begin(HardwareSerial* hwSerial)
{
  pinMode(pinRelA, OUTPUT);
  pinMode(pinRelB, OUTPUT);
  _serial = hwSerial;
  _serial->begin(_DEFAULT_BAUD_RATE, _DEFAULT_PORT_CONFIG, 25, 26);
  if (!EEPROM.begin(EEPROM_SIZE)) {
    delay(1000);
    //ESP.restart();
  }
  eep_raw_data_kwh = EEPROM.readUInt(addr_eep_kwh);
  _sts_relay = EEPROM.readByte(addr_sts_relay);
}

void iotleskwh::begin()
{
}

String iotleskwh::idMeter(){
  String _str_meter = "";
  uint64_t _chipid;
  char chr_chipid[13];
  _chipid = ESP.getEfuseMac();
  uint16_t chip = (uint16_t)(_chipid >> 32);
  snprintf(chr_chipid, 13, "%04X%08X", chip, (uint32_t)_chipid);
  for ( int i = 0; i < 12; i++) {
    _str_meter += String(chr_chipid[i]);
  }
  return _str_meter;
}


float iotleskwh::bacaVolt()
{
  uint8_t Addr = 0x04;
  uint32_t get_reg_data = readRegister(Addr);

  float Data = get_reg_data * VREF * ((R2 * 5) + R1) / (COEF_VOLT * R1 * 1000);

  _vol = Data;
  return Data;
}

float iotleskwh::bacaCur()
{
  uint8_t Addr = 0x03;
  uint32_t get_reg_data = readRegister(Addr);
  if (get_reg_data != 0)
  {
    I_RMS_ADC = get_reg_data;
  }
  else
  {
    if (V_RMS_ADC < 1 && get_reg_data == 0)
    {
      I_RMS_ADC = 0;
    }

    get_reg_data = I_RMS_ADC;
  }

  float Data =  (get_reg_data * VREF) / (COEF_ARUS * RL) ;
  _cur = Data;
  return Data;
}

float iotleskwh::bacaWatt() 
{
  uint8_t Addr = 0x06;
  int32_t get_reg_data = readRegister(Addr);
  //Serial.print("reg: ");
  //Serial.println(get_reg_data);

  if (get_reg_data != 0)
  {
    W_RMS_ADC = get_reg_data;
  }
  else
  {
    if (V_RMS_ADC < 1 && get_reg_data == 0)
    {
      W_RMS_ADC = 0;
    }
    get_reg_data = W_RMS_ADC;
  }

  unsigned int _oneC = ~get_reg_data;
  get_reg_data = _oneC + 1;
  unsigned int _mask = 0b00000000111111111111111111111111;
  get_reg_data &= _mask;


  float Data = (get_reg_data * VREF * VREF * ( (R2 * 5) + R1)) / (3537 * RL * R1 * 1000);
  if (Data > 100000)Data = 0;
  _pwr = Data;

  return Data;
}

float iotleskwh::bacaPF() {
  float Data = _pwr / (_cur * _vol);
  if (isnan(Data)) {
    Data = 0;
  }
  return Data;
}

float iotleskwh::bacaKwh() 
{
  uint8_t Addr = 0x07;
  uint32_t get_reg_data = readRegister(Addr);

  if (get_reg_data != 0)
  {
    W_CF_CNT = get_reg_data;
  }
  else
  {
    get_reg_data = W_CF_CNT;
  }

  if (get_reg_data < eep_raw_data_kwh) {
    get_reg_data = get_reg_data + eep_raw_data_kwh;
  } else {
    get_reg_data = get_reg_data;
  }

  EEPROM.writeUInt(addr_eep_kwh, get_reg_data);
  EEPROM.commit();

  float a = (1638.4 * 256 * VREF * VREF * ((R2 * 5) + R1)) / 10000;
  float b = 360 * 3537 * RL * R1 * 1000;
  float Data = (a / b) * get_reg_data;
  return Data;
}

float iotleskwh::bacaFreq()
{
  uint8_t Addr = 0x08;
  long get_reg_data = readRegister(Addr);
  float Data = 1000000 / float(get_reg_data);
  return Data;
}

uint8_t iotleskwh::WA_CREEP()
{
  uint8_t Addr = 0x14;
  long get_reg_data = readRegister(Addr);
  return get_reg_data;
}


long iotleskwh::readRegister(uint8_t regAddress)
{
  lastRcv = millis();
  uint16_t Bytes = 4;
  uint16_t Index = 0;
  long regValue = 0;

  uint8_t Start = 0x58;
  uint8_t regAddr = regAddress;
  uint8_t packet[] = {Start, regAddr};
  _serial->write(packet, sizeof(packet));

  uint8_t Payload[Bytes];

  while ((Index < Bytes) && !((millis() - lastRcv) > _TIMEOUT))
  {
    uint8_t bytesAvailable = _serial->available();
    if (bytesAvailable > 0)
    {
      for (uint16_t bytesIndex = 0; bytesIndex < bytesAvailable; bytesIndex++)
      {
        Payload[Index++] = (uint8_t)_serial->read();
      }
      lastRcv = millis();
    }
  }

  if (Index != Bytes)
  {
    //_state = BADFRAME_ERR;
    return 0;
  }


  if (crc(regAddr, Payload, sizeof(Payload), Payload[3]) != true)
  {
    //_state = BADFRAME_ERR;
    return 0;
  }

  long Data = (((uint32_t)Payload[2] << 24) + ((uint32_t)Payload[1] << 16) + ((uint32_t)Payload[0] << 8)) >> 8;
  return Data;
}

bool iotleskwh::crc(uint8_t Addr, uint8_t *ReqData, uint8_t dataLen, uint8_t _crc)
{
  uint8_t startData = 0x58;
  uint8_t checkData = 0;
  for (uint8_t a = 0; a < dataLen - 1; a++)
  {
    checkData = checkData + ReqData[a];
  }
  checkData = checkData + startData + Addr;
  checkData = ~checkData;
  if (_crc != checkData)
  {
    return false;
  }
  else
  {
    return true;
  }
}

void iotleskwh::relayOFF() {
  _sts_relay = 0;
  EEPROM.writeByte(addr_sts_relay, _sts_relay); 
  EEPROM.commit();

  digitalWrite(pinRelA, LOW);
  digitalWrite(pinRelB, HIGH);
  delay(100);
  digitalWrite(pinRelA, LOW);
  digitalWrite(pinRelB, LOW);
}

void iotleskwh::relayON() {
  _sts_relay = 1;
  EEPROM.writeByte(addr_sts_relay, _sts_relay); 
  EEPROM.commit();

  digitalWrite(pinRelA, HIGH);
  digitalWrite(pinRelB, LOW);
  delay(100);
  digitalWrite(pinRelA, LOW);
  digitalWrite(pinRelB, LOW);
}

void iotleskwh::resetMeter() {
  _serial->write(req_reset, sizeof(req_reset));
}

byte iotleskwh::stsRelay(){
    byte Data = _sts_relay;
    return Data;
}