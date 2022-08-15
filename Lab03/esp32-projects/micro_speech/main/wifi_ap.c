#include "wifi_ap.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#define AP_SSID "esp-eye"
#define AP_PASSWORD "cuddlyNemunekoM3ow"

static const char * TAG = "wifi-station";

static void event_handler(void * arg, esp_event_base_t event_base, 
                                int32_t event_id, void * event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        /*When the station is started due to esp_wifi_start*/
        ESP_LOGI(TAG, "station started");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        /*When the station fails to connect to the AP*/
        ESP_LOGI(TAG, "disconnected, retry");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        /*When the station is connected and received an IP from the AP*/
        ip_event_got_ip_t * event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "received ip:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void wifi_station_init (void) {
    /*Initialize network stack*/
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));

    /*Add event handler to default loop*/
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = AP_SSID,
            .password = AP_PASSWORD,
        }  
    };

    /*Configure device as wifi-station*/
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    /*Start wifi-station*/
    ESP_ERROR_CHECK(esp_wifi_start());
} 

