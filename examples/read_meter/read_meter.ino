#include <WiFi.h>
#include <iotleskwh.h>
iotleskwh pm;
uint8_t mac[6];

unsigned long cur_time, old_time;
String idKWH;
String new_idKWH;
char chr_idKWH[20];

float dt_volt, dt_freq, dt_arus, dt_watt;
float dt_pf;
double dt_kwh = 0;

const int ledPin =  2;
int ledState = LOW;
byte sts_relay = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  pm.begin(&Serial1);
  idKWH = pm.idMeter();
  idKWH.toCharArray(chr_idKWH, idKWH.length() + 1);
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  new_idKWH = hash_idKWH();

  pm.relayON();// relay di on kan
  //pm.relayOFF();
  sts_relay = pm.stsRelay();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  cur_time = millis();
  if (cur_time - old_time >= 1000) {
    proses_meter();
    print_meter();

    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(ledPin, ledState);
    old_time = millis();
  }
}

void proses_meter() {
  
  dt_volt = pm.bacaVolt();delay(10); // beri delay utk pembacaan stabil
  dt_freq = pm.bacaFreq();delay(10);
  dt_arus = pm.bacaCur();delay(10);
  dt_watt = pm.bacaWatt();delay(10);
  dt_kwh = pm.bacaKwh();delay(10);
  dt_pf = pm.bacaPF();delay(10);
  
}

void print_meter() {
  Serial.print("new idkwh: ");
  Serial.println(new_idKWH);

  Serial.print("volt: "); Serial.println(dt_volt);
  Serial.print("freq: "); Serial.println(dt_freq);
  Serial.print("arus: "); Serial.println(dt_arus);
  Serial.print("watt: "); Serial.println(dt_watt);
  Serial.print("kwh: "); Serial.println(dt_kwh);
  Serial.print("pf: "); Serial.println(dt_pf);
  Serial.print("sts relay: "); Serial.println(sts_relay);
}

String hash_idKWH() {
  String _buf_str = "";
  _buf_str = String(mac[0], HEX) + String(chr_idKWH[0]);
  _buf_str += String(mac[1], HEX) + String(chr_idKWH[1]);
  _buf_str += String(mac[2], HEX) + String(chr_idKWH[2]);
  _buf_str += String(mac[3], HEX) + String(chr_idKWH[3]);
  _buf_str += String(mac[4], HEX) + String(chr_idKWH[4]);
  _buf_str += String(mac[5], HEX) + String(chr_idKWH[5]);
  return _buf_str;
}