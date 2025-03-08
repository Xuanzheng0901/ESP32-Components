#include "stdio.h"
#include "FreeRTOS/FreeRTOS.h"
#include <sys/socket.h>
#include "esp_log.h"
#include "S0cket.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "SoftAP_Provisioning.h"

#define SERVER_IP CONFIG_SOCKET_IP
#define SERVER_PORT CONFIG_SOCKET_PORT

static char* TAG =  "S0cket";
EventGroupHandle_t socket_group;

TaskHandle_t Rx_Handle, HB_Handle;
char rx_buffer[256] = {0};
uint32_t Server_IP6_32[4];
int sock = -1;

void socket_heartbeat(void *pvParameter) //向socket服务端发送心跳包
{
    while(1) 
    {  
        xEventGroupWaitBits(socket_group, SOCKET_TRANSFORM_IDLE, pdFALSE, pdFALSE, portMAX_DELAY); //等待发送数据任务完成
        xEventGroupClearBits(socket_group, SOCKET_HEARTBEAT_IDLE); 
        if(send(sock, "HeartBeat", 10, 0) < 0)
            break;
        xEventGroupSetBits(socket_group, SOCKET_HEARTBEAT_IDLE);
        vTaskDelay(2000);
    }
    xEventGroupSetBits(socket_group, SOCKET_DISCONNECTED);//发送错误失败处理
    vTaskDelay(100);
}

void Socket_Tx(char* tx_buffer)
{
    xEventGroupWaitBits(socket_group, SOCKET_HEARTBEAT_IDLE, pdFALSE, pdTRUE, portMAX_DELAY); //等待可能正在发送的心跳包
    xEventGroupClearBits(socket_group, SOCKET_TRANSFORM_IDLE);
    if(tx_buffer) //判断传入的待发送数据包是否存在
        ESP_LOGI(TAG, "%d", send(sock, tx_buffer, strlen(tx_buffer), 0));
    xEventGroupSetBits(socket_group, SOCKET_TRANSFORM_IDLE);
}

void rx_task(void *pvParameter) 
{
    int len = 0;
    while(1) 
    {
        len = recv(sock, rx_buffer, sizeof(rx_buffer), 0); //阻塞并等待服务端发送数据
        if (len <= 0) //接收失败处理,这里也作为socket连接存活的判定,如果socket连接断开,recv函数会立即返回0,进入错误处理
            break;
        else 
        {
            rx_buffer[len] = 0;
            ESP_LOGI("Socket", "收到消息: %s", rx_buffer);
            memset(rx_buffer, 0, sizeof(rx_buffer));
        }
        vTaskDelay(10);
    }
    xEventGroupSetBits(socket_group, SOCKET_DISCONNECTED);
    vTaskDelay(100);
}

void resolve_ipv6(const char *hostname) 
{
    struct addrinfo hints = 
    {
        .ai_family = AF_INET6,
        .ai_socktype = SOCK_STREAM
    }, *res;
    getaddrinfo(hostname, NULL, &hints, &res);
    for(int i = 0; i < 4; i++)
        Server_IP6_32[i] = ((struct sockaddr_in6 *)(res->ai_addr))->sin6_addr.un.u32_addr[i];
    freeaddrinfo(res);
}

void Socket_Connect(void)
{   
    //ipv4协议
    // struct sockaddr_in Socket_Handle = 
    // {
    //     .sin_addr.s_addr = inet_addr(SERVER_IP),
    //     .sin_family = AF_INET,
    //     .sin_port = htons(SERVER_PORT)
    // };
    // sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    struct sockaddr_in6 Socket_Handle6 = 
    {
        .sin6_family = AF_INET6,
        .sin6_port = htons(SERVER_PORT)
    };
    for(int i = 0; i < 4; i++)
        Socket_Handle6.sin6_addr.un.u32_addr[i] = Server_IP6_32[i];

    sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_IPV6);
    ESP_LOGI(TAG, "连接中...");
    
    //uint8_t ret = connect(sock, (struct sockaddr *)&Socket_Handle, sizeof(Socket_Handle));
    uint8_t ret = connect(sock, (struct sockaddr *)&Socket_Handle6, sizeof(Socket_Handle6));

    uint8_t retry_time = 0;
    while(ret != 0)
    {
        retry_time++;
        ESP_LOGW(TAG,"连接失败,当前重试次数:%d", retry_time);
        if(retry_time % 3 == 0)
            vTaskDelay(6000);
        else
            vTaskDelay(200);
        close(sock);
        sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_IPV6);
        ret = connect(sock, (struct sockaddr *)&Socket_Handle6, sizeof(Socket_Handle6)); 
    }
    ESP_LOGI(TAG, "连接成功!");
    xEventGroupClearBits(socket_group, SOCKET_DISCONNECTED);
    xTaskCreate(socket_heartbeat, "socket_heartbeat", 1024, NULL, 5, &HB_Handle); //创建任务
    xTaskCreate(rx_task, "rx_task", 2048, NULL, 5, &Rx_Handle);
}

void socket_daemon(void *pvParameter)
{
    ESP_LOGI(TAG, "Socket守护进程已启动");
    while(1)
    {
        vTaskDelay(50);
        xEventGroupWaitBits(socket_group, SOCKET_DISCONNECTED, pdFALSE, pdFALSE, portMAX_DELAY); //在任务报错后进行处理
        vTaskDelete(Rx_Handle);
        vTaskDelete(HB_Handle);
        close(sock);
        ESP_LOGW(TAG, "正在连接至服务器...");
        sock = -1;
        Socket_Connect();
    }
}

void Socket_Init(void) 
{
    socket_group = xEventGroupCreate();
    xEventGroupWaitBits(wifi_group, GLOBAL_IP6_ADDR_READY, pdFALSE, pdFALSE, portMAX_DELAY);
    resolve_ipv6(CONFIG_SOCKET_IPV6_SERVER);
    xEventGroupSetBits(socket_group, SOCKET_HEARTBEAT_IDLE | SOCKET_TRANSFORM_IDLE);
    xTaskCreate(socket_daemon, "Socket_Daemon", 2048, NULL, 6, NULL);
    Socket_Connect();
}
