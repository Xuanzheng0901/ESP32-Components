#include "esp_event.h"
#include "mqtt_client.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "MQTT_.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"

static const char* TAG = "MQTT_Client";
static esp_mqtt_client_handle_t mqtt_client;

char mqtt_recv_buf[256];

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    esp_mqtt_event_id_t id = (esp_mqtt_event_id_t)event_id;

    if(id == MQTT_EVENT_CONNECTED)
    {
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        int msg_id = esp_mqtt_client_subscribe(client, "topic", 2);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    }
    else if(id == MQTT_EVENT_DATA)
    {
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        strncpy(mqtt_recv_buf, event->data, event->data_len * sizeof(char));
        mqtt_recv_buf[event->data_len] = 0;
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
    }
}

void mqtt_app_send(const char* topic, const char* data, int retain)
{
    if(esp_mqtt_client_publish(mqtt_client, topic, data, strlen(data), 2, retain) >= 0)
        ESP_LOGI(TAG, "Message:\"%s\" Sent to topic:\"%s\" Successfully.", data, topic);
    else
        ESP_LOGE(TAG, "MQTT Message Send Failed!");
}

void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_MQTT_WS_URL,
        .credentials = 
        {
            .username = CONFIG_MQTT_USERNAME,
            .authentication.password = CONFIG_MQTT_PWD,
        },
        .task.stack_size = 4096,
        .session.last_will = 
        {
            .msg = "ESP32DISCONN",
            .qos = 2,
            .topic = CONFIG_SUB_TOPIC
        }
    };

    esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}