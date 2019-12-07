/*
 * lcdGdi.c:
 *	Graphics-based LCD driver.
 *	This is designed to drive the parallel interface LCD drivers
 *	based on the generic 12864H chips
 *
 *	There are many variations on these chips, however they all mostly
 *	seem to be similar.
 *	This implementation has the Pins from the Pi hard-wired into it,
 *	in particular wiringPi pins 0-7 so that we can use
 *	digitalWriteByete() to speed things up somewhat.
 *
 * Copyright (c) 2015 WHJWNAVY.
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <wiringPi.h>

#include "font.h"
#include "lcd192x96.h"

#define delay_ms(x) delay(x)

typedef enum lcd_disp_mode_e
{
  LCD_DISP_MODE_MONO = 0x10,
  LCD_DISP_MODE_GRAY = 0x11,
} lcd_disp_mode_t;

typedef enum lcd_send_mode_e
{
  LCD_DISP_MODE_DAT = 0,
  LCD_SEND_MODE_CMD,
} lcd_send_mode_t;

// Hardware Pins
#define LCD_GPIO_CS 21
#define LCD_GPIO_RST 22
#define LCD_GPIO_DC 23
#define LCD_GPIO_SDA 24
#define LCD_GPIO_SCL 25

#define LCD_GPIO_CS_Clr() digitalWrite(LCD_GPIO_CS, LOW);
#define LCD_GPIO_CS_Set() digitalWrite(LCD_GPIO_CS, HIGH);

#define LCD_GPIO_RST_Clr() digitalWrite(LCD_GPIO_RST, LOW);
#define LCD_GPIO_RST_Set() digitalWrite(LCD_GPIO_RST, HIGH);

#define LCD_GPIO_DC_Clr() digitalWrite(LCD_GPIO_DC, LOW);
#define LCD_GPIO_DC_Set() digitalWrite(LCD_GPIO_DC, HIGH);

#define LCD_GPIO_SCLK_Clr() digitalWrite(LCD_GPIO_SCL, LOW);
#define LCD_GPIO_SCLK_Set() digitalWrite(LCD_GPIO_SCL, HIGH);

#define LCD_GPIO_SDA_Clr() digitalWrite(LCD_GPIO_SDA, LOW);
#define LCD_GPIO_SDA_Set() digitalWrite(LCD_GPIO_SDA, HIGH);

// Software copy of the framebuffer
static uint8_t frameBuffer[LCD_DRV_PAGE_MAX][LCD_DRV_MAX_X] = {0};

static const uint8_t BIT_SET[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
static const uint8_t BIT_CLR[8] = {0xFE, 0XFD, 0XFB, 0XF7, 0XEF, 0XDF, 0XBF, 0X7F};

static int32_t lastX = 0, lastY = 0;
static int32_t mirrorX = 0, mirrorY = 0;

/*
 * sentData:
 *	Send an data or command byte to the display.
 *********************************************************************************
 */
static void lcd_drv_send_data(uint8_t dat, uint8_t cmd)
{
  uint8_t i = 0;
  uint8_t tmp = 0;
  LCD_GPIO_SCLK_Clr();
  if (cmd == LCD_SEND_MODE_CMD)
  {
    LCD_GPIO_DC_Clr();
  }
  else
  {
    LCD_GPIO_DC_Set();
  }
  LCD_GPIO_CS_Clr();
  for (i = 0; i < 8; i++)
  {
    LCD_GPIO_SCLK_Clr();
    tmp = dat;
    if ((tmp & 0x80) == 0x80)
    {
      LCD_GPIO_SDA_Set();
    }
    else
    {
      LCD_GPIO_SDA_Clr();
    }
    LCD_GPIO_SCLK_Set();
    dat = dat << 1;
  }
  LCD_GPIO_CS_Set();
  //LCD_GPIO_DC_Set();
}

