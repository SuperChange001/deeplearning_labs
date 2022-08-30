/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "detection_responder.h"

#include "model_settings.h"
#include "image_tcp_client.h"
#include <cstdlib>
#include "driver/gpio.h"

static void image_to_uint8(uint8_t * dest, int8_t * src, size_t image_size) {
    for (size_t i = 0; i < image_size; i++) {
        dest[i] = (uint8_t)((int16_t)(src[i]) + 128);
    }
}

void RespondToDetection(tflite::ErrorReporter* error_reporter,
                        int8_t category_index, int8_t score, int8_t * image_data, size_t image_size) {
    TF_LITE_REPORT_ERROR(error_reporter, "Category: %s, Score: %d", kCategoryLabels[category_index], score);
    // uint8_t * img_buf = (uint8_t *)malloc(image_size);
    // image_to_uint8(img_buf, image_data, image_size);
    // image_tcp_client_send(category_index, score, img_buf, image_size);
    // free(img_buf);

    if(category_index == 0){
        // red
        gpio_set_level(GPIO_NUM_22, 0);
        gpio_set_level(GPIO_NUM_21, 1);
    }else if( category_index == 1){
        // white
        gpio_set_level(GPIO_NUM_22, 1);
        gpio_set_level(GPIO_NUM_21, 0);
    }else{
        // all off
        gpio_set_level(GPIO_NUM_22, 1);
        gpio_set_level(GPIO_NUM_21, 1);
    }

}