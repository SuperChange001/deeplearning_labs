#ifndef HTTP_STATS_CLIENT_H
#define HTTP_STATS_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*Use of filter:
    1. Put task names you want stats for into statically allocated array, for example:

        static const char * filter[] = {"spin0", "spin1", "heap", "wifi"};

    2. Calculate the filter length by:

        size_t filter_length = sizeof(filter)/sizeof(char *);

    3. Pass filter and length as arguments for the task monitor
    
    In case you dont want to filter, just pass NULL for the filter parameter.
*/

void http_stats_client_perfmon_start(void);
void http_stats_client_task_monitor_start(const char ** filter, size_t length);

#ifdef __cplusplus
}
#endif

#endif