#ifndef PERFMON_H
#define PERFMON_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef struct cpu_stats {
    int cpu0;
    int cpu1;
} cpu_stats;

typedef struct heap_stats {
    size_t heap_dram;
    int percentage_dram;
    size_t heap_external;
    int percentage_external;
} heap_stats;

typedef struct perf_stats {
    cpu_stats cpustats;
    heap_stats heapstats;
} perf_stats;

esp_err_t getStats(perf_stats * stats);

void perfmon_start(QueueHandle_t queue);

#endif