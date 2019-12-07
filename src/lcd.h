#ifndef _LCD_SIMULATOR_H_
#define _LCD_SIMULATOR_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "type.h"

#include "font.h"

#include "lcd192x96.h"

#define LCD_MAX_X (LCD_DRV_MAX_X)
#define LCD_MAX_Y (LCD_DRV_MAX_Y)

typedef enum lcd_color_e
{
    LCD_COL_WHITE = 0,
    LCD_COL_LIGHT_GRAY,
    LCD_COL_DARK_GRAY,
    LCD_COL_BLACK,
    LCD_COL_MAX,
    LCD_COL_TRUE = LCD_COL_BLACK,
    LCD_COL_FALSE = LCD_COL_WHITE,
} lcd_color_t;

#define LCD_SIMULATOR_NAME "ST75256_192x96_2bit"

#define LCD_DEFAULT_FONT FONT_17X24

void delay_xms(uint32_t ms);

/*****************************************************************************
函 数 名  : led_set_mirror
功能描述  : 设置屏幕镜像显示
输入参数  : mirror(0-不镜像, 1-只镜像x, 2-只镜像y, 3-镜像x和y)
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void lcd_set_mirror(uint8_t mirror);

/*****************************************************************************
函 数 名  : led_init
功能描述  : max7219初始化
输入参数  : void
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
extern void lcd_init(void);

/*****************************************************************************
函 数 名  : led_update
功能描述  : 把显存中的数据写入硬件(max7219)中
输入参数  : void
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
extern void lcd_update(void);

/*****************************************************************************
函 数 名  : led_clear
功能描述  : 用制定颜色填充(刷新)显存
输入参数  : uchar dat  指定颜色,0:不显示(黑色),非0:显示(白色)
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
extern void lcd_clear(int32_t dat);

/*****************************************************************************
函 数 名  : led_set_point
功能描述  : 设置显存中指定坐标点的颜色
输入参数  : uchar x    显存中指定点的x坐标[0-32)
uchar y    显存中指定点的y坐标[0-16)
uchar dat  坐标点的颜色,0:不显示(黑色),非0:显示(白色)
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
extern void lcd_set_point(int32_t x, int32_t y, int32_t dat);

/*****************************************************************************
函 数 名  : led_get_point
功能描述  : 获取显存中指定坐标点的颜色
输入参数  : uchar x    显存中指定点的x坐标[0-32)
uchar y    显存中指定点的y坐标[0-16)
输出参数  : 无
返 回 值  : 坐标点的颜色,0:不显示(黑色),非0:显示(白色)
*****************************************************************************/
extern int32_t lcd_get_point(int32_t x, int32_t y);

/*****************************************************************************
函 数 名  : led_reverse_point
功能描述  : 把显存中指定坐标点的颜色反转
输入参数  : int32 x    显存中指定点的x坐标[0-32)
int32 y    显存中指定点的y坐标[0-16)
输出参数  : 无
返 回 值  : 坐标点的颜色,0:不显示(黑色),非0:显示(白色)
*****************************************************************************/
extern int32_t lcd_reverse_point(int32_t x, int32_t y);

/*****************************************************************************
函 数 名  : led_set_font
功能描述  : 设置字体
输入参数  :
font_name_t font 要设置的字体名
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
extern void lcd_set_font(font_name_t font);

/*****************************************************************************
函 数 名  : led_blk_cpy2mem_s
功能描述  : 把一个矩形块复制到显存中
输入参数  :
uchar *dat   矩形块的地址
int32 x0     指定该矩形块左上角在显存中的位置x
int32 x1     指定该矩形块可视部分起始位置
int32 x2     指定该矩形块可视部分结束位置
int32 y0     指定该矩形块左上角在显存中的位置y
int32 bcolor 背景色
int32 fcolor 前景色
输出参数  : 无
返 回 值  : 0-成功,1-失败
*****************************************************************************/
extern int32_t lcd_blk_cpy2mem_s(uint8_t *dat, int32_t x0, int32_t x1, int32_t x2, int32_t y0, int32_t bcolor, int32_t fcolor);

/*****************************************************************************
函 数 名  : led_blk_cpy2mem
功能描述  : 把一个矩形块复制到显存中
输入参数  :
uchar *dat   矩形块的地址
int32 x     指定该矩形块左上角在显存中的位置x
int32 y     指定该矩形块左上角在显存中的位置y
int32 bcolor 背景色
int32 fcolor 前景色
输出参数  : 无
返 回 值  : 0-成功,1-失败
*****************************************************************************/
extern int32_t lcd_blk_cpy2mem(uint8_t *dat, int32_t x, int32_t y, int32_t bcolor, int32_t fcolor);

/*****************************************************************************
函 数 名  : led_putc
功能描述  : 显示一个字符图像(只写入显存,不更新硬件)
输入参数  :
int32 x_pos  要显示的字符图像的x坐标
int32 y_pos  要显示的字符图像的y坐标
char chr     需要显示的字符的ascii码
int32 bcolor 背景色
int32 fcolor 前景色
输出参数  : 无
返 回 值  : 0-成功,1-失败
*****************************************************************************/
extern int32_t lcd_putc(int32_t x_pos, int32_t y_pos, int8_t chr, int32_t bcolor, int32_t fcolor);