void lcd_drv_set_pos(int32_t x0, int32_t y0)
{
  x0 = ((x0 >= LCD_DRV_MAX_X) ? (LCD_DRV_MAX_X - 1) : ((x0 < 0) ? 0 : x0));
  y0 = ((y0 >= LCD_DRV_PAGE_MAX) ? (LCD_DRV_PAGE_MAX - 1) : ((y0 < 0) ? 0 : y0));

  //y0 = ((y0 % LCD_DRV_PAGE_ROW == 0) ? (y0 / LCD_DRV_PAGE_ROW) : (y0 / LCD_DRV_PAGE_ROW + 1));

  lcd_drv_send_data(0x75, LCD_SEND_MODE_CMD);                 //Page Address setting
  lcd_drv_send_data(y0, LCD_DISP_MODE_DAT);                 // YS=0
  lcd_drv_send_data(LCD_DRV_PAGE_MAX - 1, LCD_DISP_MODE_DAT); // YE=95	  11->mono  23->gray

  lcd_drv_send_data(0x15, LCD_SEND_MODE_CMD);              //Clumn Address setting
  lcd_drv_send_data(x0, LCD_DISP_MODE_DAT);              // XS=0
  lcd_drv_send_data(LCD_DRV_MAX_X - 1, LCD_DISP_MODE_DAT); // XE=191
}

void lcd_drv_set_mode(void)
{
  lcd_drv_send_data(0x30, LCD_SEND_MODE_CMD); //EXT=0

  lcd_drv_send_data(0xF0, LCD_SEND_MODE_CMD);               //Display Mode
  lcd_drv_send_data(LCD_DISP_MODE_GRAY, LCD_DISP_MODE_DAT); //10=Mono, 11=4Gray

#if 0
  lcd_drv_send_data(0x75, LCD_SEND_MODE_CMD);                 //Page Address setting
  lcd_drv_send_data(0X00, LCD_DISP_MODE_DAT);                 // YS=0
  lcd_drv_send_data(LCD_DRV_PAGE_MAX - 1, LCD_DISP_MODE_DAT); // YE=95	  11->mono  23->gray

  lcd_drv_send_data(0x15, LCD_SEND_MODE_CMD);              //Clumn Address setting
  lcd_drv_send_data(0X00, LCD_DISP_MODE_DAT);              // XS=0
  lcd_drv_send_data(LCD_DRV_MAX_X - 1, LCD_DISP_MODE_DAT); // XE=191
#else
  lcd_drv_set_pos(0, 0);
#endif

  lcd_drv_send_data(0xCA, LCD_SEND_MODE_CMD);              // Display Control
  lcd_drv_send_data(0x00, LCD_DISP_MODE_DAT);              // CL Dividing Ratio Not Divide
  lcd_drv_send_data(LCD_DRV_MAX_Y - 1, LCD_DISP_MODE_DAT); //Duty Set 96 Duty
  lcd_drv_send_data(0x00, LCD_DISP_MODE_DAT);              //Frame Inversion
}

void lcd_drv_test_gray()
{
  uint8_t i = 0, j = 0;
  lcd_drv_set_mode();
  lcd_drv_send_data(0x5c, LCD_SEND_MODE_CMD);

  for (i = 0; i < LCD_DRV_PAGE_MAX / 4; i++)
  {
    for (j = 0; j < LCD_DRV_MAX_X; j++)
    {
      lcd_drv_send_data(0xff, LCD_DISP_MODE_DAT);
      delay_ms(1);
    }
  }

  for (i = 0; i < LCD_DRV_PAGE_MAX / 4; i++)
  {
    for (j = 0; j < LCD_DRV_MAX_X; j++)
    {
      lcd_drv_send_data(0xaa, LCD_DISP_MODE_DAT);
      delay_ms(1);
    }
  }

  for (i = 0; i < LCD_DRV_PAGE_MAX / 4; i++)
  {
    for (j = 0; j < LCD_DRV_MAX_X; j++)
    {
      lcd_drv_send_data(0x55, LCD_DISP_MODE_DAT);
      delay_ms(1);
    }
  }

  for (i = 0; i < LCD_DRV_PAGE_MAX / 4; i++)
  {
    for (j = 0; j < LCD_DRV_MAX_X; j++)
    {
      lcd_drv_send_data(0x00, LCD_DISP_MODE_DAT);
      delay_ms(1);
    }
  }
}

