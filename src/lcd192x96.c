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
static uint8_t frameBuffer[LCD_HEIGHT_PAGE_MAX][LCD_WIDTH_X] = {0};

static const uint8_t BIT_SET[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
static const uint8_t BIT_CLR[8] = {0xFE, 0XFD, 0XFB, 0XF7, 0XEF, 0XDF, 0XBF, 0X7F};

static int32_t lastX = 0, lastY = 0;
static int32_t mirrorX = 0, mirrorY = 0;

/*
 * sentData:
 *	Send an data or command byte to the display.
 *********************************************************************************
 */
static void lcdDrvSendData(uint8_t dat, uint8_t cmd)
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

void lcdDrvSetMode(void)
{
  lcdDrvSendData(0x30, LCD_SEND_MODE_CMD); //EXT=0

  lcdDrvSendData(0xF0, LCD_SEND_MODE_CMD);               //Display Mode
  lcdDrvSendData(LCD_DISP_MODE_GRAY, LCD_DISP_MODE_DAT); //10=Mono, 11=4Gray

  lcdDrvSendData(0x75, LCD_SEND_MODE_CMD);                    //Page Address setting
  lcdDrvSendData(0X00, LCD_DISP_MODE_DAT);                    // YS=0
  lcdDrvSendData(LCD_HEIGHT_PAGE_MAX - 1, LCD_DISP_MODE_DAT); // YE=95	  11->mono  23->gray

  lcdDrvSendData(0x15, LCD_SEND_MODE_CMD);            //Clumn Address setting
  lcdDrvSendData(0X00, LCD_DISP_MODE_DAT);            // XS=0
  lcdDrvSendData(LCD_WIDTH_X - 1, LCD_DISP_MODE_DAT); // XE=191

  lcdDrvSendData(0xCA, LCD_SEND_MODE_CMD);             // Display Control
  lcdDrvSendData(0x00, LCD_DISP_MODE_DAT);             // CL Dividing Ratio Not Divide
  lcdDrvSendData(LCD_HEIGHT_Y - 1, LCD_DISP_MODE_DAT); //Duty Set 96 Duty
  lcdDrvSendData(0x00, LCD_DISP_MODE_DAT);             //Frame Inversion
}

void lcdDrvGrayTest()
{
  uint8_t i = 0, j = 0;
  lcdDrvSetMode();
  lcdDrvSendData(0x5c, LCD_SEND_MODE_CMD);

  for (i = 0; i < LCD_HEIGHT_PAGE_MAX / 4; i++)
  {
    for (j = 0; j < LCD_WIDTH_X; j++)
    {
      lcdDrvSendData(0xff, LCD_DISP_MODE_DAT);
      delay_ms(1);
    }
  }

  for (i = 0; i < LCD_HEIGHT_PAGE_MAX / 4; i++)
  {
    for (j = 0; j < LCD_WIDTH_X; j++)
    {
      lcdDrvSendData(0xaa, LCD_DISP_MODE_DAT);
      delay_ms(1);
    }
  }

  for (i = 0; i < LCD_HEIGHT_PAGE_MAX / 4; i++)
  {
    for (j = 0; j < LCD_WIDTH_X; j++)
    {
      lcdDrvSendData(0x55, LCD_DISP_MODE_DAT);
      delay_ms(1);
    }
  }

  for (i = 0; i < LCD_HEIGHT_PAGE_MAX / 4; i++)
  {
    for (j = 0; j < LCD_WIDTH_X; j++)
    {
      lcdDrvSendData(0x00, LCD_DISP_MODE_DAT);
      delay_ms(1);
    }
  }
}

void lcdDrvInit(void)
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

  //lcdDrvSendData(0x30, LCD_SEND_MODE_CMD); // Extension Command 1
  //lcdDrvSendData(0x6E, LCD_SEND_MODE_CMD); //Enable Master
  lcdDrvSendData(0x31, LCD_SEND_MODE_CMD); // Extension Command 2
  lcdDrvSendData(0xD7, LCD_SEND_MODE_CMD); // Disable Auto Read
  lcdDrvSendData(0x9F, LCD_DISP_MODE_DAT);
  //lcdDrvSendData(0xE0, LCD_SEND_MODE_CMD); // Enable OTP Read
  //lcdDrvSendData(0x00, LCD_DISP_MODE_DAT);
  delay_ms(10);
  //lcdDrvSendData(0xE3, LCD_SEND_MODE_CMD); // OTP Up-Load
  delay_ms(20);
  //lcdDrvSendData(0xE1, LCD_SEND_MODE_CMD); // OTP Control Out
  lcdDrvSendData(0x30, LCD_SEND_MODE_CMD); // Extension Command 1
  lcdDrvSendData(0x94, LCD_SEND_MODE_CMD); // Sleep Out
  lcdDrvSendData(0xAE, LCD_SEND_MODE_CMD); // Display OFF
  delay_ms(50);

  lcdDrvSendData(0x20, LCD_SEND_MODE_CMD); // Power Control
  lcdDrvSendData(0x0B, LCD_DISP_MODE_DAT); // VB, VR, VF All ON

  lcdDrvSendData(0x81, LCD_SEND_MODE_CMD); // Set Vop = 16V
  lcdDrvSendData(0x28, LCD_DISP_MODE_DAT); //对比度设置,这里要根据自己的屏调整,不然可能会不显示
  lcdDrvSendData(0x03, LCD_DISP_MODE_DAT);

  lcdDrvSendData(0x31, LCD_SEND_MODE_CMD); // Extension Command 2

  lcdDrvSendData(0x20, LCD_SEND_MODE_CMD); // Set Gray Scale Level
  lcdDrvSendData(0x01, LCD_DISP_MODE_DAT);
  lcdDrvSendData(0x03, LCD_DISP_MODE_DAT);
  lcdDrvSendData(0x05, LCD_DISP_MODE_DAT);
  lcdDrvSendData(0x07, LCD_DISP_MODE_DAT); //Light Gray Level Setting
  lcdDrvSendData(0x09, LCD_DISP_MODE_DAT); //Light Gray Level Setting
  lcdDrvSendData(0x0b, LCD_DISP_MODE_DAT); //Light Gray Level Setting
  lcdDrvSendData(0x0d, LCD_DISP_MODE_DAT);
  lcdDrvSendData(0x10, LCD_DISP_MODE_DAT);
  lcdDrvSendData(0x11, LCD_DISP_MODE_DAT); //Dark Gray Level Setting
  lcdDrvSendData(0x13, LCD_DISP_MODE_DAT);
  lcdDrvSendData(0x15, LCD_DISP_MODE_DAT);
  lcdDrvSendData(0x17, LCD_DISP_MODE_DAT); //Dark Gray Level Setting
  lcdDrvSendData(0x19, LCD_DISP_MODE_DAT); //Dark Gray Level Setting
  lcdDrvSendData(0x1b, LCD_DISP_MODE_DAT); //Dark Gray Level Setting
  lcdDrvSendData(0x1d, LCD_DISP_MODE_DAT);
  lcdDrvSendData(0x1f, LCD_DISP_MODE_DAT);

  lcdDrvSendData(0x32, LCD_SEND_MODE_CMD); // Analog Circuit Set
  lcdDrvSendData(0x00, LCD_DISP_MODE_DAT);
  lcdDrvSendData(0x01, LCD_DISP_MODE_DAT); // Booster Efficiency =Level 1
  lcdDrvSendData(0x02, LCD_DISP_MODE_DAT); //Bias=1/12

  lcdDrvSendData(0x51, LCD_SEND_MODE_CMD); // Booster Level x10
  lcdDrvSendData(0xFB, LCD_DISP_MODE_DAT);

  lcdDrvSendData(0x30, LCD_SEND_MODE_CMD); // Extension Command 1

  lcdDrvSendData(0xBC, LCD_SEND_MODE_CMD); // Data Scan Direction
  lcdDrvSendData(0x00, LCD_DISP_MODE_DAT);

  lcdDrvSendData(0x08, LCD_SEND_MODE_CMD); // Data Format Select, LSB is on bottom; D7->D0 (Default)
  //lcdDrvSendData(0x0C, LCD_SEND_MODE_CMD); // Data Format Select, LSB is on top; D0->D7

  lcdDrvSendData(0xA6, LCD_SEND_MODE_CMD); // Normal Display
  lcdDrvSendData(0x31, LCD_SEND_MODE_CMD); // Extension Command 2
  lcdDrvSendData(0x40, LCD_SEND_MODE_CMD); // Internal Power Supply

  lcdDrvSetMode();

  lcdDrvSendData(0x30, LCD_SEND_MODE_CMD); // Extension Command 1
  lcdDrvSendData(0xAF, LCD_SEND_MODE_CMD); // Display ON

  //lcdDrvGrayTest();
  //delay_ms(1000);
}

