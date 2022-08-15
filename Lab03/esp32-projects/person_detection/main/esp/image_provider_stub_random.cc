#include "../image_provider.h"

#include <stdlib.h>

static void fill_random (int8_t* image_data, size_t num) {
    for (int i = 0; i < num; i++) {
        int r = rand() % 256;
        r = r - 128;
        image_data[i] = (int8_t) r;
    }    
}

TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, int8_t* image_data) {
    //Instead of initializing the camera and taking a picture, just provide a zero image
    size_t num = image_width * image_height * channels;
    fill_random(image_data, num);
    return kTfLiteOk;
}