void lcd_drv_hw_init(void)
{
  wiringPiSetup();
  pinMode(LCD_GPIO_SCL, OUTPUT);
  pinMode(LCD_GPIO_SDA, OUTPUT);
  pinMode(LCD_GPIO_RST, OUTPUT);
  pinMode(LCD_GPIO_DC, OUTPUT);
  pinMode(LCD_GPIO_CS, OUTPUT);

  LCD_GPIO_RST_Set();
  delay_ms(10);
  LCD_GPIO_RST_Clr();
  delay_ms(10);
  LCD_GPIO_RST_Set();

  //lcd_drv_send_data(0x30, LCD_SEND_MODE_CMD); // Extension Command 1
  //lcd_drv_send_data(0x6E, LCD_SEND_MODE_CMD); //Enable Master
  lcd_drv_send_data(0x31, LCD_SEND_MODE_CMD); // Extension Command 2
  lcd_drv_send_data(0xD7, LCD_SEND_MODE_CMD); // Disable Auto Read
  lcd_drv_send_data(0x9F, LCD_DISP_MODE_DAT);
  //lcd_drv_send_data(0xE0, LCD_SEND_MODE_CMD); // Enable OTP Read
  //lcd_drv_send_data(0x00, LCD_DISP_MODE_DAT);
  delay_ms(10);
  //lcd_drv_send_data(0xE3, LCD_SEND_MODE_CMD); // OTP Up-Load
  delay_ms(20);
  //lcd_drv_send_data(0xE1, LCD_SEND_MODE_CMD); // OTP Control Out
  lcd_drv_send_data(0x30, LCD_SEND_MODE_CMD); // Extension Command 1
  lcd_drv_send_data(0x94, LCD_SEND_MODE_CMD); // Sleep Out
  lcd_drv_send_data(0xAE, LCD_SEND_MODE_CMD); // Display OFF
  delay_ms(50);

  lcd_drv_send_data(0x20, LCD_SEND_MODE_CMD); // Power Control
  lcd_drv_send_data(0x0B, LCD_DISP_MODE_DAT); // VB, VR, VF All ON

  lcd_drv_send_data(0x81, LCD_SEND_MODE_CMD); // Set Vop = 16V
  lcd_drv_send_data(0x28, LCD_DISP_MODE_DAT); //对比度设置,这里要根据自己的屏调整,不然可能会不显示
  lcd_drv_send_data(0x03, LCD_DISP_MODE_DAT);

  lcd_drv_send_data(0x31, LCD_SEND_MODE_CMD); // Extension Command 2

  lcd_drv_send_data(0x20, LCD_SEND_MODE_CMD); // Set Gray Scale Level
  lcd_drv_send_data(0x01, LCD_DISP_MODE_DAT);
  lcd_drv_send_data(0x03, LCD_DISP_MODE_DAT);
  lcd_drv_send_data(0x05, LCD_DISP_MODE_DAT);
  lcd_drv_send_data(0x07, LCD_DISP_MODE_DAT); //Light Gray Level Setting
  lcd_drv_send_data(0x09, LCD_DISP_MODE_DAT); //Light Gray Level Setting
  lcd_drv_send_data(0x0b, LCD_DISP_MODE_DAT); //Light Gray Level Setting
  lcd_drv_send_data(0x0d, LCD_DISP_MODE_DAT);
  lcd_drv_send_data(0x10, LCD_DISP_MODE_DAT);
  lcd_drv_send_data(0x11, LCD_DISP_MODE_DAT); //Dark Gray Level Setting
  lcd_drv_send_data(0x13, LCD_DISP_MODE_DAT);
  lcd_drv_send_data(0x15, LCD_DISP_MODE_DAT);
  lcd_drv_send_data(0x17, LCD_DISP_MODE_DAT); //Dark Gray Level Setting
  lcd_drv_send_data(0x19, LCD_DISP_MODE_DAT); //Dark Gray Level Setting
  lcd_drv_send_data(0x1b, LCD_DISP_MODE_DAT); //Dark Gray Level Setting
  lcd_drv_send_data(0x1d, LCD_DISP_MODE_DAT);
  lcd_drv_send_data(0x1f, LCD_DISP_MODE_DAT);

  lcd_drv_send_data(0x32, LCD_SEND_MODE_CMD); // Analog Circuit Set
  lcd_drv_send_data(0x00, LCD_DISP_MODE_DAT);
  lcd_drv_send_data(0x01, LCD_DISP_MODE_DAT); // Booster Efficiency =Level 1
  lcd_drv_send_data(0x02, LCD_DISP_MODE_DAT); //Bias=1/12

  lcd_drv_send_data(0x51, LCD_SEND_MODE_CMD); // Booster Level x10
  lcd_drv_send_data(0xFB, LCD_DISP_MODE_DAT);

  lcd_drv_send_data(0x30, LCD_SEND_MODE_CMD); // Extension Command 1

  lcd_drv_send_data(0xBC, LCD_SEND_MODE_CMD); // Data Scan Direction
  lcd_drv_send_data(0x00, LCD_DISP_MODE_DAT);

  lcd_drv_send_data(0x08, LCD_SEND_MODE_CMD); // Data Format Select, LSB is on bottom; D7->D0 (Default)
  //lcd_drv_send_data(0x0C, LCD_SEND_MODE_CMD); // Data Format Select, LSB is on top; D0->D7

  lcd_drv_send_data(0xA6, LCD_SEND_MODE_CMD); // Normal Display
  lcd_drv_send_data(0x31, LCD_SEND_MODE_CMD); // Extension Command 2
  lcd_drv_send_data(0x40, LCD_SEND_MODE_CMD); // Internal Power Supply

  lcd_drv_set_mode();

  lcd_drv_send_data(0x30, LCD_SEND_MODE_CMD); // Extension Command 1
  lcd_drv_send_data(0xAF, LCD_SEND_MODE_CMD); // Display ON

  //lcd_drv_test_gray();
  //delay_ms(1000);
}