/*****************************************************************************
函 数 名  : led_putc_s
功能描述  : 显示一个字符图像(只写入显存,不更新硬件)
输入参数  :
int32 x_pos     要显示的字符图像的x坐标
int32 x_disp0   指定可视部分起始位置
int32 x_disp1   指定可视部分结束位置
int32 y_pos     要显示的字符图像的y坐标
char chr        需要显示的字符的ascii码
int32 bcolor    背景色
int32 fcolor    前景色
输出参数  : 无
返 回 值  : 0-成功,1-失败
*****************************************************************************/
extern int32_t lcd_putc_s(int32_t x_pos, int32_t x_disp0, int32_t x_disp1, int32_t y_pos, int8_t chr, int32_t bcolor, int32_t fcolor);

/*****************************************************************************
函 数 名  : led_puts
功能描述  : 显示一个字符串图像(只写入显存,不更新硬件)
输入参数  :
int32 x_pos     要显示的字符串的x坐标
int32 y_pos     要显示的字符串的y坐标
char *str       需要显示的字符串
int32 bcolor    背景色
int32 fcolor    前景色
输出参数  : 无
返 回 值  : 0-成功,1-失败
*****************************************************************************/
extern int32_t lcd_puts(int32_t x_pos, int32_t y_pos, int8_t *str, int32_t bcolor, int32_t fcolor);

/*****************************************************************************
函 数 名  : led_puts_s
功能描述  : 显示一个字符串图像(只写入显存,不更新硬件)
输入参数  :
int32 x_pos     要显示的字符图像的x坐标
int32 x_disp0   指定可视部分起始位置
int32 x_disp1   指定可视部分结束位置
int32 y_pos     要显示的字符串的y坐标
char *str       需要显示的字符串
int32 bcolor    背景色
int32 fcolor    前景色
输出参数  : 无
返 回 值  : 0-成功,1-失败
*****************************************************************************/
extern int32_t lcd_puts_s(int32_t x_pos, int32_t x_disp0, int32_t x_disp1, int32_t y_pos, int8_t *str, int32_t bcolor, int32_t fcolor);

/*****************************************************************************
函 数 名  : led_scroll_puts
功能描述  : 滚动显示一个字符串图像(只写入显存,不更新硬件)
输入参数  :
int32 x_pos     要显示的字符图像的x坐标
int32 x_disp0   指定可视部分起始位置
int32 x_disp1   指定可视部分结束位置
int32 y_pos     要显示的字符串的y坐标
char *str       需要滚动显示的字符串
int32 delay     滚动速度
int32 bcolor    背景色
int32 fcolor    前景色
dir_t dir       滚动方向
输出参数  : 无
返 回 值  : 0-成功,1-失败
*****************************************************************************/
extern int32_t lcd_scroll_puts(int32_t x_pos, int32_t x_disp0, int32_t x_disp1, int32_t y_pos, int8_t *str, int32_t bcolor, int32_t fcolor, dir_t dir, int32_t delay);

/*****************************************************************************
函 数 名  : led_scroll_puts_s
功能描述  : 自动滚动显示一个字符串图像(只写入显存,不更新硬件)
输入参数  :
int32 x_pos     要显示的字符图像的x坐标
int32 x_disp0   指定可视部分起始位置
int32 x_disp1   指定可视部分结束位置
int32 y_pos     要显示的字符串的y坐标
char *str       需要滚动显示的字符串
int32 delay     滚动速度
int32 bcolor    背景色
int32 fcolor    前景色
输出参数  : 无
返 回 值  : 0-成功,1-失败
*****************************************************************************/
extern int32_t lcd_scroll_puts_s(int32_t x_disp0, int32_t x_disp1, int32_t y_pos, int8_t *str, int32_t bcolor, int32_t fcolor, int32_t delay);

/*****************************************************************************
函 数 名  : led_text_s
功能描述  : 自动显示一串文本,支持换行回车,支持自动换行(只写入显存,不更新硬件)
输入参数  :
int32 x_pos     要显示的字符串的x坐标
int32 x_disp0   指定可视部分起始位置
int32 x_disp1   指定可视部分结束位置
int32 y_pos     要显示的字符串的y坐标
char *str       需要显示的字符串
int32 bcolor    背景色
int32 fcolor    前景色
输出参数  : 无
返 回 值  : 0-成功,1-失败
*****************************************************************************/
extern int32_t lcd_text_s(int32_t x_disp0, int32_t x_disp1, int32_t y_pos, int8_t *str, int32_t bcolor, int32_t fcolor);

extern void lcd_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t colour);
extern void lcd_rectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t colour, int32_t filled);
extern void lcd_circle(int32_t x, int32_t y, int32_t r, int32_t colour, int32_t filled);
extern void lcd_ellipse(int32_t cx, int32_t cy, int32_t xRadius, int32_t yRadius, int32_t colour, int32_t filled);

extern int32_t lcd_putbmp(int32_t x0, int32_t y0, int32_t width, int32_t height, uint8 *bmp, int32_t colour);
extern int32_t lcd_putbmpspeed(int32_t x0, int32_t y0, int32_t width, int32_t height, uint8 *bmp, int32_t colour);

#endif // !_LCD_SIMULATOR_H_
