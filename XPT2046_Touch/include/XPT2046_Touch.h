#ifndef __XPT2046_TOUCH_H
#define __XPT2046_TOUCH_H

#include "driver/spi_master.h"
#include "esp_lcd_touch_xpt2046.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"

#define TOUCH_SPI_NUM             SPI3_HOST

#define TOUCH_GPIO_SCLK           10
#define TOUCH_GPIO_CS             11
#define TOUCH_GPIO_MOSI           12
#define TOUCH_GPIO_MISO           13
#define TOUCH_GPIO_INTR           14

void app_touch_init(void);
void lvgl_touchpad_read(lv_indev_t * indev, lv_indev_data_t * data);

#endif