/*
 * lcdGdiUpdate:
 *	Copy our software version to the real display
 *********************************************************************************
 */
void lcdGdiUpdate(void)
{
  uint8_t x = 0, y = 0;
  lcdDrvSetMode();
  lcdDrvSendData(0x5C, LCD_SEND_MODE_CMD); // write data to lcd
  for (y = 0; y < LCD_HEIGHT_PAGE_MAX; y++)
  {
    for (x = 0; x < LCD_WIDTH_X; x++)
    {
      lcdDrvSendData(frameBuffer[y][x], LCD_DISP_MODE_DAT);
    }
  }
}

/*
 * lcdGdiSetOrientation:
 *	Set the display orientation:
 *	0: Normal, the display is portrait mode, 0,0 is top left
 *	1: Mirror x
 *	2: Mirror y
 *	3: Mirror x and y
 *********************************************************************************
 */
void lcdGdiSetOrientation(int32_t orientation)
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
 * lcdGdiGetScreenSize:
 *	Return the max X & Y screen sizes. Needs to be called again, if you 
 *	change screen orientation.
 *********************************************************************************
 */
void lcdGdiGetScreenSize(int32_t *x, int32_t *y)
{
  *x = LCD_WIDTH_X;
  *y = LCD_HEIGHT_Y;
}

