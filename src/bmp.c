#include "bmp.h"
#include "lcd.h"
#include <time.h>

typedef struct lcd_img_hdr_s
{
    int32_t flag;         //file flag, "LVIF"
    int32_t video_width;  //video width, 1920
    int32_t video_height; //video height, 1080
    int32_t lcd_width;    //lcd width, 128
    int32_t lcd_height;   //lcd height, 64
    int32_t video_fps;    //video fps, 25
    int32_t video_frame;  //video frame count, 875
    int32_t pixel_bit;    //每个像素点所占的bit
} lcd_img_hdr_t;

#define IMG_FRAME_DELAY (10) //16.75ms

#define IMG_HDR_FLAG (0x4649564c) //"LVIF"

#define IMG_HDR_LEN (sizeof(IMG_HDR))
#define IMG_PIXEL_BIT (8 / IMG_HDR.pixel_bit)
#if 1 //LED_SIMULATOR_HWTYPE_ST75256
#define IMG_FRAME_PAGE IMG_HDR.lcd_height
#define IMG_FRAME_NP IMG_HDR.lcd_width
#else // LED_SIMULATOR_HWTYPE_DEFAULT
#define IMG_FRAME_PAGE IMG_HDR.lcd_width
#define IMG_FRAME_NP IMG_HDR.lcd_height
#endif
#define IMG_FRAME_WIDTH (((IMG_FRAME_PAGE) / IMG_PIXEL_BIT) + ((((IMG_FRAME_PAGE) % IMG_PIXEL_BIT) == 0) ? 0 : 1))
#define IMG_FRAME_LEN (IMG_FRAME_WIDTH * (IMG_FRAME_NP))
#define IMG_FILE_LEN (IMG_FRAME_LEN * (IMG_HDR.video_frame))

FILE *BMP_FILE = NULL;
int8_t *BMP_FILE_BUFF = NULL;
lcd_img_hdr_t IMG_HDR = {0};

lcd_control_t LCD_CTRL_FLAG = 0;

void bmp_debug(void)
{
    DEBUG_LOG("video_width[%d]", IMG_HDR.video_width);
    DEBUG_LOG("video_height[%d]", IMG_HDR.video_height);
    DEBUG_LOG("lcd_width[%d]", IMG_HDR.lcd_width);
    DEBUG_LOG("lcd_height[%d]", IMG_HDR.lcd_height);
    DEBUG_LOG("video_fps[%d]", IMG_HDR.video_fps);
    DEBUG_LOG("video_frame[%d]", IMG_HDR.video_frame);
    DEBUG_LOG("pixel_bit[%d]", IMG_HDR.pixel_bit);
    DEBUG_LOG("video_fps[%d]", IMG_HDR.video_fps);

    DEBUG_LOG("IMG_FRAME_WIDTH[%d]", IMG_FRAME_WIDTH);
    DEBUG_LOG("IMG_FRAME_LEN[%d]", IMG_FRAME_LEN);
    DEBUG_LOG("IMG_FILE_LEN[%d]", IMG_FILE_LEN);
    DEBUG_LOG("IMG_FRAME_WIDTH[%d]", IMG_FRAME_WIDTH);
}

int32_t bmp_init(int8_t *filename)
{
    int32_t hdr_len = 0;
    int32_t rcnt = 0;

    if (filename == NULL)
    {
        return ERROR;
    }

    BMP_FILE = fopen(filename, "rb");
    if (BMP_FILE == NULL)
    {
        return ERROR;
    }

    hdr_len = fread(&IMG_HDR, 1, IMG_HDR_LEN, BMP_FILE);

    if (hdr_len != IMG_HDR_LEN)
    {
        fclose(BMP_FILE);
        return ERROR;
    }

    if (IMG_HDR.flag != IMG_HDR_FLAG)
    {
        fclose(BMP_FILE);
        return ERROR;
    }

    BMP_FILE_BUFF = malloc(IMG_FILE_LEN);
    if (BMP_FILE_BUFF == NULL)
    {
        return ERROR;
    }
    int32_t img_frame_width = IMG_FRAME_WIDTH;
    int32_t img_frame_size = IMG_FRAME_LEN;
    int32_t img_file_size = IMG_FILE_LEN;
    memset(BMP_FILE_BUFF, 0, IMG_FILE_LEN);
    fseek(BMP_FILE, IMG_HDR_LEN, SEEK_SET);
    rcnt = fread(BMP_FILE_BUFF, 1, IMG_FILE_LEN, BMP_FILE);

    if (rcnt != IMG_FILE_LEN)
    {
        return ERROR;
    }

    LCD_CTRL_FLAG = LCD_CTRL_START;

    bmp_debug();

    return OK;
}

