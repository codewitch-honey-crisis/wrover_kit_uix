#include <stdio.h>
#include <string.h>
#include "lcd_pin_config.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_ili9341.h"
#include "lcd_controller.h"

static esp_lcd_panel_handle_t panel_handle = NULL;
static void *user_ctx = NULL;
static esp_lcd_panel_io_color_trans_done_cb_t on_color_trans_done_cb = NULL;

void lcd_color_trans_done_register_cb(esp_lcd_panel_io_color_trans_done_cb_t color_trans_done_cb, void *data)
{
    on_color_trans_done_cb = color_trans_done_cb;
    user_ctx = data;
}

esp_err_t lcd_flush(int16_t x1, int16_t y1, int16_t x2, int16_t y2, void *color_map)
{
    return esp_lcd_panel_draw_bitmap(panel_handle, x1, y1, x2+1 , y2+1 , color_map);
}

esp_err_t lcd_init(size_t max_transfer_bytes)
{
    gpio_config_t bk_cs_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << PIN_NUM_BCKL)
    };
    gpio_config(&bk_cs_gpio_config);
    if(PIN_NUM_BCKL!=-1) {
        gpio_set_level(PIN_NUM_BCKL, LCD_BK_LIGHT_OFF_LEVEL);
    }
    
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = 16,
    };
    
    spi_bus_config_t buscfg;
    memset(&buscfg, 0, sizeof(buscfg));
    buscfg.sclk_io_num = PIN_NUM_CLK;
    buscfg.mosi_io_num = PIN_NUM_MOSI;
    buscfg.miso_io_num = -PIN_NUM_MISO;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = max_transfer_bytes;
     // Initialize the SPI bus on VSPI (SPI3)
    spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config;
    memset(&io_config, 0, sizeof(io_config));
    io_config.dc_gpio_num = PIN_NUM_DC,
    io_config.cs_gpio_num = PIN_NUM_CS,
    io_config.pclk_hz = LCD_PIXEL_CLOCK_HZ,
    io_config.lcd_cmd_bits = 8,
    io_config.lcd_param_bits = 8,
    io_config.spi_mode = 0,
    io_config.trans_queue_depth = 10,
    io_config.on_color_trans_done = on_color_trans_done_cb;
    // Attach the LCD to the SPI bus
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle);

    esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle);

    static const uint8_t rotation = 1;

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_invert_color(panel_handle, false);
    bool swap_xy, mirror_x,mirror_y;
    switch(rotation&3) {
        case 0:
            mirror_x = true;
            mirror_y = false;
            swap_xy  = false;
            break;
        case 1:
            mirror_x = false;
            mirror_y = false;
            swap_xy = true;
            break;
        case 2:
            mirror_x = false;
            mirror_y = true;
            swap_xy = false;
            break;
        case 3:
            mirror_x = true;
            mirror_y = true;
            swap_xy = true;
            break;
    }
    esp_lcd_panel_swap_xy(panel_handle,swap_xy);
    esp_lcd_panel_mirror(panel_handle,mirror_x,mirror_y);
    
    esp_lcd_panel_set_gap(panel_handle, 0, 0);
    esp_lcd_panel_disp_off(panel_handle, true);

    /* Turn on LCD backlight */
    gpio_set_level(PIN_NUM_BCKL, LCD_BK_LIGHT_ON_LEVEL);

    
    
    return true;
}
