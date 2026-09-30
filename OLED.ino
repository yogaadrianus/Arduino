#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     4
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(2, 2);
  display.setTextColor(WHITE);
  display.setCursor(5, 20);
  display.println("Hello !");
  display.setTextSize(1);
  display.setCursor(5, 40);
  display.println("Stella Gracia");
  display.setCursor(5, 50);
  display.println("     Lampung");
  display.display();
  delay(5000);
}

void loop() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 0);
  display.println("Scrolling Text");
  display.display();
  delay(100);

  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
  delay(1000);
}