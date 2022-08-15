/* FreeRTOS Real Time Stats Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_task_info.h"
#include "esp_log.h"
#include "esp_err.h"

#include "task_monitor.h"

#include "TCB_t.h"

#define STATS_TASK_PRIO     3
#define STATS_TICKS         pdMS_TO_TICKS(1000)
#define ARRAY_SIZE_OFFSET   5   //Increase this if print_real_time_stats returns ESP_ERR_INVALID_SIZE

static QueueHandle_t taskstats_queue;

static size_t s_prepopulated_num = 0;
static heap_task_totals_t s_totals_arr[MAX_TASK_NUM];
static heap_task_block_t s_block_arr[MAX_BLOCK_NUM];

static const char * TAG = "task_monitor";

static void task_monitor_populate_taskstats (task_monitor_stats_table * stats, TaskStatus_t * start_array, UBaseType_t start_array_size, 
        TaskStatus_t * end_array, UBaseType_t end_array_size, uint32_t total_elapsed_time);

/**
 * @brief   Function to print the CPU usage of tasks over a given duration.
 *
 * This function will measure and print the CPU usage of tasks over a specified
 * number of ticks (i.e. real time stats). This is implemented by simply calling
 * uxTaskGetSystemState() twice separated by a delay, then calculating the
 * differences of task run times before and after the delay.
 *
 * @note    If any tasks are added or removed during the delay, the stats of
 *          those tasks will not be printed.
 * @note    This function should be called from a high priority task to minimize
 *          inaccuracies with delays.
 * @note    When running in dual core mode, each core will correspond to 50% of
 *          the run time.
 *
 * @param   xTicksToWait    Period of stats measurement
 *
 * @return
 *  - ESP_OK                Success
 *  - ESP_ERR_NO_MEM        Insufficient memory to allocated internal arrays
 *  - ESP_ERR_INVALID_SIZE  Insufficient array size for uxTaskGetSystemState. Trying increasing ARRAY_SIZE_OFFSET
 *  - ESP_ERR_INVALID_STATE Delay duration too short
 */

static esp_err_t task_monitor_getStats(task_monitor_stats_table * stats_table, TickType_t xTicksToWait)
{
    TaskStatus_t *start_array = NULL, *end_array = NULL;
    UBaseType_t start_array_size, end_array_size;
    uint32_t start_run_time, end_run_time;
    task_monitor_taskstats * taskStats = NULL;
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

    vTaskDelay(xTicksToWait);

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

    //Populate the task stats table
    task_monitor_populate_taskstats(stats_table, start_array, start_array_size, end_array, end_array_size, total_elapsed_time);

    ret = ESP_OK;

exit:    //Common return path
    free(taskStats);
    free(start_array);
    free(end_array);
    return ret;
}


static void task_monitor_print_stats(const task_monitor_stats_table * stats) {
    //Print out the stats information
    ESP_LOGI(TAG, "| Task | Run Time | Percentage | Internal Heap Mem | External Heap Mem");
    for (int i = 0; i < stats->size; i++) {
        /*Testing out getting stack sizes of task:
        TCB_t * tcb = (TCB_t *)stats->taskstats[i].taskHandle;
        UBaseType_t stack_size = (UBaseType_t)(tcb->pxEndOfStack) - (UBaseType_t)(tcb->pxStack) + 4;
        ESP_LOGI(TAG, "%d", stack_size);
        */
        ESP_LOGI(TAG, "%s | %d | %d%% |%d | %d", stats->taskstats[i].pcTaskName, stats->taskstats[i].task_elapsed_time, 
            stats->taskstats[i].percentage_time, stats->taskstats[i].heap_size_internal, stats->taskstats[i].heap_size_external);
    }
}

