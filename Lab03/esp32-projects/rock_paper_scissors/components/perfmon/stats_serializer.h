#ifndef STATS_SERIALIZER_H
#define STATS_SERIALIZER_H

#include "task_monitor.h"
#include "perfmon.h"

char * stats_serializer_perf_stats(const perf_stats * stats);
char * stats_serializer_stats_table (const task_monitor_stats_table * stats_table);

#endif