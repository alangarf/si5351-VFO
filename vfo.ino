#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>
#include <Bounce2.h>
#include "si5351.h"
#include "Wire.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define SCREEN_HZ_WIDTH 14
#define SCREEN_FREQ_POS (SCREEN_WIDTH - SCREEN_HZ_WIDTH)

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13

#define ENC_A 3
#define ENC_B 2
#define ENC_BUTTON 4

#define STARTUP_FREQ 7000000
#define STARTUP_RADIX_POS 2 // 100's

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                         OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
Encoder myEnc(ENC_A, ENC_B);
Bounce debouncer = Bounce();
Si5351 si5351;

long freq = STARTUP_FREQ;
int radix_pos = STARTUP_RADIX_POS;

long radix = 1;
long oldPosition  = -9999;

void setup() {
  Serial.begin(9600);

  debouncer.attach(ENC_BUTTON, INPUT_PULLUP);
  debouncer.interval(5);

  bool i2c_found;
  if (! si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0)) {
    Serial.println("Device not found on I2C bus!");
    for (;;);  // Don't proceed, loop forever
  }

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();
  
  display.setTextSize(2);
  display.setTextColor(WHITE);
}

void loop() {
  # debounce the dial button
  debouncer.update();
  int button = debouncer.read();
  
  # get encoders current position
  long newPosition = myEnc.read() >> 2;
  int dir = 0;

  if (newPosition != oldPosition) {
    if (oldPosition > newPosition) {
      dir = 1;
    } else {
      dir = -1;
    }

    if (button) {
      // frequency mode
      freq += (dir * radix);

      if (freq < 8000) {
        freq = 8000;
      }

      if (freq > 160000000) {
        freq = 160000000;
      }

      si5351.set_freq(freq * 100, SI5351_CLK0);

    } else {
      // radix adjust
      radix_pos += dir;

      if (radix_pos < 0) {
        radix_pos = 0;
      }
      if (radix_pos > 7) {
        radix_pos = 7;
      }

      radix = 1;
      for (int l = 0; l < radix_pos; l++) {
        radix = radix * 10;
      }
    }

    oldPosition = newPosition;
  }

  display.clearDisplay();
  display.setTextSize(2);

  String tFreq = String(freq);
  display.setCursor(SCREEN_FREQ_POS - (tFreq.length() * 12), 10);
  display.println(tFreq);

  display.setCursor(SCREEN_FREQ_POS - ((radix_pos + 1) * 12), 16);
  display.println("_");

  display.setTextSize(1);
  display.setCursor(SCREEN_FREQ_POS, 10);
  display.print("Hz");
  display.display();
}
