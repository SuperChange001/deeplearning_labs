#include "../image_provider.h"

#include <cstring>


TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, int8_t* image_data) {
    //Instead of initializing the camera and taking a picture, just provide a zero image
    size_t num = image_width * image_height * channels;
    memset(image_data, 0, num);
    return kTfLiteOk;
}