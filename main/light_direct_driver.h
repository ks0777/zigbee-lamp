#include <errno.h>
#include <stdbool.h>
#include "sdkconfig.h"
#include "esp_err.h"

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_netif.h"


#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define PORT 1337

esp_err_t wifi_sta_do_connect(bool wait);
bool example_is_our_netif(const char *prefix, esp_netif_t *netif);

static const char *TAG_LOL = "ZB_LIGHT_DIRECT";
static esp_netif_t *s_example_sta_netif = NULL;
static SemaphoreHandle_t s_semph_get_ip_addrs = NULL;
static int s_retry_num = 0;

void set_colors_direct(uint16_t r, uint16_t g, uint16_t b, uint16_t cw, uint16_t ww);

//static void udp_server_task(void *pvParameters);
static void udp_server_task(void *pvParameters) {
    char rx_buffer[10];
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    while (1) {
		struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
		dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
		dest_addr_ip4->sin_family = AF_INET;
		dest_addr_ip4->sin_port = htons(PORT);
		ip_protocol = IPPROTO_IP;

        int sock = socket(AF_INET, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG_LOL, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG_LOL, "Socket created");

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG_LOL, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG_LOL, "Socket bound, port %d", PORT);

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);

        while (1) {
            //ESP_LOGI(TAG_LOL, "Waiting for data");
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer), 0, (struct sockaddr *)&source_addr, &socklen);

			if (len == 1) {
				nvs_handle_t nvs_handle;
				ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));
				uint8_t current_mode;
				nvs_get_u8(nvs_handle, "direct_mode", &current_mode);
				nvs_set_u8(nvs_handle, "direct_mode", rx_buffer[0] != 0);
				nvs_close(nvs_handle);
				if (current_mode != (rx_buffer[0] != 0)) {
					esp_restart();	
				}
			} else if (len == 10) {
				uint16_t *colors = (uint16_t*)rx_buffer;
				set_colors_direct(colors[0], colors[1], colors[2], colors[3], colors[4]);
            } else {
                ESP_LOGE(TAG_LOL, "recvfrom failed: errno %d", errno);
            }
        }

        if (sock != -1) {
            ESP_LOGE(TAG_LOL, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}


static void example_handler_on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    s_retry_num++;
    if (s_retry_num > 3) {
        ESP_LOGI(TAG_LOL, "WiFi Connect failed %d times, stop reconnect.", s_retry_num);
        /* let example_wifi_sta_do_connect() return */
        if (s_semph_get_ip_addrs) {
            xSemaphoreGive(s_semph_get_ip_addrs);
        }
        return;
    }
    ESP_LOGI(TAG_LOL, "Wi-Fi disconnected, trying to reconnect...");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        return;
    }
    ESP_ERROR_CHECK(err);
}

static void example_handler_on_wifi_connect(void *esp_netif, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
}

static void example_handler_on_sta_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    s_retry_num = 0;
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    if (!example_is_our_netif("netif_sta", event->esp_netif)) {
        return;
    }
    ESP_LOGI(TAG_LOL, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    if (s_semph_get_ip_addrs) {
        xSemaphoreGive(s_semph_get_ip_addrs);
    } else {
        ESP_LOGI(TAG_LOL, "- IPv4 address: " IPSTR ",", IP2STR(&event->ip_info.ip));
    }
}

