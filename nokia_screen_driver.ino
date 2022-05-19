#ifndef NOKIA_SCREEN_DRIVER_INO
#define NOKIA_SCREEN_DRIVER_INO

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

const int clk = 9, din = 13, d_c = 12, ce = 10, rst = 11;
Adafruit_PCD8544 nokiaScreen = Adafruit_PCD8544(clk, din, d_c, ce, rst);

enum SM_LCD_DRIVER_States
{
  SM_LCD_DRIVER_INIT,
  SM_LCD_DRIVER_SCREEN
};

int SM_LCD_DRIVER_Tick(int state)
{
  switch (state)
  {
  case SM_LCD_DRIVER_INIT:
    state = SM_LCD_DRIVER_SCREEN;
    break;
  case SM_LCD_DRIVER_SCREEN:
    break;
  }
  switch (state)
  {
  case SM_LCD_DRIVER_INIT:
    break;
  case SM_LCD_DRIVER_SCREEN:
    // Serial.print("xAxis: ");
    // Serial.println(analogRead(xAxis));
    // Serial.print("yAxis: ");
    // Serial.println(analogRead(yAxis));

    break;
  }
  return state;
}

#endif