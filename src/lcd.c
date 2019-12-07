#include "lcd.h"

#define _memset_ memset
#define _memcpy_ memcpy
#define _memcmp_ memcmp
#define _strlen_ strlen
#define _strcmp_ strcmp
//#define delay_xms usleep

static uint8_t mirrorX = 0;
static uint8_t mirrorY = 0;

static int32_t LCD_DISP_COLOUR[LCD_COL_MAX] =
{
    LCD_DRV_COLOUR_WHITE,
    LCD_DRV_COLOUR_LIGHT_GREY,
    LCD_DRV_COLOUR_DARK_GREY,
    LCD_DRV_COLOUR_BLACK
};

font_t *LCD_DISP_FONT = NULL;

void delay_xms(uint32_t ms)
{
    delay(ms);
}

/*****************************************************************************
函 数 名  : led_set_mirror
功能描述  : 设置屏幕镜像显示
输入参数  : mirror(0-不镜像, 1-只镜像x, 2-只镜像y, 3-镜像x和y)
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void lcd_set_mirror(uint8_t mirror)
{
    switch (mirror)
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

/*****************************************************************************
函 数 名  : led_init
功能描述  : max7219初始化
输入参数  : void
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void lcd_init(void)
{
    lcd_set_mirror(0);
    lcd_set_font(LCD_DEFAULT_FONT);
    lcd_drv_init();
}

/*****************************************************************************
函 数 名  : led_set_point
功能描述  : 设置显存中指定坐标点的颜色
输入参数  : uchar x    显存中指定点的x坐标[0-32)
uchar y    显存中指定点的y坐标[0-16)
uchar dat  坐标点的颜色,0:不显示(黑色),非0:显示(白色)
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void lcd_set_point(int32_t x, int32_t y, int32_t dat)
{
    lcd_drv_set_point(x, y, dat);
}

/*****************************************************************************
函 数 名  : led_get_point
功能描述  : 获取显存中指定坐标点的颜色
输入参数  : uchar x    显存中指定点的x坐标[0-32)
uchar y    显存中指定点的y坐标[0-16)
输出参数  : 无
返 回 值  : 坐标点的颜色,0:不显示(黑色),非0:显示(白色)
*****************************************************************************/
int32_t lcd_get_point(int32_t x, int32_t y)
{
    return lcd_drv_get_point(x, y);
}

/*****************************************************************************
函 数 名  : led_reverse_point
功能描述  : 把显存中指定坐标点的颜色反转
输入参数  : int32 x    显存中指定点的x坐标[0-32)
int32 y    显存中指定点的y坐标[0-16)
输出参数  : 无
返 回 值  : 坐标点的颜色,0:不显示(黑色),非0:显示(白色)
*****************************************************************************/
int32_t lcd_reverse_point(int32_t x, int32_t y)
{
    int32_t ret = 0;

    ret = lcd_get_point(x, y);
    ret = LCD_COL_MAX - ret - 1;
    lcd_set_point(x, y, ret);

    return ret;
}

/*****************************************************************************
函 数 名  : led_update
功能描述  : 把显存中的数据写入硬件(max7219)中
输入参数  : void
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void lcd_update(void)
{
    //lcd_drv_clear(LCD_DRV_COLOUR_WHITE);
    lcd_drv_update();
}

/*****************************************************************************
函 数 名  : led_clear
功能描述  : 用制定颜色填充(刷新)显存
输入参数  : uchar dat  指定颜色,0:不显示(黑色),非0:显示(白色)
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void lcd_clear(int32_t dat)
{
    lcd_drv_clear(dat);
}

/*****************************************************************************
函 数 名  : led_set_font
功能描述  : 设置字体
输入参数  :
font_name_t font 要设置的字体名
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void lcd_set_font(font_name_t font)
{
    font = (font >= FONT_MAX) ? (FONT_MAX - 1) : ((font <= FONT_MIN) ? FONT_MIN : font);
    LCD_DISP_FONT = font_get(font);
}

/*****************************************************************************
函 数 名  : led_get_font
功能描述  : 获取字体
输入参数  : 无
输出参数  : 无
返 回 值  : 当前字体信息
*****************************************************************************/
font_t *lcd_get_font(void)
{
    if (LCD_DISP_FONT != NULL)
    {
        return LCD_DISP_FONT;
    }
    else
    {
        return font_get(FONT_MIN);
    }
}

