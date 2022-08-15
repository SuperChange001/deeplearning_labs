#include "esp_log.h"
#include "esp_http_client.h"
#include <string.h>

#include "http_time_stats_client.h"


#define DDM_HOST "192.168.10.117"
#define DDM_PORT 8080
#define DDM_PATH "/"

static char post_data[100];

static const char *TAG = "http_time_stats";


esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                ESP_LOGI(TAG, "%.*s", evt->data_len, (char*)evt->data);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

static void http_time_stats_client_send_request(esp_http_client_handle_t client, char * post_data) {    
    /*Post request*/
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_close(client);
}    

static inline void http_time_stats_serialize(char * buffer, size_t buffer_len, int64_t time) {
    snprintf(buffer, buffer_len, "{\"data\":[{\"time\":[%lld]}]}", time);
}


void http_time_stats_client_send_time(int64_t time_elapsed) {
    esp_http_client_config_t config = {
        //.url = "http://httpbin.org/post", //For testing the POST-request
        .host =  DDM_HOST,
        .port = DDM_PORT,
        .path = DDM_PATH,
        .event_handler = _http_event_handler
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    http_time_stats_serialize(post_data, 100, time_elapsed);
    http_time_stats_client_send_request(client, post_data);
}

