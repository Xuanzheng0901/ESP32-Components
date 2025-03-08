#include "Storage.h"

static const char* TAG = "SPI_FLASH";

void SPI_FLASH_Init(void)
{
    esp_vfs_spiffs_conf_t cfg = {
        .base_path = "/sdcard",
        .partition_label = "storage",
        .max_files = 3,
        .format_if_mount_failed = 1
    };

    esp_vfs_spiffs_register(&cfg);
    size_t total = 0, used = 0;
    esp_spiffs_info(cfg.partition_label, &total, &used);
    ESP_LOGI(TAG, "分区大小: 总=%d, 已用=%d", total, used);
}

void read_file(const char* file_name, char* buffer)
{
    FILE* f = fopen(file_name, "r");
    if(f == NULL)
    {
        printf("文件不存在或被占用");
        return;
    }
    memset(buffer, 0, sizeof(buffer));
    char read_buffer[100];
    while(fgets(read_buffer, sizeof(read_buffer), f) != NULL)
    {
        strcat(buffer, read_buffer);
        memset(read_buffer, 0, sizeof(read_buffer));
    }
}