/*
 * lcdGdi.h:
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
#ifndef __LCD192X96_H_
#define __LCD192X96_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// Size
#define LCD_WIDTH_X 192
#define LCD_HEIGHT_Y 96
#define LCD_COLOUR_BIT      (2)
#define LCD_COLOUT_BIT_MSK  (0x03)
#define LCD_HEIGHT_PAGE_ROW (8/LCD_COLOUR_BIT)
#define LCD_HEIGHT_PAGE_MAX (LCD_HEIGHT_Y/LCD_HEIGHT_PAGE_ROW)

typedef enum lcd_colour_e
{
    LCD_COLOUR_WHITE = 0,
    LCD_COLOUR_LIGHT_GREY,
    LCD_COLOUR_DARK_GREY,
    LCD_COLOUR_BLACK,
    LCD_COLOUR_MAX,
} lcd_colour_t;

extern void lcdGdiGetScreenSize(int32_t *x, int32_t *y);
extern void lcdGdiSetOrientation(int32_t orientation);
extern void lcdGdiPoint(int32_t x, int32_t y, int32_t colour);
extern int32_t lcdGdiGetpoint(int32_t x, int32_t y);
extern void lcdGdiLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t colour);
extern void lcdGdiLineTo(int32_t x, int32_t y, int32_t colour);
extern void lcdGdiRectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t colour, int32_t filled);
extern void lcdGdiCircle(int32_t x, int32_t y, int32_t r, int32_t colour, int32_t filled);
extern void lcdGdiEllipse(int32_t cx, int32_t cy, int32_t xRadius, int32_t yRadius, int32_t colour, int32_t filled);
extern void lcdGdiPutchar(int32_t x, int32_t y, int32_t c, int32_t bgCol, int32_t fgCol);
extern void lcdGdiPuts(int32_t x, int32_t y, const char *str, int32_t bgCol, int32_t fgCol);
extern void lcdGdiPutnum(int32_t x, int32_t y, int32_t num, int32_t bgCol, int32_t fgCol);
extern void lcdGdiPutbmp(int32_t x0, int32_t y0, int32_t with, int32_t height, uint8_t *bmp, int32_t colour);
extern void lcdGdiPutbmpspeed(int32_t x0, int32_t y0, int32_t with, int32_t height, uint8_t *bmp, int32_t colour);
extern void lcdGdiUpdate(void);
extern void lcdGdiOpen(void);
extern void lcdGdiCloase(void);
extern void lcdGdiHardwareClear(void);
extern void lcdGdiClear(int32_t colour);

extern int32_t lcdGdiSetup(void);

#endif