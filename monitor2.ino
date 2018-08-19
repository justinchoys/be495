#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
#define PSTR // Make Arduino Due happy
#endif

#define PIN 6

SoftwareSerial BTSerial(10,11); // RX | TX

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(5, 8, PIN,
NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
NEO_GRB            + NEO_KHZ800);

const uint16_t colors[] = {matrix.Color(255, 255, 255)};


void setup() {
  BTSerial.begin(9600);
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(colors[0]);
}

void loop() {
  if(BTSerial.available())
  {
    int inByte = BTSerial.read();
    reportValue(inByte);
    delay(100); 
  }
}


void reportValue(int x) {
  matrix.fillScreen(0);
  matrix.setCursor(0, 0);
  matrix.print(char(x));
  matrix.show();
}





