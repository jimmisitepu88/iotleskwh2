#ifndef _IOTLESKWH_H_
#define _IOTLESKWH_H_

#include "Arduino.h"
#include "EEPROM.h"
#define EEPROM_SIZE 1000

#define _TIMEOUT                      50
#define _DEFAULT_BAUD_RATE            9600
#define _DEFAULT_PORT_CONFIG          SERIAL_8N1
#define _DEFAULT_RMS_UPDATE           400
#define _DEFAULT_AC_FREQUENCY         50

//iotleskwh
#define R1 0.51
#define R2 390
#define VREF 1.218
#define COEF_VOLT 73989
#define COEF_ARUS 305978
//#define RL 0.1961// 0.1963 ( error 0.09 )
//#define RL 0.1875
#define RL 0.2



struct frame_t 
{
    uint8_t Index;
    uint8_t Addr;
    uint8_t Payload[4];
    uint8_t Checksum;
};


class iotleskwh
{
  public:
    iotleskwh();
        void begin();
    void begin(HardwareSerial* hwSerial);

        float bacaVolt();  
        float bacaCur();   
        float bacaWatt();  
        float bacaKwh();      
        float getEnergy(uint32_t cf);
        float bacaFreq();  
        float bacaPF();

        uint8_t WA_CREEP();
        void relayON();
        void relayOFF();   
        void resetMeter();
        byte stsRelay();
        String idMeter();

        long readRegister(uint8_t regAddress);
        long readRegister(uint8_t regAddress,uint8_t deviceID);
        bool writeModeRegister();
        bool writeTpsRegister();
        bool writeRegister(uint8_t regAddress, uint32_t regValue);

        bool crc(uint8_t Addr ,uint8_t *ReqData, uint8_t dataLen,uint8_t _crc);
   private:
        
        HardwareSerial* _serial;
        uint8_t _rawHolder[21];
        uint32_t lastRcv = 0;
        uint32_t V_RMS_ADC = 0;
        uint32_t I_RMS_ADC = 0;
        int32_t  W_RMS_ADC = 0;
        uint32_t W_CF_CNT = 0;

    byte addr_eep_kwh = 1; 
        byte addr_sts_relay = 10;
        byte _sts_relay = 0;
        unsigned int eep_raw_data_kwh = 0;

        float _vol, _pwr, _cur;
        byte pinRelA = 32;
        byte pinRelB = 13;
        byte req_reset[3] = {0x1C, 0x5A5A5A};
};

#endif 