/*****************************************************************************
函 数 名  : _check_invalid_char_
功能描述  : 检查字符是否有效(是否被字库支持)
输入参数  : char chr  被检查的字符
输出参数  : 无
返 回 值  : 该字符在字库中的索引(偏移量)
*****************************************************************************/
int32_t _check_invalid_char_(int8_t chr)
{
    int32_t idx = 0;
    font_t *pfont = lcd_get_font();

    if ((chr >= pfont->cmin) && (chr <= pfont->cmax))
    {
        idx = chr - pfont->cmin;
    }
    else
    {
        idx = pfont->cmin;
    }

    return idx;
}

int32_t lcd_blk_cpy2mem_b(uint8_t *dat, int32_t x0, int32_t y0, int32_t x1, int32_t x2, int32_t width, int32_t height, int32_t bcolor, int32_t fcolor)
{
    int32_t i = 0, j = 0;
    int32_t k = 0, n = 0;
    uint8_t datb = 0x80;
    int32_t x_sz = 0, y_sz = 0;

    if ((x1 > x2) || (dat == NULL))
    {
        return ERROR;
    }

    //x0 = ((x0 >= LCD_MAX_X) ? (LCD_MAX_X - 1) : ((x0 < 0) ? 0 : x0));
    //y0 = ((y0 >= LCD_MAX_Y) ? (LCD_MAX_Y - 1) : ((y0 < 0) ? 0 : y0));

    x_sz = /*((width + x0) >= LCD_MAX_Y) ? LCD_MAX_Y - x0 : */ width;
    y_sz = /*((height + y0) >= LCD_MAX_Y) ? LCD_MAX_Y - y0 : */ height;

    n = -1;
    for (j = y0; j < (y0 + y_sz); j++)
    {
        k = -1;
        for (i = x0; i < (x0 + x_sz); i++)
        {
            if ((i - x0) % 8 == 0)
            {
                k++;
                n++;
            }

            if ((i < x1) || (i > x2) || (x1 > x2)) //裁剪
            {
                continue;
            }

            if ((*(dat + n)) & (datb >> ((i - x0) - k * 8)))
            {
                lcd_set_point(i, j, fcolor);
            }
            else
            {
                lcd_set_point(i, j, bcolor);
            }
        }
    }

    return OK;
}

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
int32_t lcd_blk_cpy2mem_s(uint8_t *dat, int32_t x0, int32_t x1, int32_t x2, int32_t y0, int32_t bcolor, int32_t fcolor)
{
#if 0
    int32_t i = 0, j = 0;
    int32_t k = 0, n = 0;
    uint8_t datb = 0x80;
    int32_t x_sz = 0, y_sz = 0;
    font_t *pfont = lcd_get_font();

    x_sz = pfont->width;
    y_sz = pfont->height;

    if ((x1 > x2) || (dat == NULL))
    {
        return ERROR;
    }

    n = -1;
    for (j = y0; j < (y0 + y_sz); j++)
    {
        k = -1;
        for (i = x0; i < (x0 + x_sz); i++)
        {
            if ((i - x0) % 8 == 0)
            {
                k++;
                n++;
            }

            if ((i < x1) || (i > x2) || (x1 > x2))//裁剪
            {
                continue;
            }

            if ((*(dat + n)) & (datb >> ((i - x0) - k * 8)))
            {
                lcd_set_point(i, j, fcolor);
            }
            else
            {
                lcd_set_point(i, j, bcolor);
            }
        }
    }

    return OK;
#else
    font_t *pfont = lcd_get_font();
    return lcd_blk_cpy2mem_b(dat, x0, y0, x1, x2, pfont->width, pfont->height, bcolor, fcolor);
#endif
}

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
int32_t lcd_blk_cpy2mem(uint8_t *dat, int32_t x, int32_t y, int32_t bcolor, int32_t fcolor)
{
    font_t *pfont = lcd_get_font();
    //return lcd_blk_cpy2mem_s(dat, x, x, x + pfont->width, y, bcolor, fcolor);
    return lcd_blk_cpy2mem_b(dat, x, y, x, x + pfont->width, pfont->width, pfont->height, bcolor, fcolor);
}

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
#if 0
int32_t lcd_putc(int32_t x_pos, int32_t y_pos, int8_t chr, int32_t bcolor, int32_t fcolor)
{
    int32_t idx = 0;
    uint8_t *dat = NULL;
    int32_t fontwbyte = 0;//点阵字体数据每行所占的字节数
    font_t *pfont = lcd_get_font();

    if ((x_pos >= LCD_MAX_X) || (x_pos <= 0 - (pfont->width)) || (y_pos >= LCD_MAX_Y) || (y_pos <= 0 - (pfont->height)))
    {
        return ERROR;
    }

    idx = _check_invalid_char_(chr);

    //计算点阵字体数据每行所占的字节数
    fontwbyte = ((pfont->width) % 8) ? ((pfont->width) / 8 + 1) : ((pfont->width) / 8);

    //计算指定字符的点阵数据指针(偏移)
    dat = (uint8_t *)((pfont->pdata) + (idx * fontwbyte * (pfont->height)));

    return lcd_blk_cpy2mem(dat, x_pos, y_pos, bcolor, fcolor);
}
#else
int32_t lcd_putc(int32_t x_pos, int32_t y_pos, int8_t chr, int32_t bcolor, int32_t fcolor)
{
    return lcd_putc_s(x_pos, 0, LCD_MAX_X, y_pos, chr, bcolor, fcolor);
}
#endif

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
int32_t lcd_putc_s(int32_t x_pos, int32_t x_disp0, int32_t x_disp1, int32_t y_pos, int8_t chr, int32_t bcolor, int32_t fcolor)
{
    int32_t idx = 0;
    uint8_t *dat = NULL;
    int32_t fontwbyte = 0; //点阵字体数据每行所占的字节数
    font_t *pfont = lcd_get_font();

    if ((x_pos >= LCD_MAX_X) || (x_pos <= 0 - (pfont->width)) || (y_pos >= LCD_MAX_Y) || (y_pos <= 0 - (pfont->height)))
    {
        return ERROR;
    }

    if (x_disp1 < x_disp0)
    {
        return ERROR;
    }

    idx = _check_invalid_char_(chr);

    //计算点阵字体数据每行所占的字节数
    fontwbyte = ((pfont->width) % 8) ? ((pfont->width) / 8 + 1) : ((pfont->width) / 8);

    //计算指定字符的点阵数据指针(偏移)
    dat = (uint8_t *)((pfont->pdata) + (idx * fontwbyte * (pfont->height)));

    return lcd_blk_cpy2mem_s(dat, x_pos, x_disp0, x_disp1, y_pos, bcolor, fcolor);
}

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
#if 0
int32_t lcd_puts(int32_t x_pos, int32_t y_pos, int8_t *str, int32_t bcolor, int32_t fcolor)
{
    int32_t xpos = x_pos, i = 0, slen = 0;
    font_t *pfont = lcd_get_font();

    if ((str == NULL) || (y_pos <= 0 - (pfont->height)) || (y_pos >= LCD_MAX_Y))
    {
        return ERROR;
    }

    slen = _strlen_(str);

    for (i = 0; i < slen; i++)
    {
        lcd_putc(xpos, y_pos, str[i], bcolor, fcolor);
        xpos += (pfont->width);
        if (xpos >= LCD_MAX_X)
        {
            break;
        }
    }

    return OK;
}
#else
int32_t lcd_puts(int32_t x_pos, int32_t y_pos, int8_t *str, int32_t bcolor, int32_t fcolor)
{
    return lcd_puts_s(x_pos, 0, LCD_MAX_X, y_pos, str, bcolor, fcolor);
}
#endif

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
int32_t lcd_puts_s(int32_t x_pos, int32_t x_disp0, int32_t x_disp1, int32_t y_pos, int8_t *str, int32_t bcolor, int32_t fcolor)
{
    int32_t xpos = x_pos, i = 0, slen = 0;
    int32_t xdisp0 = 0, xdisp1 = 0;
    font_t *pfont = lcd_get_font();

    if ((str == NULL) || (y_pos <= 0 - (pfont->height)) || (y_pos >= LCD_MAX_Y) || (x_disp1 < x_disp0))
    {
        return ERROR;
    }

    slen = _strlen_(str);

    xdisp0 = x_disp0 - x_pos;
    xdisp1 = x_disp1 - x_pos;

    for (i = 0; i < slen; i++)
    {
        if (i < (xdisp0 / (pfont->width)))
        {
            xpos += (pfont->width);
            continue;
        }
        if (i == (xdisp0 / (pfont->width)))
        {
            lcd_putc_s(xpos, x_disp0, xpos + pfont->width, y_pos, str[i], bcolor, fcolor);
            xpos += (pfont->width);
            continue;
        }
        if (i == (xdisp1 / (pfont->width)))
        {
            lcd_putc_s(xpos, xpos, x_disp1, y_pos, str[i], bcolor, fcolor);
            xpos += (pfont->width);
            continue;
        }
        if (i > (xdisp1 / (pfont->width)))
        {
            break;
        }
        if ((xpos >= LCD_MAX_X) || (xpos > x_disp1))
        {
            break;
        }
        lcd_putc(xpos, y_pos, str[i], bcolor, fcolor);
        xpos += (pfont->width);
    }

    return OK;
}

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
int32_t lcd_scroll_puts(int32_t x_pos, int32_t x_disp0, int32_t x_disp1, int32_t y_pos, int8_t *str, int32_t bcolor, int32_t fcolor, dir_t dir, int32_t delay)
{
    int32_t i = 0;
    int32_t pos = 0;
    int32_t slen = 0;
    slen = _strlen_(str);
    font_t *pfont = lcd_get_font();

    if ((str == NULL) || (y_pos <= 0 - (pfont->height)) || (y_pos >= LCD_MAX_Y) || (x_disp1 < x_disp0))
    {
        return ERROR;
    }

    switch (dir)
    {
    case DIR_LEFT:
        pos = 0 - slen * (pfont->width);
        for (i = x_pos; i > pos; i--)
        {
            if (((i - pos) < LCD_MAX_X) && ((i - pos) < x_disp1))
            { //最后一个字符的坐标没有越界
                break;
            }
            lcd_puts_s(i, x_disp0, x_disp1, y_pos, str, bcolor, fcolor);
            lcd_update();
            lcd_puts_s(i, x_disp0, x_disp1, y_pos, str, bcolor, bcolor);
            if (delay)
                delay_xms(delay);
        }
        break;
    case DIR_RIGHT:
        pos = LCD_MAX_X;
        for (i = x_pos; i < pos; i++)
        {
            if ((i > 0) && (i > x_disp0))
            { //第一个字符的坐标没有越界
                break;
            }
            lcd_puts_s(i, x_disp0, x_disp1, y_pos, str, bcolor, fcolor);
            lcd_update();
            lcd_puts_s(i, x_disp0, x_disp1, y_pos, str, bcolor, bcolor);
            if (delay)
                delay_xms(delay);
        }
        break;
    default:
        break;
    }

    return OK;
}

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
int32_t lcd_scroll_puts_s(int32_t x_disp0, int32_t x_disp1, int32_t y_pos, int8_t *str, int32_t bcolor, int32_t fcolor, int32_t delay)
{
    font_t *pfont = lcd_get_font();
    int32_t slen = _strlen_(str);
    int32_t x_pos = x_disp0;

    if ((str == NULL) || (y_pos <= 0 - (pfont->height)) || (y_pos >= LCD_MAX_Y) || (x_disp1 < x_disp0))
    {
        return ERROR;
    }

    if ((slen * (pfont->width) >= LCD_MAX_X) || (slen * (pfont->width) >= (x_disp1 - x_disp0)))
    {
        lcd_scroll_puts(x_pos, x_disp0, x_disp1, y_pos, str, bcolor, fcolor, DIR_LEFT, delay);
        x_pos = x_disp0 - (slen * (pfont->width) - (x_disp1 - x_disp0));
        lcd_scroll_puts(x_pos, x_disp0, x_disp1, y_pos, str, bcolor, fcolor, DIR_RIGHT, delay);
    }
    else
    {
        lcd_puts_s(x_pos, x_disp0, x_disp1, y_pos, str, bcolor, fcolor);
    }
    return OK;
}

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
int32_t lcd_text_s(int32_t x_disp0, int32_t x_disp1, int32_t y_pos, int8_t *str, int32_t bcolor, int32_t fcolor)
{
    int32_t i = 0, j = 0;
    int32_t slen = _strlen_(str);
    int32_t xpos = x_disp0;
    font_t *pfont = lcd_get_font();

    if ((str == NULL) || (y_pos <= 0 - (pfont->height)) || (y_pos >= LCD_MAX_Y) || (x_disp1 < x_disp0))
    {
        return ERROR;
    }

    for (i = 0; i < slen; i++)
    {
        if (str[i] == '\r')
        {
            xpos = x_disp0;
            continue;
        }

        if (str[i] == '\n')
        {
            y_pos += (pfont->height);
            continue;
        }

        if ((xpos >= LCD_MAX_X) || (xpos >= x_disp1))
        {
            xpos = x_disp0;
            y_pos += (pfont->height);
            if (y_pos >= LCD_MAX_Y)
            {
                break;
            }
        }

        lcd_putc_s(xpos, x_disp0, x_disp1, y_pos, str[i], bcolor, fcolor);
        xpos += (pfont->width);
    }

    return OK;
}

