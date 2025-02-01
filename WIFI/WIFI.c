#include "WIFI.h"
static const char* TAG = "WiFi-Sta";
static esp_netif_t *netif_handle = NULL;
uint8_t wifi_status = 0;

static void netif_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT)
    {
        if(event_id == WIFI_EVENT_STA_CONNECTED)
        {
            esp_netif_create_ip6_linklocal(netif_handle); //连接至wifi后立即获取ipv6
            wifi_status = 1;
        }
        else if(event_id == WIFI_EVENT_STA_DISCONNECTED || event_id == WIFI_EVENT_STA_START)
        {
            wifi_status = 0;
            esp_wifi_connect(); //wifi启动或断连时尝试连接至目标wifi
        }
    }
    else if(event_base == IP_EVENT) //打印ip
    {
        if(event_id == IP_EVENT_STA_GOT_IP)
            ESP_LOGW(TAG, "Got IPv4 address:"IPSTR, IP2STR(&((ip_event_got_ip_t *)event_data)->ip_info.ip));
        else if(event_id == IP_EVENT_GOT_IP6)
            ESP_LOGW(TAG, "Got IPv6 address:"IPV6STR, IPV62STR(((ip_event_got_ip6_t *)event_data)->ip6_info.ip));
    }
}

void wifi_init(void)
{
    nvs_flash_init();
    esp_event_loop_create_default();
    esp_netif_init();

    netif_handle = esp_netif_create_default_wifi_sta();

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &netif_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &netif_event_handler, NULL);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .threshold.authmode = CONFIG_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        },
    };

    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
}
