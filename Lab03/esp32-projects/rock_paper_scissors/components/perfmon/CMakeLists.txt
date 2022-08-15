idf_component_register(SRCS  "stats_serializer.c" "task_monitor.c" "perfmon.c" "http_stats_client.c"
                    INCLUDE_DIRS .
                    REQUIRES esp_http_client json nvs_flash)
