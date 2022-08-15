#ifndef WIFI_STA_H
#define WIFI_STA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"

    esp_err_t wifi_sta_init(void);
    esp_err_t wifi_sta_join(const char* ssid, const char* password);
    uint32_t wifi_get_local_ip(void);

#ifdef __cplusplus
}
#endif

#endif