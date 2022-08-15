
#
# Main component of TF Micro project 'person_detection_int8'.
#

idf_component_register(
    SRCS detection_responder.cc esp/image_provider.cc esp/main.cc main_functions.cc model_settings.cc esp/app_camera_esp.c person_detect_model_data.cc ../../performance_monitor/main/stats_serializer.c ../../performance_monitor/main/perfmon.c ../../performance_monitor/main/task_monitor.c ../../performance_monitor/main/wifi_sta.c ../../performance_monitor/main/http_stats_client.c
    INCLUDE_DIRS "" )