/*
 * lcd_drv_update:
 *	Copy our software version to the real display
 *********************************************************************************
 */
void lcd_drv_update(void)
{
  uint8_t x = 0, y = 0;
  lcd_drv_set_mode();
  lcd_drv_send_data(0x5C, LCD_SEND_MODE_CMD); // write data to lcd
  for (y = 0; y < LCD_DRV_PAGE_MAX; y++)
  {
    for (x = 0; x < LCD_DRV_MAX_X; x++)
    {
      lcd_drv_send_data(frameBuffer[y][x], LCD_DISP_MODE_DAT);
    }
  }
}

/*
 * lcd_drv_set_point:
 *	Plot a pixel.
 *********************************************************************************
 */
void lcd_drv_set_point(int32_t x, int32_t y, int32_t colour)
{
  uint8_t bitmsk = 0;
  uint8_t bitmv = 0;
  uint8_t colour_t = (uint8_t)colour;
  uint8_t frameBuffer_t = 0;
  if (mirrorX)
    x = (LCD_DRV_MAX_X - x - 1);

  if (mirrorY)
    y = (LCD_DRV_MAX_Y - y - 1);

  lastX = x;
  lastY = y;

  if ((x < 0) || (x >= LCD_DRV_MAX_X) || (y < 0) || (y >= LCD_DRV_MAX_Y))
    return;

  colour_t = colour_t & LCD_DRV_COLOUR_BIT_MSK;
  bitmsk = (y % LCD_DRV_PAGE_ROW);
  bitmv = ((LCD_DRV_PAGE_ROW - bitmsk - 1) * LCD_DRV_COLOUR_BIT);

  frameBuffer_t = frameBuffer[y / LCD_DRV_PAGE_ROW][x];

  frameBuffer_t = (frameBuffer_t & ((uint8_t)(~(LCD_DRV_COLOUR_BIT_MSK << bitmv))));

  frameBuffer_t = (frameBuffer_t | ((uint8_t)(colour_t << bitmv)));

  frameBuffer[y / LCD_DRV_PAGE_ROW][x] = frameBuffer_t;
}

/*
 * lcd_drv_set_point:
 *	Plot a pixel.
 *********************************************************************************
 */
int32_t lcd_drv_get_point(int32_t x, int32_t y)
{
  uint8_t bitmsk = 0;
  uint8_t bitmv = 0;
  uint8_t frameBuffer_t = 0;
  uint8_t colour = 0;
  if (mirrorX)
    x = (LCD_DRV_MAX_X - x - 1);

  if (mirrorY)
    y = (LCD_DRV_MAX_Y - y - 1);

  if ((x < 0) || (x >= LCD_DRV_MAX_X) || (y < 0) || (y >= LCD_DRV_MAX_Y))
    return -1;

  bitmsk = (y % LCD_DRV_PAGE_ROW);
  bitmv = ((LCD_DRV_PAGE_ROW - bitmsk - 1) * LCD_DRV_COLOUR_BIT);

  frameBuffer_t = frameBuffer[y / LCD_DRV_PAGE_ROW][x];

  frameBuffer_t = ((uint8_t)(frameBuffer_t >> bitmv));
  colour = frameBuffer_t & LCD_DRV_COLOUR_BIT_MSK;

  return (int32_t)colour;
}