/*
 *********************************************************************************
 * Standard Graphical Functions
 *********************************************************************************
 */

/*
 * lcdGdiPoint:
 *	Plot a pixel.
 *********************************************************************************
 */
void lcdGdiPoint(int32_t x, int32_t y, int32_t colour)
{
  uint8_t bitmsk = 0;
  uint8_t bitmv = 0;
  uint8_t colour_t = (uint8_t)colour;
  uint8_t frameBuffer_t = 0;
  if (mirrorX)
    x = (LCD_WIDTH_X - x - 1);

  if (mirrorY)
    y = (LCD_HEIGHT_Y - y - 1);

  lastX = x;
  lastY = y;

  if ((x < 0) || (x >= LCD_WIDTH_X) || (y < 0) || (y >= LCD_HEIGHT_Y))
    return;

  colour_t = colour_t & LCD_COLOUT_BIT_MSK;
  bitmsk = (y % LCD_HEIGHT_PAGE_ROW);
  bitmv = ((LCD_HEIGHT_PAGE_ROW - bitmsk - 1) * LCD_COLOUR_BIT);

  frameBuffer_t = frameBuffer[y / LCD_HEIGHT_PAGE_ROW][x];

  frameBuffer_t = (frameBuffer_t & ((uint8_t)(~(LCD_COLOUT_BIT_MSK << bitmv))));

  frameBuffer_t = (frameBuffer_t | ((uint8_t)(colour_t << bitmv)));

  frameBuffer[y / LCD_HEIGHT_PAGE_ROW][x] = frameBuffer_t;
}

/*
 * lcdGdiPoint:
 *	Plot a pixel.
 *********************************************************************************
 */
int32_t lcdGdiGetpoint(int32_t x, int32_t y)
{
  uint8_t bitmsk = 0;
  uint8_t bitmv = 0;
  uint8_t frameBuffer_t = 0;
  uint8_t colour = 0;
  if (mirrorX)
    x = (LCD_WIDTH_X - x - 1);

  if (mirrorY)
    y = (LCD_HEIGHT_Y - y - 1);

  if ((x < 0) || (x >= LCD_WIDTH_X) || (y < 0) || (y >= LCD_HEIGHT_Y))
    return -1;

  bitmsk = (y % LCD_HEIGHT_PAGE_ROW);
  bitmv = ((LCD_HEIGHT_PAGE_ROW - bitmsk - 1) * LCD_COLOUR_BIT);

  frameBuffer_t = frameBuffer[y / LCD_HEIGHT_PAGE_ROW][x];

  frameBuffer_t = ((uint8_t)(frameBuffer_t >> bitmv));
  colour = frameBuffer_t & LCD_COLOUT_BIT_MSK;

  return (int32_t)colour;
}

/*
 * lcdGdiLine: lcdGdiLineTo:
 *	Classic Bressenham Line code
 *********************************************************************************
 */
void lcdGdiLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t colour)
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
    lcdGdiPoint(x0, y0, colour);
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

void lcdGdiLineTo(int32_t x, int32_t y, int32_t colour)
{
  lcdGdiLine(lastX, lastY, x, y, colour);
}

/*
 * lcdGdiRectangle:
 *	A rectangle is a spoilt days fishing
 *********************************************************************************
 */
void lcdGdiRectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t colour, int32_t filled)
{
  int32_t x;
  if (filled)
  {
    if (x1 == x2)
    {
      lcdGdiLine(x1, y1, x2, y2, colour);
    }
    else if (x1 < x2)
    {
      for (x = x1; x <= x2; ++x)
      {
        lcdGdiLine(x, y1, x, y2, colour);
      }
    }
    else
    {
      for (x = x2; x <= x1; ++x)
      {
        lcdGdiLine(x, y1, x, y2, colour);
      }
    }
  }
  else
  {
    lcdGdiLine(x1, y1, x2, y1, colour);
    lcdGdiLineTo(x2, y2, colour);
    lcdGdiLineTo(x1, y2, colour);
    lcdGdiLineTo(x1, y1, colour);
  }
}

/*
 * lcdGdiCircle:
 *      This is the midpoint32 circle algorithm.
 *********************************************************************************
 */
void lcdGdiCircle(int32_t x, int32_t y, int32_t r, int32_t colour, int32_t filled)
{
  int32_t ddF_x = 1;
  int32_t ddF_y = -2 * r;

  int32_t f = 1 - r;
  int32_t x1 = 0;
  int32_t y1 = r;

  if (filled)
  {
    lcdGdiLine(x, y + r, x, y - r, colour);
    lcdGdiLine(x + r, y, x - r, y, colour);
  }
  else
  {
    lcdGdiPoint(x, y + r, colour);
    lcdGdiPoint(x, y - r, colour);
    lcdGdiPoint(x + r, y, colour);
    lcdGdiPoint(x - r, y, colour);
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
      lcdGdiLine(x + x1, y + y1, x - x1, y + y1, colour);
      lcdGdiLine(x + x1, y - y1, x - x1, y - y1, colour);
      lcdGdiLine(x + y1, y + x1, x - y1, y + x1, colour);
      lcdGdiLine(x + y1, y - x1, x - y1, y - x1, colour);
    }
    else
    {
      lcdGdiPoint(x + x1, y + y1, colour);
      lcdGdiPoint(x - x1, y + y1, colour);
      lcdGdiPoint(x + x1, y - y1, colour);
      lcdGdiPoint(x - x1, y - y1, colour);
      lcdGdiPoint(x + y1, y + x1, colour);
      lcdGdiPoint(x - y1, y + x1, colour);
      lcdGdiPoint(x + y1, y - x1, colour);
      lcdGdiPoint(x - y1, y - x1, colour);
    }
  }
}

/*
 * lcdGdiEllipse:
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
static void plot4ellipsePoints(int32_t cx, int32_t cy, int32_t x, int32_t y, int32_t colour, int32_t filled)
{
  if (filled)
  {
    lcdGdiLine(cx + x, cy + y, cx - x, cy + y, colour);
    lcdGdiLine(cx - x, cy - y, cx + x, cy - y, colour);
  }
  else
  {
    lcdGdiPoint(cx + x, cy + y, colour);
    lcdGdiPoint(cx - x, cy + y, colour);
    lcdGdiPoint(cx - x, cy - y, colour);
    lcdGdiPoint(cx + x, cy - y, colour);
  }
}

void lcdGdiEllipse(int32_t cx, int32_t cy, int32_t xRadius, int32_t yRadius, int32_t colour, int32_t filled)
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
    plot4ellipsePoints(cx, cy, x, y, colour, filled);
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
    plot4ellipsePoints(cx, cy, x, y, colour, filled);
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
 * lcdGdiPutchar:
 *	Print a single character to the screen
 *********************************************************************************
 */
void lcdGdiPutchar(int32_t x, int32_t y, int32_t c, int32_t bgCol, int32_t fgCol)
{
  int32_t y1, y2;

  uint8_t line;
  uint8_t *fontPtr;

  // Can't print if we're offscreen

  if ((x < 0) || (x > (LCD_WIDTH_X - fontWidth)) || (y < 0) || (y > (LCD_HEIGHT_Y - fontHeight)))
    return;

  fontPtr = font + c * fontHeight;

  for (y1 = 0; y1 < fontHeight; y1++)
  {
    y2 = y + y1;
    line = *fontPtr++;
    lcdGdiPoint(x + 0, y2, (line & 0x80) == 0 ? bgCol : fgCol);
    lcdGdiPoint(x + 1, y2, (line & 0x40) == 0 ? bgCol : fgCol);
    lcdGdiPoint(x + 2, y2, (line & 0x20) == 0 ? bgCol : fgCol);
    lcdGdiPoint(x + 3, y2, (line & 0x10) == 0 ? bgCol : fgCol);
    lcdGdiPoint(x + 4, y2, (line & 0x08) == 0 ? bgCol : fgCol);
    lcdGdiPoint(x + 5, y2, (line & 0x04) == 0 ? bgCol : fgCol);
    lcdGdiPoint(x + 6, y2, (line & 0x02) == 0 ? bgCol : fgCol);
    lcdGdiPoint(x + 7, y2, (line & 0x01) == 0 ? bgCol : fgCol);
  }
}

