#pragma once
#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL 0
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL

#define LCD_HOST    HSPI_HOST

#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   22

#define PIN_NUM_DC   21
#define PIN_NUM_RST  18
#define PIN_NUM_BCKL 5