static void task_monitor_stats_task(void *arg)
{
    //Delay to make heap tracking work
    vTaskDelay(pdMS_TO_TICKS(5000));

    //Print real time stats periodically
    while (1) {
        ESP_LOGI(TAG, "Getting real time stats over %d ticks", STATS_TICKS);
        task_monitor_stats_table stats_table;
        if (task_monitor_getStats(&stats_table, STATS_TICKS) == ESP_OK) {
            task_monitor_print_stats(&stats_table);
            ESP_LOGI(TAG, "Real time stats obtained");
            xQueueSendToBack(taskstats_queue, &stats_table, portMAX_DELAY);
        } else {
            ESP_LOGI(TAG, "Error getting real time stats");
        }
        //vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelete(NULL);
}

void task_monitor_start (QueueHandle_t queue)
{
    //Allow other core to finish initialization
    vTaskDelay(pdMS_TO_TICKS(100));
    
    taskstats_queue = queue;
    xTaskCreatePinnedToCore(task_monitor_stats_task, "stats", 4096, NULL, STATS_TASK_PRIO, NULL, tskNO_AFFINITY);
}

static inline size_t task_monitor_get_stack_size(TaskHandle_t handle) {
        UBaseType_t stack_size = 0;
        if (handle != NULL) {
            TCB_t * tcb = (TCB_t *)handle;
            stack_size = (UBaseType_t)(tcb->pxEndOfStack) - (UBaseType_t)(tcb->pxStack) + 4;
        }    
        return stack_size;
}


static void task_monitor_populate_taskstats (task_monitor_stats_table * stats, TaskStatus_t * start_array, UBaseType_t start_array_size, 
        TaskStatus_t * end_array, UBaseType_t end_array_size, uint32_t total_elapsed_time) {

    //Get heap information
    heap_task_info_params_t heap_info = {0};
    heap_info.caps[0] = MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL;      // Gets internal heap with CAP_8BIT capabilities
    heap_info.mask[0] = MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL;
    heap_info.caps[1] = MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM;       // Gets external heap info with CAP_8BIT capabilities
    heap_info.mask[1] = MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM; 
    heap_info.tasks = NULL;                     // Passing NULL captures heap info for all tasks
    heap_info.num_tasks = 0;
    heap_info.totals = s_totals_arr;            // Gets task wise allocation details
    heap_info.num_totals = &s_prepopulated_num;
    heap_info.max_totals = MAX_TASK_NUM;        // Maximum length of "s_totals_arr"
    heap_info.blocks = s_block_arr;             // Gets block wise allocation details. For each block, gets owner task, address and size
    heap_info.max_blocks = MAX_BLOCK_NUM;       // Maximum length of "s_block_arr"

    heap_caps_get_per_task_info(&heap_info);

    //Set size of taskstats array
    stats->size = start_array_size+1;

    size_t heap_caps_internal = heap_caps_get_total_size(MALLOC_CAP_8BIT| MALLOC_CAP_INTERNAL);
    size_t heap_caps_spiram = heap_caps_get_total_size(MALLOC_CAP_8BIT| MALLOC_CAP_SPIRAM);

    //Populate the taskstats structs with information
    for (int i = 0; i < start_array_size; i++) {
        memset(&stats->taskstats[i], 0, sizeof(task_monitor_taskstats));
        stats->taskstats[i].taskHandle = start_array[i].xHandle;
        strncpy(stats->taskstats[i].pcTaskName, start_array[i].pcTaskName, MAX_TASK_NAME);


        //Get stack size
        stats->taskstats[i].stack_size = task_monitor_get_stack_size(stats->taskstats[i].taskHandle);

        //Get CPU-Load stats
        for (int j = 0; j < end_array_size; j++) {
            if (stats->taskstats[i].taskHandle == end_array[j].xHandle) {
                stats->taskstats[i].task_elapsed_time = end_array[j].ulRunTimeCounter - start_array[i].ulRunTimeCounter;
                stats->taskstats[i].percentage_time = (stats->taskstats[i].task_elapsed_time * 100UL) / (total_elapsed_time * portNUM_PROCESSORS);
                break;
            }
        }

        //Get heap usage stats
        for (int j = 0; j < *heap_info.num_totals; j++) {
            if (stats->taskstats[i].taskHandle == heap_info.totals[j].task) {
                stats->taskstats[i].heap_size_internal = heap_info.totals[j].size[0];
                stats->taskstats[i].heap_percentage_internal = (heap_info.totals[j].size[0] * 100UL)/(heap_caps_internal);
                //Check if SPIRAM is actually enabled
                if (heap_caps_spiram != 0) {
                    stats->taskstats[i].heap_size_external = heap_info.totals[j].size[1];
                    stats->taskstats[i].heap_percentage_external = (heap_info.totals[j].size[1] * 100UL)/(heap_caps_spiram);
                }
                break;
            }
        }
    }    
    //Get pre-scheduler allocations
    int endIndex = stats->size-1;
    for (int i = 0; i < *heap_info.num_totals; i++) {
        if  (heap_info.totals[i].task == NULL) {
            memset(&stats->taskstats[endIndex], 0, sizeof(task_monitor_taskstats));
            stats->taskstats[endIndex].taskHandle = NULL;
            strncpy(stats->taskstats[endIndex].pcTaskName, "Pre-scheduler", MAX_TASK_NAME);
            stats->taskstats[endIndex].stack_size = task_monitor_get_stack_size(stats->taskstats[endIndex].taskHandle);
            stats->taskstats[endIndex].heap_size_internal = heap_info.totals[i].size[0];
            stats->taskstats[endIndex].heap_percentage_internal = (heap_info.totals[i].size[0] * 100UL)/(heap_caps_internal);
            if (heap_caps_spiram != 0) {
                stats->taskstats[endIndex].heap_size_external = heap_info.totals[i].size[1];
                stats->taskstats[endIndex].heap_percentage_external = (heap_info.totals[i].size[1] * 100UL)/(heap_caps_spiram);
            }
            break;
        }       
    }
}