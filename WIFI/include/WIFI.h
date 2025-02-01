#ifndef __WIFI_H
#define __WIFI_H
#include <stdio.h>
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "sdkconfig.h"

void wifi_init(void);

#endif