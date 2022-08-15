/*Stub to prevent camera access going playing into inference time*/

#include <string.h>

#include "../image_provider.h"

TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width, int image_height, int channels, int8_t* image_data) {
    memset(image_data, 0, image_width * image_height * channels * sizeof(int8_t));
    return kTfLiteOk;
}