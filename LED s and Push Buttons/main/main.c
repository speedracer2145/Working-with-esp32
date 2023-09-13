// Program to show the priority of tasks in ESP-32
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "main.h"
#include "blink_fn.h"


void countSerial(void* pvParameters){
  int count = 0;
  while(1){
  printf("%d\n", count++);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  }

}

void block(void *pvParameters){
  //int count = 0;
  while(true){
    // count++;
    // printf("Hello %d\n", count);
    // vTaskDelay(200/portTICK_PERIOD_MS);
  }
}

void app_main(void) {
  //blink_init_gpio();
  // 5 is priority. max is 16. appmain is 0, lowest. RTOS makes program part of OS instead of on top.
  // xTaskCreate(blink_start_blink, "blink_start_blink", 2048, NULL, 5, NULL);

  // while(1) {
  //   vTaskDelay(pdMS_TO_TICKS(1000));
  // }
    xTaskCreatePinnedToCore(block, "blocking_task", 2048, NULL, 0, NULL,1);
    xTaskCreatePinnedToCore(countSerial, "counting_task", 2048, NULL, 2, NULL,0);

}