/*
* lcd_line: gdi_lineto:
*	Classic Bressenham Line code
*******************************************************************************
*/
void lcd_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t colour)
{
    int32_t dx, dy;
    int32_t sx, sy;
    int32_t err, e2;

    dx = abs(x1 - x0);
    dy = abs(y1 - y0);

    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;

    err = dx - dy;
    for (;;)
    {
        lcd_set_point(x0, y0, colour);
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

/*
* lcd_rectangle:
*	A rectangle is a spoilt days fishing
*******************************************************************************
*/
void lcd_rectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t colour, int32_t filled)
{
    int32_t x;
    if (filled)
    {
        if (x1 == x2)
        {
            lcd_line(x1, y1, x2, y2, colour);
        }
        else if (x1 < x2)
        {
            for (x = x1; x <= x2; ++x)
            {
                lcd_line(x, y1, x, y2, colour);
            }
        }
        else
        {
            for (x = x2; x <= x1; ++x)
            {
                lcd_line(x, y1, x, y2, colour);
            }
        }
    }
    else
    {
        lcd_line(x1, y1, x2, y1, colour);
        lcd_line(x2, y1, x2, y2, colour);
        lcd_line(x2, y2, x1, y2, colour);
        lcd_line(x1, y2, x1, y1, colour);
    }
}

/*
* lcd__circle:
*      This is the midpoint32 circle algorithm.
*******************************************************************************
*/
void lcd_circle(int32_t x, int32_t y, int32_t r, int32_t colour, int32_t filled)
{
    int32_t ddF_x = 1;
    int32_t ddF_y = -2 * r;

    int32_t f = 1 - r;
    int32_t x1 = 0;
    int32_t y1 = r;

    if (filled)
    {
        lcd_line(x, y + r, x, y - r, colour);
        lcd_line(x + r, y, x - r, y, colour);
    }
    else
    {
        lcd_set_point(x, y + r, colour);
        lcd_set_point(x, y - r, colour);
        lcd_set_point(x + r, y, colour);
        lcd_set_point(x - r, y, colour);
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
            lcd_line(x + x1, y + y1, x - x1, y + y1, colour);
            lcd_line(x + x1, y - y1, x - x1, y - y1, colour);
            lcd_line(x + y1, y + x1, x - y1, y + x1, colour);
            lcd_line(x + y1, y - x1, x - y1, y - x1, colour);
        }
        else
        {
            lcd_set_point(x + x1, y + y1, colour);
            lcd_set_point(x - x1, y + y1, colour);
            lcd_set_point(x + x1, y - y1, colour);
            lcd_set_point(x - x1, y - y1, colour);
            lcd_set_point(x + y1, y + x1, colour);
            lcd_set_point(x - y1, y + x1, colour);
            lcd_set_point(x + y1, y - x1, colour);
            lcd_set_point(x - y1, y - x1, colour);
        }
    }
}

