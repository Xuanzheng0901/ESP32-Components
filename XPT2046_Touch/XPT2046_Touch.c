#include <stdio.h>
#include "XPT2046_Touch.h"

esp_lcd_touch_handle_t touch_handle = NULL;
extern bool first_touch;

void lvgl_touchpad_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    uint16_t touch_x[1] = {0};
    uint16_t touch_y[1] = {0};
    uint8_t touch_cnt = 0;

    /* 从触摸设备读取坐标 */
    esp_lcd_touch_read_data(touch_handle);
    esp_lcd_touch_get_coordinates(touch_handle, touch_x, touch_y, NULL, &touch_cnt, 1);

    //ESP_LOGI("TOUCH", "raw: x:%d, y:%d", touch_x[0], touch_y[0]);
    // if(touch_x[0] > 160)
    //     touch_x[0] = (uint16_t)(((float)(touch_x[0]-160))*1.12)  + 160;
    // else if(touch_x[0] < 160 && touch_x[0] > 0)
    //     touch_x[0] =  160 - (uint16_t)(((float)(160-touch_x[0]))*1.09);

    // if(touch_y[0] > 120)
    //     touch_y[0] = (uint16_t)(((float)(touch_y[0]-120))*1.15) + 120;
    // else if(touch_y[0] < 120 && touch_y[0] > 0)
    //     touch_y[0] = 120 - (uint16_t)(((float)(120-touch_y[0]))*1.12);
    ESP_LOGI("TOUCH", "mod: x:%d, y:%d", touch_x[0], touch_y[0]);

    if (touch_cnt > 0) 
    {//进行坐标处理
        data->point.x = touch_x[0];
        data->point.y = touch_y[0];
        data->state = LV_INDEV_STATE_PRESSED;
        /*
        可以在这里进行一些关于触摸位置的回调
        */
    } 
    else 
    {
        data->state = LV_INDEV_STATE_RELEASED;
        //first_touch = true;
    }
}

void app_touch_init(void)
{
    spi_bus_config_t buscfg = {
        .miso_io_num = TOUCH_GPIO_MISO,
        .mosi_io_num = TOUCH_GPIO_MOSI,
        .sclk_io_num = TOUCH_GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(TOUCH_GPIO_CS);
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)TOUCH_SPI_NUM, &io_config, &io_handle);

    esp_lcd_touch_config_t touch_cfg = {
        .x_max = 320,
        .y_max = 240,
        .rst_gpio_num = GPIO_NUM_NC,
        .int_gpio_num = TOUCH_GPIO_INTR,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 1,
            .mirror_y = 0,
        },
    };
    esp_lcd_touch_new_spi_xpt2046(io_handle, &touch_cfg, &touch_handle);
    
}
