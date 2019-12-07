#ifndef __FONT_CODE_H_
#define __FONT_CODE_H_

#include "type.h"

typedef enum font_name_e
{
    FONT_5X7,//
    FONT_5X8,
    FONT_7X12,
    FONT_8X16,//
    FONT_11X16,
    FONT_14X20,
    FONT_17X24,
    FONT_MAX,
    FONT_MIN = 0,
} font_name_t;

typedef struct font_s
{
    int32  name;
    int32  width;
    int32  height;
    int8  cmin;
    int8  cmax;
    uint8* pdata;
} font_t;

#define FNT_EN_ASCII_MIN ' '//32  -- 0
#define FNT_EN_ASCII_MAX '~'//126 -- 94

extern font_t* font_get(uint8_t fidx);

#endif