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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../main_functions.h"
#include "../wifi_ap.h"

void tf_main(void) {
  setup();
  while (true) {
    loop();
  }
}

void initLeds(){
  // White LED
  gpio_pad_select_gpio(GPIO_NUM_22);
  // Red LED
  gpio_pad_select_gpio(GPIO_NUM_21);

  gpio_set_direction(GPIO_NUM_22, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_21, GPIO_MODE_OUTPUT);
}

extern "C" void app_main() {
  initLeds();
  
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  wifi_station_init();

  xTaskCreate((TaskFunction_t)&tf_main, "tensorflow", 32 * 1024, NULL, 8, NULL);
  vTaskDelete(NULL);
}
