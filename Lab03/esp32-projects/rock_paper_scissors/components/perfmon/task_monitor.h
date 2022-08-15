#ifndef TASK_MONITOR_H
#define TASK_MONITOR_H

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define MAX_TASK_NUM 20                         // Max number of per tasks info that it can store
#define MAX_BLOCK_NUM 20                        // Max number of per block info that it can store
#define MAX_TASK_NAME 50

typedef struct task_monitor_taskstats {
    TaskHandle_t taskHandle;
    char pcTaskName[MAX_TASK_NAME];	
    uint32_t task_elapsed_time;
    uint32_t percentage_time;
    size_t stack_size;
    size_t heap_size_internal;
    size_t heap_percentage_internal;
    size_t heap_size_external;
    size_t heap_percentage_external;
} task_monitor_taskstats;

typedef struct task_monitor_stats_table {
    task_monitor_taskstats taskstats[MAX_TASK_NUM];
    size_t size;
} task_monitor_stats_table;

void task_monitor_start(QueueHandle_t queue);

#endif
