#ifndef __STORAGE_H
#define __STORAGE_H

#include "stdio.h"
#include "string.h"
#include "esp_log.h"
#include "esp_spiffs.h"

void SPI_FLASH_Init(void);
void read_file(const char* file_name, char* buffer);

#endif