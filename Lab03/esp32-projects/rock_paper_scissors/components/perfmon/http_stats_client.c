#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "perfmon.h"
#include "task_monitor.h"
#include "perfmon_settings.h"
#include "stats_serializer.h"
#include "esp_http_client.h"

#include "http_stats_client.h"


static const char *TAG = "HTTP_CLIENT";

static QueueHandle_t stats_queue;

static char ** http_stats_client_filter = NULL;
static size_t http_stats_client_filter_length = 0;

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

static void http_stats_client_sendRequest(esp_http_client_handle_t client, char * post_data) {    
    //POST-Request

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

static void http_stats_client_perfmon_task(void * args) {
    esp_http_client_config_t config = {
        //.url = "http://httpbin.org/post", //For testing the POST-request
        .host =  DDM_HOST,
        .port = DDM_PORT,
        .path = DDM_PATH,
        .event_handler = _http_event_handler
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    char * post_data = NULL;
    perf_stats * perfmon_buffer;

    while (1) {
        //Get stats data from queue
        perfmon_buffer = malloc(sizeof(perf_stats));
        xQueueReceive(stats_queue, perfmon_buffer, portMAX_DELAY);
        post_data = stats_serializer_perf_stats(perfmon_buffer);
        free(perfmon_buffer);
        
        http_stats_client_sendRequest(client, post_data);
        free(post_data);
        //vTaskDelay(pdMS_TO_TICKS(1000)); 
    }        
    vTaskDelete(NULL);
}



static void task_monitor_filter(task_monitor_stats_table * buffer, char ** filter, size_t length) {
    if (filter != NULL) {   
        task_monitor_stats_table temp;
        temp.size = 0;
        for (int i = 0; i < buffer->size; i++) {
            for (int j = 0; j < length; j++) {
                if (!strcmp(buffer->taskstats[i].pcTaskName, filter[j])) {
                    temp.taskstats[temp.size] = buffer->taskstats[i];
                    ++temp.size;
                    break;
                }
            }
        }
        buffer->size = temp.size;
        for (int i = 0; i < temp.size; i++) {
            buffer->taskstats[i] = temp.taskstats[i];
        }
    }
}


static void http_stats_client_task_monitor_task(void * args) {
    esp_http_client_config_t config = {
        //.url = "http://httpbin.org/post", //For testing the POST-request
        .host =  DDM_HOST,
        .port = DDM_PORT,
        .path = DDM_PATH,
        .event_handler = _http_event_handler
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    char * post_data = NULL;
    task_monitor_stats_table * task_monitor_buffer = NULL;

    while (1) {
        //Get stats data from queue
        task_monitor_buffer = malloc(sizeof(task_monitor_stats_table));
        xQueueReceive(stats_queue, task_monitor_buffer, portMAX_DELAY);
        task_monitor_filter(task_monitor_buffer, http_stats_client_filter, http_stats_client_filter_length);

        post_data = stats_serializer_stats_table(task_monitor_buffer);
        free(task_monitor_buffer);

        http_stats_client_sendRequest(client, post_data);
        free(post_data);
        //vTaskDelay(pdMS_TO_TICKS(1000)); 
    }        
    vTaskDelete(NULL);
}

void http_stats_client_perfmon_start (void) {

   
    stats_queue = xQueueCreate(1, sizeof(perf_stats));
    perfmon_start(stats_queue);

    //Damit dns lookup nicht vor ip-erhalt, besser: Semaphore!
    vTaskDelay(pdMS_TO_TICKS(1000));
    xTaskCreate(&http_stats_client_perfmon_task, "http_request", 4*1024, NULL, 1, NULL);    
}



void http_stats_client_task_monitor_start (const char ** filter, size_t filter_length) {
    if (filter != NULL) {
        http_stats_client_filter = filter;
        http_stats_client_filter_length = filter_length;
    }
    stats_queue = xQueueCreate(1, sizeof(task_monitor_stats_table));
    task_monitor_start(stats_queue);
 
    //Damit dns lookup nicht vor ip-erhalt, besser: Semaphore!
    vTaskDelay(pdMS_TO_TICKS(1000));
    xTaskCreate(&http_stats_client_task_monitor_task, "http_request", 4*1024, NULL, 1, NULL);    
}
