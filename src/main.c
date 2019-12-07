#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "type.h"

#include "lcd.h"
#include "bmp.h"

#define LCD_MOVIE_NAME_LEN (1024)

#define HELP_PRINT_FORMATS "  %-10s -- %s\r\n"

static void print_usage(const char *exe_name)
{
    printf("Usage: %s <file-path>...\r\n", exe_name);
    printf("Options:\r\n");
    printf(HELP_PRINT_FORMATS, "-h,--help", "Show this help message.");
    printf(HELP_PRINT_FORMATS, "-f FILE_PATH,--file=FILE_PATH", "Lcd Movie Player File.");
    printf(HELP_PRINT_FORMATS, "-l LOOP_TIMES,--loop=LOOP_TIMES", "Loop Number Of Times.");
    printf("\r\n");
}

int main(int argc, char **argv)
{
    int ecode = 0;
    int opt = 0, opt_num = 0;
    int option_index = 0;
    char movie_path[LCD_MOVIE_NAME_LEN + 1] = {0};
    int loop_times = 1;
    static struct option long_options[] =
    {
        {"help", no_argument, 0, 'h'},
        {"file", required_argument, 0, 'f'},
        {"loop", required_argument, 0, 'l'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "f:l:h", long_options, &option_index)) != -1)
    {
        opt_num++;
        switch (opt)
        {
        case 0:
            printf("option[%d - %s]", option_index, long_options[option_index].name);
            if (optarg)
                printf(" with arg %s\n", optarg);
            switch (option_index)
            {
            case 1: // file
                strncpy(movie_path, optarg, LCD_MOVIE_NAME_LEN);
                break;
            case 2: // loop
                loop_times = atoi(optarg);
                break;
            default:
                break;
            }
            break;
        case 'f':
            strncpy(movie_path, optarg, LCD_MOVIE_NAME_LEN);
            break;
        case 'l':
            loop_times = atoi(optarg);
            break;
        case 'h':
        default:
            print_usage(argv[0]);
            ecode = 1;
            goto error;
        }
    }

    if (opt_num < 1)
    {
        // DEBUG_ERR(ecode, "Parameter required!");
        print_usage(argv[0]);
        ecode = 1;
        goto error;
    }

    if (strlen(movie_path) == 0)
    {
        DEBUG_ERR(ecode, "Invalid file path!");
        print_usage(argv[0]);
        ecode = 1;
        goto error;
    }

    if (loop_times <= 0)
    {
        DEBUG_ERR(ecode, "Invalid loop times!");
        print_usage(argv[0]);
        ecode = 1;
        goto error;
    }

    DEBUG_LOG("Play Movie [%s] Loop Times [%d].", movie_path, loop_times);

    lcd_init();
    ecode = bmp_init(movie_path);
    if (ecode != OK)
    {
        DEBUG_ERR(ecode, "Movie Player Init Error!");
        ecode = 2;
        goto error;
    }
    bmp_start();
    while (loop_times--)
    {
        while (bmp_show(0, 0, LCD_COL_TRUE) != LCD_CTRL_STOP);
        bmp_start();
    }
    bmp_dinit();
    DEBUG_LOG("Play Movie [%s] End.", movie_path);
    ecode = 0;
error:
    if (ecode > 1)
    {
        DEBUG_ERR(ecode, "Error to play movie!");
    }
    return ecode;
}