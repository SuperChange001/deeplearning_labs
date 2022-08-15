#include "mnist_detection_responder.h"

#include "mnist_tcp_client.h"
#include <cstdlib>

static void image_to_uint8(uint8_t * dest, int8_t * src, size_t image_size) {
    for (size_t i = 0; i < image_size; i++) {
        dest[i] = (uint8_t)((int16_t)(src[i]) + 128);
    }
}

void RespondToDetection(tflite::ErrorReporter* error_reporter,
                        const char * category, int8_t score, int8_t * image_data, size_t image_size) {
    TF_LITE_REPORT_ERROR(error_reporter, "Category: %s, Score: %d", category, score);
    uint8_t * img_buf = (uint8_t *)malloc(image_size);
    image_to_uint8(img_buf, image_data, image_size);
    mnist_tcp_client_send(img_buf, image_size);
    free(img_buf);
}