/*
 * lcd_drv_bmp_speed:
 *	Send a picture to the display. 
 *********************************************************************************
 */
void lcd_drv_bmp_speed(int32_t x0, int32_t y0, int32_t width, int32_t height, uint8_t *bmp, int32_t colour)
{
  int32_t x = 0, y = 0;
  int32_t width_t = width;
  int32_t i = 0;
  uint8 dat = 0;

  x0 = ((x0 >= LCD_DRV_MAX_X) ? (LCD_DRV_MAX_X - 1) : ((x0 < 0) ? 0 : x0));
  y0 = ((y0 >= LCD_DRV_MAX_Y) ? (LCD_DRV_MAX_Y - 1) : ((y0 < 0) ? 0 : y0));

#if 0
  if ((y0 % LCD_DRV_PAGE_ROW) != 0)
  {
    y0 = ((y0 / LCD_DRV_PAGE_ROW) + 1) * LCD_DRV_PAGE_ROW;
  }
#endif

  height = ((height + y0) >= LCD_DRV_MAX_Y) ? LCD_DRV_MAX_Y - y0 : height;
  y0 = ((y0 % LCD_DRV_PAGE_ROW == 0) ? (y0 / LCD_DRV_PAGE_ROW) : (y0 / LCD_DRV_PAGE_ROW + 1));
  width = ((width + x0) >= LCD_DRV_MAX_X) ? LCD_DRV_MAX_X - x0 : width;
  height = ((height % LCD_DRV_PAGE_ROW == 0) ? (height / LCD_DRV_PAGE_ROW) : (height / LCD_DRV_PAGE_ROW + 1));

  //lcd_drv_set_mode();
  #if 0
  for (y = y0; y < height; y++)
  {
    lcd_drv_set_pos(x0, y);
    lcd_drv_send_data(0x5C, LCD_SEND_MODE_CMD); // write data to lcd
    for (x = x0; x < width; x++)
    {
      if ((x >= LCD_DRV_MAX_X) || (y >= LCD_DRV_PAGE_MAX))
      {
        break;
      }
      dat = *bmp++;
      lcd_drv_send_data(((colour != 0) ? dat : ~dat), LCD_DISP_MODE_DAT);
    }
  }
  #else
  lcd_drv_set_pos(0, 0);
  lcd_drv_send_data(0x5C, LCD_SEND_MODE_CMD); // write data to lcd
  for (y = 0; y < LCD_DRV_PAGE_MAX; y++)
  {
    for (x = 0; x < LCD_DRV_MAX_X; x++)
    {
      if ((x < x0) || (x >= (width + x0)) || (y < y0) || (y0 >= (height + y0)))
      {
        dat = (colour != 0) ? 0x00 : (~0x00);
      } 
      else
      {
        dat = *bmp++;
      }
      lcd_drv_send_data(((colour != 0) ? dat : ~dat), LCD_DISP_MODE_DAT);
    }
  }
  #endif
  
}

/*
 * lcd_drv_open:
 *	Open hardware display.
 *********************************************************************************
 */
void lcd_drv_open(void)
{
#if 0
  lcd_drv_send_data(0X8D, LCD_SEND_MODE_CMD); //SET DCDC
  lcd_drv_send_data(0X14, LCD_SEND_MODE_CMD); //DCDC ON
  lcd_drv_send_data(0XAF, LCD_SEND_MODE_CMD); //DISPLAY ON
#endif
}

/*
 * lcd_drv_close:
 *	Cloase the hardware display.
 *********************************************************************************
 */
void lcd_drv_close(void)
{
#if 0
  lcd_drv_send_data(0X8D, LCD_SEND_MODE_CMD); //SET DCDC
  lcd_drv_send_data(0X10, LCD_SEND_MODE_CMD); //DCDC OFF
  lcd_drv_send_data(0XAE, LCD_SEND_MODE_CMD); //DISPLAY OFF
#endif
}

/*
 * lcd_drv_hw_clear:
 *	Clear the hardware display.
 *********************************************************************************
 */
