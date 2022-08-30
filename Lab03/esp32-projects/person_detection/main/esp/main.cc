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

#include "../main_functions.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "driver/gpio.h"
static const char * filter[] = {"tensorflow", "wifi", "tiT", "http_request", "IDLE0", "IDLE1"};

void tf_main(void) {
  setup();
  while (true) {
    int64_t start_time = esp_timer_get_time();
    loop();
    int64_t end_time = esp_timer_get_time();
    printf("Time elapsed: %lld\n", end_time - start_time);
    //Prevent watchdog time to trigger for IDLE0
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

extern "C" void app_main() {
  // White LED
  gpio_pad_select_gpio(GPIO_NUM_22);
  // Red LED
  gpio_pad_select_gpio(GPIO_NUM_21);

  gpio_set_direction(GPIO_NUM_22, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_21, GPIO_MODE_OUTPUT);


  xTaskCreate((TaskFunction_t)&tf_main, "tensorflow", 32 * 1024, NULL, 8, NULL);
  size_t filter_len = sizeof(filter)/sizeof(char*);
  //http_stats_client_perfmon_start();
  // http_stats_client_task_monitor_start(filter, filter_len);
  vTaskDelete(NULL);
}
