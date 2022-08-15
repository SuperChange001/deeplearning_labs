
#
# Main component of TF Micro project 'person_detection_int8'.
#

idf_component_register(
    SRCS detection_responder.cc esp/image_provider.cc esp/wifi_sta.c esp/main.cc main_functions.cc model_settings.cc esp/app_camera_esp.c rps_quant.cc image_tcp_client.c
    INCLUDE_DIRS "" )