void lcd_drv_hw_clear(void)
{
#if 0
  int32_t i, n;
  for (i = 0; i < 8; i++)
  {
    lcd_drv_send_data(0xb0 + i, LCD_SEND_MODE_CMD);
    lcd_drv_send_data(0x02, LCD_SEND_MODE_CMD);
    lcd_drv_send_data(0x10, LCD_SEND_MODE_CMD);
    for (n = 0; n < 128; n++)
    {
      lcd_drv_send_data(0, LCD_DISP_MODE_DAT);
    }
  }
#endif
}

/*
 * lcd_drv_clear:
 *	Clear the display to the given colour.
 *********************************************************************************
 */

void lcd_drv_clear(int32_t colour)
{
  int32_t x = 0, y = 0;
  int32_t col = 0;

  if (colour)
    col = 0xff;
  else
    col = 0x00;

  for (y = 0; y < LCD_DRV_PAGE_MAX; y++)
  {
    for (x = 0; x < LCD_DRV_MAX_X; x++)
    {
      frameBuffer[y][x] = col;
    }
  }
}

/*
 * lcd_drv_init:
 *	Initialise the display and GPIO.
 *********************************************************************************
 */
int32_t lcd_drv_init(void)
{
  lcd_drv_hw_init();

  lcd_drv_open();
#if LCD_DRV_INCLUDE_GUILIB
  lcd_drv_set_orientation(0);
#endif
  lcd_drv_clear(LCD_DRV_COLOUR_WHITE);
  lcd_drv_hw_clear();
  lcd_drv_update();

  return 0;
}

/*
 *********************************************************************************
 * Standard Graphical Functions
 *********************************************************************************
 */
#if LCD_DRV_INCLUDE_GUILIB
/*
 * lcd_drv_set_orientation:
 *	Set the display orientation:
 *	0: Normal, the display is portrait mode, 0,0 is top left
 *	1: Mirror x
 *	2: Mirror y
 *	3: Mirror x and y
 *********************************************************************************
 */
void lcd_drv_set_orientation(int32_t orientation)
{
  switch (orientation)
  {
  case 0:
    mirrorX = 0;
    mirrorY = 0;
    break;

  case 1:
    mirrorX = 1;
    mirrorY = 0;
    break;

  case 2:
    mirrorX = 0;
    mirrorY = 1;
    break;

  case 3:
    mirrorX = 1;
    mirrorY = 1;
    break;

  default:
    break;
  }
}

/*
 * lcd_drv_get_screen_size:
 *	Return the max X & Y screen sizes. Needs to be called again, if you 
 *	change screen orientation.
 *********************************************************************************
 */
void lcd_drv_get_screen_size(int32_t *x, int32_t *y)
{
  *x = LCD_DRV_MAX_X;
  *y = LCD_DRV_MAX_Y;
}

/*
 * lcd_drv_line: lcd_drv_lineto:
 *	Classic Bressenham Line code
 *********************************************************************************
 */
void lcd_drv_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t colour)
{
  int32_t dx, dy;
  int32_t sx, sy;
  int32_t err, e2;

  lastX = x1;
  lastY = y1;

  dx = abs(x1 - x0);
  dy = abs(y1 - y0);

  sx = (x0 < x1) ? 1 : -1;
  sy = (y0 < y1) ? 1 : -1;

  err = dx - dy;
  for (;;)
  {
    lcd_drv_set_point(x0, y0, colour);
    if ((x0 == x1) && (y0 == y1))
      break;

    e2 = 2 * err;

    if (e2 > -dy)
    {
      err -= dy;
      x0 += sx;
    }

    if (e2 < dx)
    {
      err += dx;
      y0 += sy;
    }
  }
}

void lcd_drv_lineto(int32_t x, int32_t y, int32_t colour)
{
  lcd_drv_line(lastX, lastY, x, y, colour);
}

/*
 * lcd_drv_rectangle:
 *	A rectangle is a spoilt days fishing
 *********************************************************************************
 */
void lcd_drv_rectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t colour, int32_t filled)
{
  int32_t x;
  if (filled)
  {
    if (x1 == x2)
    {
      lcd_drv_line(x1, y1, x2, y2, colour);
    }
    else if (x1 < x2)
    {
      for (x = x1; x <= x2; ++x)
      {
        lcd_drv_line(x, y1, x, y2, colour);
      }
    }
    else
    {
      for (x = x2; x <= x1; ++x)
      {
        lcd_drv_line(x, y1, x, y2, colour);
      }
    }
  }
  else
  {
    lcd_drv_line(x1, y1, x2, y1, colour);
    lcd_drv_lineto(x2, y2, colour);
    lcd_drv_lineto(x1, y2, colour);
    lcd_drv_lineto(x1, y1, colour);
  }
}