int32_t bmp_start(void)
{
    LCD_CTRL_FLAG = LCD_CTRL_START;
    return LCD_CTRL_FLAG;
}

int32_t bmp_dinit(void)
{
    if (BMP_FILE != NULL)
    {
        fclose(BMP_FILE);
        BMP_FILE = NULL;
    }

    if (BMP_FILE_BUFF != NULL)
    {
        free(BMP_FILE_BUFF);
        BMP_FILE_BUFF = NULL;
    }

    memset(&IMG_HDR, 0, IMG_HDR_LEN);

    LCD_CTRL_FLAG = LCD_CTRL_STOP;

    lcd_clear(LCD_COL_FALSE);

    return OK;
}

// 返回自系统开机以来的毫秒数（tick）
static uint32_t GetTickCount()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

int32_t bmp_show(int32_t x0, int32_t y0, int32_t colour)
{
    uint8_t *bmp_buff = NULL;
    static int32_t frame = 0;

#if 1 //DEBUG
    int8_t frame_string[100] = {0};

    int32_t img_frame_width = IMG_FRAME_WIDTH;
    int32_t img_frame_size = IMG_FRAME_LEN;
    int32_t img_file_size = IMG_FILE_LEN;
#endif
    static uint32_t frame_fps0 = 0;
    static uint32_t frame_fps1 = 0;
    uint32_t video_fps_delay = 0;

    if (BMP_FILE_BUFF == NULL)
    {
        return LCD_CTRL_STOP;
    }

    switch (LCD_CTRL_FLAG)
    {
    case LCD_CTRL_START:
        frame = 0;
        LCD_CTRL_FLAG = LCD_CTRL_RUN;
        break;
    case LCD_CTRL_RUN:
        if (frame < (IMG_HDR.video_frame - 1))
        {
            frame++;
        }
        else
        {
            LCD_CTRL_FLAG = LCD_CTRL_STOP;
        }
        break;
    case LCD_CTRL_STOP:
    default:
        return LCD_CTRL_STOP;
        break;
    }

    bmp_buff = (BMP_FILE_BUFF + (frame * IMG_FRAME_LEN));
#if 1
    x0 = ((LCD_MAX_X - IMG_HDR.lcd_width) / 2) - 1;
    y0 = ((LCD_MAX_Y - IMG_HDR.lcd_height) / 2) - 1;
#endif
    if (lcd_putbmpspeed(x0, y0, IMG_HDR.lcd_width, IMG_HDR.lcd_height, bmp_buff, colour) != OK)
    {
        return LCD_CTRL_STOP;
    }

#if 0 //DEBUG
    sprintf(frame_string, "%04d", frame);
    led_puts(0, 10, frame_string, colour, !colour);
#endif
    lcd_update();
    frame_fps1 = GetTickCount();
    if ((frame_fps1 - frame_fps0) < (1000 / IMG_HDR.video_fps))
    {
        video_fps_delay = ((1000 / IMG_HDR.video_fps) - (frame_fps1 - frame_fps0) + IMG_FRAME_DELAY);
        if(video_fps_delay > 0) 
        {
            DEBUG_LOG("Waiting [%d]ms ...", video_fps_delay);
            delay_xms(video_fps_delay);
        }
    }
    frame_fps0 = frame_fps1;
    return LCD_CTRL_FLAG;
}
