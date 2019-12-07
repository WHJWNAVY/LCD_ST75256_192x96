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
#include <wiringPi.h>

// Size
#define LCD_DRV_MAX_X (192)
#define LCD_DRV_MAX_Y (96)
#define LCD_DRV_COLOUR_BIT (2)
#define LCD_DRV_COLOUR_BIT_MSK (0x03)
#define LCD_DRV_PAGE_ROW (8 / LCD_DRV_COLOUR_BIT)
#define LCD_DRV_PAGE_MAX (LCD_DRV_MAX_Y / LCD_DRV_PAGE_ROW)

#define LCD_DRV_INCLUDE_GUILIB 0

typedef enum lcd_colour_e
{
    LCD_DRV_COLOUR_WHITE = 0,
    LCD_DRV_COLOUR_LIGHT_GREY,
    LCD_DRV_COLOUR_DARK_GREY,
    LCD_DRV_COLOUR_BLACK,
    LCD_DRV_COLOUR_MAX,
} lcd_drv_colour_t;

extern void lcd_drv_set_point(int32_t x, int32_t y, int32_t colour);
extern int32_t lcd_drv_get_point(int32_t x, int32_t y);
extern void lcd_drv_bmp_speed(int32_t x0, int32_t y0, int32_t width, int32_t height, uint8_t *bmp, int32_t colour);
extern void lcd_drv_update(void);
extern void lcd_drv_open(void);
extern void lcd_drv_close(void);
extern void lcd_drv_hw_clear(void);
extern void lcd_drv_clear(int32_t colour);
extern int32_t lcd_drv_init(void);

#if LCD_DRV_INCLUDE_GUILIB
extern void lcd_drv_get_screen_size(int32_t *x, int32_t *y);
extern void lcd_drv_set_orientation(int32_t orientation);
extern void lcd_drv_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t colour);
extern void lcd_drv_lineto(int32_t x, int32_t y, int32_t colour);
extern void lcd_drv_rectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t colour, int32_t filled);
extern void lcd_drv_circle(int32_t x, int32_t y, int32_t r, int32_t colour, int32_t filled);
extern void lcd_drv_ellipse(int32_t cx, int32_t cy, int32_t xRadius, int32_t yRadius, int32_t colour, int32_t filled);
extern void lcd_drv_putc(int32_t x, int32_t y, int32_t c, int32_t bgCol, int32_t fgCol);
extern void lcd_drv_puts(int32_t x, int32_t y, const char *str, int32_t bgCol, int32_t fgCol);
extern void lcd_drv_putn(int32_t x, int32_t y, int32_t num, int32_t bgCol, int32_t fgCol);
extern void lcd_drv_bmp(int32_t x0, int32_t y0, int32_t with, int32_t height, uint8_t *bmp, int32_t colour);
#endif

#endif