/*
 * lcd_drv_circle:
 *      This is the midpoint32 circle algorithm.
 *********************************************************************************
 */
void lcd_drv_circle(int32_t x, int32_t y, int32_t r, int32_t colour, int32_t filled)
{
  int32_t ddF_x = 1;
  int32_t ddF_y = -2 * r;

  int32_t f = 1 - r;
  int32_t x1 = 0;
  int32_t y1 = r;

  if (filled)
  {
    lcd_drv_line(x, y + r, x, y - r, colour);
    lcd_drv_line(x + r, y, x - r, y, colour);
  }
  else
  {
    lcd_drv_set_point(x, y + r, colour);
    lcd_drv_set_point(x, y - r, colour);
    lcd_drv_set_point(x + r, y, colour);
    lcd_drv_set_point(x - r, y, colour);
  }

  while (x1 < y1)
  {
    if (f >= 0)
    {
      y1--;
      ddF_y += 2;
      f += ddF_y;
    }
    x1++;
    ddF_x += 2;
    f += ddF_x;
    if (filled)
    {
      lcd_drv_line(x + x1, y + y1, x - x1, y + y1, colour);
      lcd_drv_line(x + x1, y - y1, x - x1, y - y1, colour);
      lcd_drv_line(x + y1, y + x1, x - y1, y + x1, colour);
      lcd_drv_line(x + y1, y - x1, x - y1, y - x1, colour);
    }
    else
    {
      lcd_drv_set_point(x + x1, y + y1, colour);
      lcd_drv_set_point(x - x1, y + y1, colour);
      lcd_drv_set_point(x + x1, y - y1, colour);
      lcd_drv_set_point(x - x1, y - y1, colour);
      lcd_drv_set_point(x + y1, y + x1, colour);
      lcd_drv_set_point(x - y1, y + x1, colour);
      lcd_drv_set_point(x + y1, y - x1, colour);
      lcd_drv_set_point(x - y1, y - x1, colour);
    }
  }
}

/*
 * lcd_drv_ellipse:
 *	Fast ellipse drawing algorithm by 
 *      John Kennedy
 *	Mathematics Department
 *	Santa Monica College
 *	1900 Pico Blvd.
 *	Santa Monica, CA 90405
 *	jrkennedy6@gmail.com
 *	-Confirned in email this algorithm is in the public domain -GH-
 *********************************************************************************
 */
static void plot_for_ellipse_points(int32_t cx, int32_t cy, int32_t x, int32_t y, int32_t colour, int32_t filled)
{
  if (filled)
  {
    lcd_drv_line(cx + x, cy + y, cx - x, cy + y, colour);
    lcd_drv_line(cx - x, cy - y, cx + x, cy - y, colour);
  }
  else
  {
    lcd_drv_set_point(cx + x, cy + y, colour);
    lcd_drv_set_point(cx - x, cy + y, colour);
    lcd_drv_set_point(cx - x, cy - y, colour);
    lcd_drv_set_point(cx + x, cy - y, colour);
  }
}

void lcd_drv_ellipse(int32_t cx, int32_t cy, int32_t xRadius, int32_t yRadius, int32_t colour, int32_t filled)
{
  int32_t x, y;
  int32_t xChange, yChange, ellipseError;
  int32_t twoAsquare, twoBsquare;
  int32_t stoppingX, stoppingY;

  twoAsquare = 2 * xRadius * xRadius;
  twoBsquare = 2 * yRadius * yRadius;

  x = xRadius;
  y = 0;

  xChange = yRadius * yRadius * (1 - 2 * xRadius);
  yChange = xRadius * xRadius;

  ellipseError = 0;
  stoppingX = twoBsquare * xRadius;
  stoppingY = 0;

  while (stoppingX >= stoppingY) // 1st set of point32s
  {
    plot_for_ellipse_points(cx, cy, x, y, colour, filled);
    ++y;
    stoppingY += twoAsquare;
    ellipseError += yChange;
    yChange += twoAsquare;

    if ((2 * ellipseError + xChange) > 0)
    {
      --x;
      stoppingX -= twoBsquare;
      ellipseError += xChange;
      xChange += twoBsquare;
    }
  }

  x = 0;
  y = yRadius;

  xChange = yRadius * yRadius;
  yChange = xRadius * xRadius * (1 - 2 * yRadius);

  ellipseError = 0;
  stoppingX = 0;
  stoppingY = twoAsquare * yRadius;

  while (stoppingX <= stoppingY) //2nd set of point32s
  {
    plot_for_ellipse_points(cx, cy, x, y, colour, filled);
    ++x;
    stoppingX += twoBsquare;
    ellipseError += xChange;
    xChange += twoBsquare;

    if ((2 * ellipseError + yChange) > 0)
    {
      --y;
      stoppingY -= twoAsquare;
      ellipseError += yChange;
      yChange += twoAsquare;
    }
  }
}