/*
 * lcdGdiPuts:
 *	Send a string to the display. Obeys \n and \r formatting
 *********************************************************************************
 */
void lcdGdiPuts(int32_t x, int32_t y, const char *str, int32_t bgCol, int32_t fgCol)
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

    lcdGdiPutchar(mx, my, c, bgCol, fgCol);

    mx += fontWidth;
    //if (mx >= (LCD_WIDTH_X - fontWidth))
    if (mx > (LCD_WIDTH_X - fontWidth))
    {
      mx = 0;
      my += fontHeight;
    }
  }
}

/*
 * lcdGdiPutnum:
 *	Send a number to the display. 
 *********************************************************************************
 */
void lcdGdiPutnum(int32_t x, int32_t y, int32_t num, int32_t bgCol, int32_t fgCol)
{
  int8_t numString[50] = {0};
  if (sprintf(numString, "%d", num) < 0)
    return;
  lcdGdiPuts(x, y, numString, bgCol, fgCol);
}

/*
 * lcdGdiPutbmp:
 *	Send a picture to the display. 
 *********************************************************************************
 */
void lcdGdiPutbmp(int32_t x0, int32_t y0, int32_t with, int32_t height, uint8_t *bmp, int32_t colour)
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

/*
 * lcdGdiPutbmpspeed:
 *	Send a picture to the display. 
 *********************************************************************************
 */
void lcdGdiPutbmpspeed(int32_t x0, int32_t y0, int32_t with, int32_t height, uint8_t *bmp, int32_t colour)
{
#if 0
  int32_t x = 0, y = 0;
  uint8_t data = 0;

  for (y = y0; y < (height / 8); y++)
  {
    setPos(x0, y);
    for (x = x0; x < with; x++)
    {
      data = *bmp++;
      lcdDrvSendData(((colour != 0) ? data : ~data), LCD_DISP_MODE_DAT);
    }
  }
#endif
}

/*
 * lcdGdiOpen:
 *	Open hardware display.
 *********************************************************************************
 */
void lcdGdiOpen(void)
{
#if 0
  lcdDrvSendData(0X8D, LCD_SEND_MODE_CMD); //SET DCDC
  lcdDrvSendData(0X14, LCD_SEND_MODE_CMD); //DCDC ON
  lcdDrvSendData(0XAF, LCD_SEND_MODE_CMD); //DISPLAY ON
#endif
}

/*
 * lcdGdiCloase:
 *	Cloase the hardware display.
 *********************************************************************************
 */
void lcdGdiCloase(void)
{
#if 0
  lcdDrvSendData(0X8D, LCD_SEND_MODE_CMD); //SET DCDC
  lcdDrvSendData(0X10, LCD_SEND_MODE_CMD); //DCDC OFF
  lcdDrvSendData(0XAE, LCD_SEND_MODE_CMD); //DISPLAY OFF
#endif
}

/*
 * lcdGdiHardwareClear:
 *	Clear the hardware display.
 *********************************************************************************
 */
void lcdGdiHardwareClear(void)
{
#if 0
  int32_t i, n;
  for (i = 0; i < 8; i++)
  {
    lcdDrvSendData(0xb0 + i, LCD_SEND_MODE_CMD);
    lcdDrvSendData(0x02, LCD_SEND_MODE_CMD);
    lcdDrvSendData(0x10, LCD_SEND_MODE_CMD);
    for (n = 0; n < 128; n++)
    {
      lcdDrvSendData(0, LCD_DISP_MODE_DAT);
    }
  }
#endif
}

/*
 * lcdGdiClear:
 *	Clear the display to the given colour.
 *********************************************************************************
 */

void lcdGdiClear(int32_t colour)
{
  int32_t x = 0, y = 0;
  int32_t col = 0;

  if (colour)
    col = 0xff;
  else
    col = 0x00;

  for (y = 0; y < LCD_HEIGHT_PAGE_MAX; y++)
  {
    for (x = 0; x < LCD_WIDTH_X; x++)
    {
      frameBuffer[y][x] = col;
    }
  }
}

/*
 * lcdGdiSetup:
 *	Initialise the display and GPIO.
 *********************************************************************************
 */
int32_t lcdGdiSetup(void)
{
  lcdDrvInit();

  lcdGdiOpen();
  lcdGdiSetOrientation(0);
  lcdGdiClear(LCD_COLOUR_WHITE);
  lcdGdiHardwareClear();
  lcdGdiUpdate();

  return 0;
}
