#include "cJSON.h"
#include "perfmon_settings.h"

#include "stats_serializer.h"


/*
static char (*serializer_funcs[2])(const PLACEHOLDER *) {
    &stats_serializer_perf_stats,
    &stats_serializer_stats_table
};

char * stats_serializer_serialize(const PLACEHOLDER * stats) {
    return serializer_funcs[stats->type](stats);
}*/


static inline void stats_serializer_add_stats_element (cJSON * data, const char * key, double value) {
        cJSON * object = cJSON_CreateObject();
        cJSON * stats = cJSON_AddArrayToObject(object, key);
        cJSON * percentage = cJSON_CreateNumber(value);
        cJSON_AddItemToArray(stats, percentage);
        cJSON_AddItemToArray(data, object);
}

char * stats_serializer_perf_stats (const perf_stats * stats) {
    cJSON * json_stats = cJSON_CreateObject();
    cJSON * data = cJSON_AddArrayToObject(json_stats, "data");
  
    stats_serializer_add_stats_element(data, "cpu0", stats->cpustats.cpu0);
    stats_serializer_add_stats_element(data, "cpu1", stats->cpustats.cpu1);
    #if PERFMON_SETTINGS_DEMO_MODE == 0
        stats_serializer_add_stats_element(data, "Heap_DRAM", stats->heapstats.heap_dram);
        #ifdef CONFIG_ESP32_SPIRAM_SUPPORT
            stats_serializer_add_stats_element(data, "Heap_External", stats->heapstats.heap_external);
        #endif
    #endif 
    stats_serializer_add_stats_element(data, "Percentage_DRAM", stats->heapstats.percentage_dram);
    #ifdef CONFIG_ESP32_SPIRAM_SUPPORT
        stats_serializer_add_stats_element(data, "Percentage_External", stats->heapstats.percentage_external);
    #endif
    char * result = cJSON_Print(json_stats);
    cJSON_Delete(json_stats);
    return result;
}

char * stats_serializer_stats_table (const task_monitor_stats_table * stats_table) {
    cJSON * json_stats_table = cJSON_CreateObject();
    cJSON * data = cJSON_AddArrayToObject(json_stats_table, "data");

    for (int i = 0; i < stats_table->size; i++) {
        char buffer[100];
        snprintf(buffer, 100, "%s_cpu_usage",stats_table->taskstats[i].pcTaskName);
        stats_serializer_add_stats_element(data, buffer , stats_table->taskstats[i].percentage_time);
        #if PERFMON_SETTINGS_DEMO_MODE == 0
            /*snprintf(buffer, 100, "%s_stack_size", stats_table->taskstats[i].pcTaskName);
            stats_serializer_add_stats_element(data, buffer,  stats_table->taskstats[i].stack_size);*/
            snprintf(buffer, 100, "%s_heap_dram", stats_table->taskstats[i].pcTaskName);
            stats_serializer_add_stats_element(data, buffer,  stats_table->taskstats[i].heap_size_internal);
            #ifdef CONFIG_ESP32_SPIRAM_SUPPORT
                snprintf(buffer, 100, "%s_heap_external", stats_table->taskstats[i].pcTaskName);
                stats_serializer_add_stats_element(data, buffer,  stats_table->taskstats[i].heap_size_external);
            #endif
        #endif
        snprintf(buffer, 100, "%s_percentage_dram", stats_table->taskstats[i].pcTaskName);
        stats_serializer_add_stats_element(data, buffer,  stats_table->taskstats[i].heap_percentage_internal);
        #ifdef CONFIG_ESP32_SPIRAM_SUPPORT
            snprintf(buffer, 100, "%s_percentage_external", stats_table->taskstats[i].pcTaskName);
            stats_serializer_add_stats_element(data, buffer,  stats_table->taskstats[i].heap_percentage_external);
        #endif
    }
    char * result = cJSON_Print(json_stats_table);
    cJSON_Delete(json_stats_table);
    return result;
}