/*
* gdi_ellipse:
*	Fast ellipse drawing algorithm by
*      John Kennedy
*	Mathematics Department
*	Santa Monica College
*	1900 Pico Blvd.
*	Santa Monica, CA 90405
*	jrkennedy6@gmail.com
*	-Confirned in email this algorithm is in the public domain -GH-
*******************************************************************************
*/
static void plot4ellipsePoints(int32_t cx, int32_t cy, int32_t x, int32_t y,
                               int32_t colour, int32_t filled)
{
    if (filled)
    {
        lcd_line(cx + x, cy + y, cx - x, cy + y, colour);
        lcd_line(cx - x, cy - y, cx + x, cy - y, colour);
    }
    else
    {
        lcd_set_point(cx + x, cy + y, colour);
        lcd_set_point(cx - x, cy + y, colour);
        lcd_set_point(cx - x, cy - y, colour);
        lcd_set_point(cx + x, cy - y, colour);
    }
}

void lcd_ellipse(int32_t cx, int32_t cy, int32_t xRadius, int32_t yRadius, int32_t colour, int32_t filled)
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
* lcd128x64putbmp:
*	Send a picture to the display.
*********************************************************************************
*/
int32_t lcd_putbmp(int32_t x0, int32_t y0, int32_t width, int32_t height, uint8 *bmp, int32_t colour)
{
    x0 = ((x0 >= LCD_MAX_X) ? (LCD_MAX_X - 1) : ((x0 < 0) ? 0 : x0));
    y0 = ((y0 >= LCD_MAX_Y) ? (LCD_MAX_Y - 1) : ((y0 < 0) ? 0 : y0));

    //width = ((width + x0) >= LCD_MAX_X) ? LCD_MAX_X - x0 : width;
    //height = ((height + y0) >= LCD_MAX_Y) ? LCD_MAX_Y - y0 : height;

    return lcd_blk_cpy2mem_b(bmp, x0, y0, x0, x0 + width, width, height, !colour, colour);
}

/*
* lcd128x64putbmpspeed:
*	Send a picture to the display.
*********************************************************************************
*/
int32_t lcd_putbmpspeed(int32_t x0, int32_t y0, int32_t width, int32_t height, uint8 *bmp, int32_t colour)
{
    lcd_drv_bmp_speed(x0, y0, width, height, bmp, colour);
    return OK;
}