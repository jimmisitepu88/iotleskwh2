#include <iotleskwh.h>
iotleskwh pm;

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_I2CDevice.h>
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>             // Arduino SPI library
#define TFT_MOSI 23  // SDA Pin on ESP32
#define TFT_SCLK 18  // SCL Pin on ESP32
#define TFT_CS   5  // Chip select control pin
#define TFT_DC    17  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
unsigned long cur_time, old_time;
unsigned long cur_time_rst, old_time_rst;

int counter = 0;
#define pin_lcd 16

const int ledPin =  2;
int ledState = LOW;

#define INA 32
#define INB 13
#define pin_lcd 16

float dt_volt, dt_freq, dt_arus, dt_watt;
float dt_pf;
double dt_kwh = 0;
float tambah_token;
float sisa_token;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);
  pinMode(pin_lcd, OUTPUT);
  digitalWrite(pin_lcd, LOW);
  Serial.begin(115200);
  delay(1000);

  pm.begin(&Serial1); // begin init meter kwh
  tft.init(240, 320, SPI_MODE2);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);

  proses_meter();
  delay(100);
}

void loop() {
  cur_time = millis();
  if (cur_time - old_time >= 1000) {

    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    proses_meter();
    digitalWrite(ledPin, ledState);
    old_time = millis();
  }

  cur_time_rst = millis();
  if (cur_time_rst - old_time_rst >= 5000) {
    //tft.clearDisplay();
    old_time_rst = millis();
  }
}

void proses_meter() {
  Serial.println("baca kwh");
  dt_volt = pm.bacaVolt();delay(10); //beri delay utk baca stabil
  dt_freq = pm.bacaFreq();delay(10);
  dt_arus = pm.bacaCur();delay(10);
  dt_watt = pm.bacaWatt();delay(10);
  dt_kwh = pm.bacaKwh();delay(10);
  dt_pf = pm.bacaPF();delay(10);
  

  sisa_token = tambah_token - dt_kwh;
  if (sisa_token <= 0) {
    sisa_token = 0;
  }

  tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
  tft.setTextSize(3);
  tft.setCursor(45, 10);
  tft.print("iotLes kWh");

  int pos_x = 35;
  int pos_y = 45;

  tampil_txt("V", ST77XX_CYAN, 192, 14 + pos_y);
  tampil_txt("Vol", ST77XX_CYAN, 10, 14 + pos_y);
  tampil_nilai(dt_volt, ST77XX_CYAN, 10 + pos_x, 10 + pos_y);

  tampil_txt("Hz", ST77XX_CYAN, 192, 44 + pos_y);
  tampil_txt("Fre", ST77XX_CYAN, 10, 44 + pos_y);
  tampil_nilai(dt_freq, ST77XX_CYAN, 10 + pos_x, 40 + pos_y);

  tampil_txt("A", ST77XX_CYAN, 192, 74 + pos_y);
  tampil_txt("Cur", ST77XX_CYAN, 10, 74 + pos_y);
  tampil_nilai(dt_arus, ST77XX_CYAN, 10 + pos_x, 70 + pos_y);

  tampil_txt("W", ST77XX_CYAN, 192, 104 + pos_y);
  tampil_txt("Pwr", ST77XX_CYAN, 10, 104 + pos_y);
  tampil_nilai(dt_watt, ST77XX_CYAN, 10 + pos_x, 100 + pos_y);

  tampil_txt("kWh", ST77XX_CYAN, 192, 134 + pos_y);
  tampil_txt("Ene", ST77XX_CYAN, 10, 134 + pos_y);
  tampil_nilai(dt_kwh, ST77XX_CYAN, 10 + pos_x, 130 + pos_y);


  tampil_txt("%", ST77XX_CYAN, 192, 163 + pos_y);
  tampil_txt("Pf ", ST77XX_CYAN, 10, 163 + pos_y);
  tampil_nilai(dt_pf, ST77XX_CYAN, 10 + pos_x, 160 + pos_y);
}

void tampil_txt(char _buffer[50], int warna, int posX, int posY) {
  tft.setTextColor(warna, ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(posX, posY);
  tft.print(_buffer);
}

void tampil_nilai(float _dt, int warna,  int posX, int posY) {
  char _buffer[50];
  tft.setTextColor(warna, ST77XX_BLACK);
  tft.setTextSize(3);
  if (_dt < 1) {
    sprintf(_buffer, ":0%d.%04d", (unsigned int)_dt, (unsigned int)(_dt * 10000) % 10000);
    tft.setCursor(posX, posY);
    tft.print(_buffer);
  }
  if (_dt > 1 && _dt < 10) {
    sprintf(_buffer, ":0%d.%04d", (unsigned int)_dt, (unsigned int)(_dt * 10000) % 10000);
    tft.setCursor(posX, posY);
    tft.print(_buffer);
  }
  if (_dt >= 10 && _dt <  100) {
    sprintf(_buffer, ":%d.%04d", (unsigned int)_dt, (unsigned int)(_dt * 10000) % 10000);
    tft.setCursor(posX, posY);
    tft.print(_buffer);
  }
  if (_dt >= 100 && _dt < 1000) {
    sprintf(_buffer, ":%d.%03d", (unsigned int)_dt, (unsigned int)(_dt * 1000) % 1000);
    tft.setCursor(posX, posY);
    tft.print(_buffer);
  }
  if (_dt >= 1000 && _dt < 10000) {
    sprintf(_buffer, ":%d.%02d", (unsigned int)_dt, (unsigned int)(_dt * 100) % 100);
    tft.setCursor(posX, posY);
    tft.print(_buffer);
  }
  if (_dt >= 10000 && _dt < 100000) {
    sprintf(_buffer, ":%d.%01d", (unsigned int)_dt, (unsigned int)(_dt * 10) % 10);
    tft.setCursor(posX, posY);
    tft.print(_buffer);
  }
}

void relay_on() {
  digitalWrite(INA, LOW);
  digitalWrite(INB, HIGH);
  delay(100);
  digitalWrite(INA, LOW);
  digitalWrite(INB, LOW);
}

void relay_off() {
  digitalWrite(INA, HIGH);
  digitalWrite(INB, LOW);
  delay(100);
  digitalWrite(INA, LOW);
  digitalWrite(INB, LOW);
}