#ifndef _LCD_BMP_H_
#define _LCD_BMP_H_

#include "type.h"

typedef enum lcd_control_e
{
    LCD_CTRL_STOP = 0,
    LCD_CTRL_START,
    LCD_CTRL_RUN,
} lcd_control_t;

extern int32_t bmp_init(int8_t *filename);
extern int32_t bmp_dinit(void);
extern int32_t bmp_start(void);
extern int32_t bmp_show(int32_t x0, int32_t y0, int32_t colour);

#endif