/*
 * lcd_drv_putc:
 *	Print a single character to the screen
 *********************************************************************************
 */
void lcd_drv_putc(int32_t x, int32_t y, int32_t c, int32_t bgCol, int32_t fgCol)
{
  int32_t y1, y2;

  uint8_t line;
  uint8_t *fontPtr;

  // Can't print if we're offscreen

  if ((x < 0) || (x > (LCD_DRV_MAX_X - fontWidth)) || (y < 0) || (y > (LCD_DRV_MAX_Y - fontHeight)))
    return;

  fontPtr = font + c * fontHeight;

  for (y1 = 0; y1 < fontHeight; y1++)
  {
    y2 = y + y1;
    line = *fontPtr++;
    lcd_drv_set_point(x + 0, y2, (line & 0x80) == 0 ? bgCol : fgCol);
    lcd_drv_set_point(x + 1, y2, (line & 0x40) == 0 ? bgCol : fgCol);
    lcd_drv_set_point(x + 2, y2, (line & 0x20) == 0 ? bgCol : fgCol);
    lcd_drv_set_point(x + 3, y2, (line & 0x10) == 0 ? bgCol : fgCol);
    lcd_drv_set_point(x + 4, y2, (line & 0x08) == 0 ? bgCol : fgCol);
    lcd_drv_set_point(x + 5, y2, (line & 0x04) == 0 ? bgCol : fgCol);
    lcd_drv_set_point(x + 6, y2, (line & 0x02) == 0 ? bgCol : fgCol);
    lcd_drv_set_point(x + 7, y2, (line & 0x01) == 0 ? bgCol : fgCol);
  }
}

/*
 * lcd_drv_puts:
 *	Send a string to the display. Obeys \n and \r formatting
 *********************************************************************************
 */
void lcd_drv_puts(int32_t x, int32_t y, const char *str, int32_t bgCol, int32_t fgCol)
{
  int32_t c, mx, my;

  mx = x;
  my = y;

  while (*str)
  {
    c = *str++;

    if (c == '\r')
    {
      mx = x;
      continue;
    }

    if (c == '\n')
    {
      my += fontHeight;
      continue;
    }

    lcd_drv_putc(mx, my, c, bgCol, fgCol);

    mx += fontWidth;
    //if (mx >= (LCD_DRV_MAX_X - fontWidth))
    if (mx > (LCD_DRV_MAX_X - fontWidth))
    {
      mx = 0;
      my += fontHeight;
    }
  }
}

/*
 * lcd_drv_putn:
 *	Send a number to the display. 
 *********************************************************************************
 */
void lcd_drv_putn(int32_t x, int32_t y, int32_t num, int32_t bgCol, int32_t fgCol)
{
  int8_t numString[50] = {0};
  if (sprintf(numString, "%d", num) < 0)
    return;
  lcd_drv_puts(x, y, numString, bgCol, fgCol);
}

/*
 * lcd_drv_bmp:
 *	Send a picture to the display. 
 *********************************************************************************
 */
void lcd_drv_bmp(int32_t x0, int32_t y0, int32_t with, int32_t height, uint8_t *bmp, int32_t colour)
{
  int32_t x = 0, y = 0;
  uint8_t data = 0;

  for (y = y0; y < (height / 8); y++)
  {
    for (x = x0; x < with; x++)
    {
      data = *bmp++;
      frameBuffer[y][x] = ((colour != 0) ? data : ~data);
    }
  }
}
#endif