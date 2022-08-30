
#
# Main component of TF Micro project 'person_detection_int8'.
#

idf_component_register(
    SRCS detection_responder.cc esp/image_provider.cc esp/main.cc main_functions.cc model_settings.cc esp/app_camera_esp.c person_detect_model_data.cc
    INCLUDE_DIRS "" )



