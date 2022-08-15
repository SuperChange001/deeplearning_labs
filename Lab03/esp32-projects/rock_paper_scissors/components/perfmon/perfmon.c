#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>

#include "perfmon.h"


#define STATS_TICKS         pdMS_TO_TICKS(1000)
#define ARRAY_SIZE_OFFSET   5   //Increase this if print_real_time_stats returns ESP_ERR_INVALID_SIZE

static QueueHandle_t stats_queue;
static const char * TAG = "perfmon";


static int inline perfmon_calculate_percentage (size_t amount, size_t total) {
	return (int)(((float) amount / (float) total) * 100.f);
}

static esp_err_t perfmon_getHeapStats (heap_stats * stats) {
	size_t total_dram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
	size_t total_external = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);	
	size_t free_dram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
	size_t free_external = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

	int percentage_dram = perfmon_calculate_percentage(free_dram, total_dram);
    int percentage_external = 100;
    //Check if SPIRAM is enabled
    if (total_external != 0) {
	    percentage_external = perfmon_calculate_percentage(free_external, total_external);
    }

    stats->heap_dram = total_dram - free_dram,
    stats->percentage_dram = 100 - percentage_dram,
    stats->heap_external = total_external - free_external,
    stats->percentage_external = 100 - percentage_external;

    return ESP_OK;                  
}

static esp_err_t perfmon_getCpuStats (cpu_stats * stats) {
    TaskStatus_t *start_array = NULL, *end_array = NULL;
    UBaseType_t start_array_size, end_array_size;
    uint32_t start_run_time, end_run_time;
    esp_err_t ret;


    //Allocate array to store current task states
    start_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    start_array = malloc(sizeof(TaskStatus_t) * start_array_size);
    if (start_array == NULL) {
        ret = ESP_ERR_NO_MEM;
        goto exit;
    }
    //Get current task states
    start_array_size = uxTaskGetSystemState(start_array, start_array_size, &start_run_time);
    if (start_array_size == 0) {
        ret = ESP_ERR_INVALID_SIZE;
        goto exit;
    }

    vTaskDelay(STATS_TICKS);

    //Allocate array to store tasks states post delay
    end_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    end_array = malloc(sizeof(TaskStatus_t) * end_array_size);
    if (end_array == NULL) {
        ret = ESP_ERR_NO_MEM;
        goto exit;
    }
    //Get post delay task states
    end_array_size = uxTaskGetSystemState(end_array, end_array_size, &end_run_time);
    if (end_array_size == 0) {
        ret = ESP_ERR_INVALID_SIZE;
        goto exit;
    }

    //Calculate total_elapsed_time in units of run time stats clock period.
    uint32_t total_elapsed_time = (end_run_time - start_run_time);
    if (total_elapsed_time == 0) {
        ret = ESP_ERR_INVALID_STATE;
        goto exit;
    }
    

    int idle0_startIndex = 0;
    int idle1_startIndex = 0;

    for (int i = 0; i < start_array_size; i++) {
        if (start_array[i].xHandle == xTaskGetIdleTaskHandleForCPU(0)) {
            idle0_startIndex = i;
        }
        else if (start_array[i].xHandle == xTaskGetIdleTaskHandleForCPU(1)) {
            idle1_startIndex = i;
        }
    }

    for (int i = 0; i < end_array_size; i++) {
        if (end_array[i].xHandle == xTaskGetIdleTaskHandleForCPU(0)) {
            uint32_t task_elapsed_time = end_array[i].ulRunTimeCounter - start_array[idle0_startIndex].ulRunTimeCounter;
            //Stats per core, so dont divide through number of cores!
            uint32_t percentage_time = (task_elapsed_time * 100UL) / (total_elapsed_time); 
            stats->cpu0 = 100 - percentage_time;
        }
        else if (end_array[i].xHandle == xTaskGetIdleTaskHandleForCPU(1)) {
            uint32_t task_elapsed_time = end_array[i].ulRunTimeCounter - start_array[idle1_startIndex].ulRunTimeCounter;
            //Stats per core, so dont divide through number of cores!
            uint32_t percentage_time = (task_elapsed_time * 100UL) / (total_elapsed_time);
            stats->cpu1 = 100 - percentage_time;
        }
    }
 
    ret = ESP_OK;

    exit:    //Common return path
    free(start_array);
    free(end_array);
    return ret;    
}

esp_err_t perfmon_getStats(perf_stats * stats) {
    esp_err_t ret = ESP_OK;

    ret = perfmon_getCpuStats(&stats->cpustats);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = perfmon_getHeapStats(&stats->heapstats);
    if (ret != ESP_OK) {
        return ret;
    }
    return ret;
}

static void perfmon_print_stats(const perf_stats * stats) {
    ESP_LOGI(TAG, "CPU0: %d, CPU1: %d", stats->cpustats.cpu0, stats->cpustats.cpu1);
    ESP_LOGI(TAG, "DRAM used: %d, In percent: %d", stats->heapstats.heap_dram, stats->heapstats.percentage_dram);
    ESP_LOGI(TAG, "External RAM used: %d, In percent: %d", stats->heapstats.heap_external, stats->heapstats.percentage_external);
}

static void perfmon_task (void * args) {
    while (1) {
        perf_stats stats;
        if (ESP_OK == perfmon_getStats(&stats)) {
            xQueueSendToBack(stats_queue, &stats, portMAX_DELAY);
            perfmon_print_stats(&stats);
        }
        //vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelete(NULL);
}


void perfmon_start(QueueHandle_t queue) {
    stats_queue = queue;
    xTaskCreatePinnedToCore(perfmon_task, "perfmon", 2*1024, NULL, 3, NULL, tskNO_AFFINITY);
}