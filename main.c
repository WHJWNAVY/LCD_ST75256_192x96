#include <stdio.h>
#include <wiringPi.h>
#include "lcd192x96.h"

int main(void)
{
    uint8_t x = 0, y = 0;
    uint8_t colour = 0;
    lcdGdiSetup();
    printf("oled init ok!\n");
    int i = 0;
    while (1)
    {
        lcdGdiSetOrientation(i++);
        if (i > 3)
            i = 0;
        printf("oled on!\n");
        colour = 0;
        for (y = 0; y < 64; y++)
        {
            for (x = 0; x < 128; x++)
            {
                lcdGdiPoint(x, y, colour);
                colour++;
                if (colour >= LCD_COLOUR_MAX)
                {
                    colour = 0;
                }
                //lcdGdiUpdate();
            }
        }
        lcdGdiUpdate();
        delay(1000);

        printf("oled off!\n");
        colour = 0;
        for (x = 0; x < 128; x++)
        {
            for (y = 0; y < 64; y++)
            {
                lcdGdiPoint(x, y, colour++);
                colour++;
                if (colour >= LCD_COLOUR_MAX)
                {
                    colour = 0;
                }
                //lcdGdiUpdate();
            }
        }
        lcdGdiUpdate();
        delay(1000);

        lcdGdiClear(0);
        lcdGdiLine(0, 0, 127, 63, LCD_COLOUR_BLACK);
        lcdGdiUpdate();
        printf("line ok!\n");
        delay(1500);

        lcdGdiClear(0);
        lcdGdiRectangle(0, 0, 63, 31, LCD_COLOUR_BLACK, 1);
        lcdGdiRectangle(63, 31, 127, 63, LCD_COLOUR_BLACK, 1);
        lcdGdiUpdate();
        printf("rectangle ok!\n");
        delay(1500);

        lcdGdiClear(0);
        lcdGdiCircle(63, 31, 30, LCD_COLOUR_BLACK, 1);
        lcdGdiUpdate();
        printf("circle ok!\n");
        delay(1500);

        lcdGdiClear(0);
        lcdGdiEllipse(63, 31, 40, 20, LCD_COLOUR_BLACK, 1);
        lcdGdiUpdate();
        printf("ellipse ok!\n");
        delay(1500);

        lcdGdiClear(0);
        lcdGdiPuts(0, 0, "Hello,\nWorld!\r\nWHJWNAVY\r\nQQ:805400349", 0, LCD_COLOUR_BLACK);
        lcdGdiUpdate();
        printf("puts ok!\n");
        delay(1500);

        lcdGdiClear(0);
    }
    printf("exit!\n");
}
