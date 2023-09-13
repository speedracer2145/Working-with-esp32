//Program to blink the LED on ESP32 DevKitC by using a push button
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#define LED_PIN 22
#define PUSH_BUTTON_PIN 19
#include "main.h"
#include "blink_fn.h"

void app_main(void) {
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);   
    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);
    int val=0,mul=1;;
          /*
            if(gpio_get_level(PUSH_BUTTON_PIN)==0){
              val=1;
            }
            if(gpio_get_level(PUSH_BUTTON_PIN) == 1 && val==1)
            {
              val=0;
              break;
            }
            */
           while(1)
           {
            mul=1;
            if(gpio_get_level(PUSH_BUTTON_PIN)==1 && val==0)
            {
              val=1;
                                    vTaskDelay(pdMS_TO_TICKS(300));
            }
            else if (gpio_get_level(PUSH_BUTTON_PIN) == 1 && val==1)
            {
                                   vTaskDelay(pdMS_TO_TICKS(300));
              val=0;
            }

            
              while(val==1)
              {
                mul++;
                    if(gpio_get_level(PUSH_BUTTON_PIN)==1 && val==0)
                    {
                      val=1;
                      vTaskDelay(pdMS_TO_TICKS(300));
                    }
                    else if (gpio_get_level(PUSH_BUTTON_PIN) == 1 && val==1)
                    {
                      val=0;
                      vTaskDelay(pdMS_TO_TICKS(300));
                    }
                      gpio_set_level(LED_PIN, 1);
                      vTaskDelay(pdMS_TO_TICKS(mul*100));
                      gpio_set_level(LED_PIN, 0);
                      vTaskDelay(pdMS_TO_TICKS(mul*100));
              }
           }
}







    // if (gpio_get_level(PUSH_BUTTON_PIN) == 1){
    //       c++;

    //     if(c==2)
    //     {
    //       gpio_set_level(LED_PIN, 0);
    //          vTaskDelay(pdMS_TO_TICKS(1000));
    //          c=0;
    //     }
          
    //     }

    //     vTaskDelay(1);
    // }


/*

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "main.h"
#include "blink_fn.h"

void app_main(void) {
  blink_init_gpio();
  xTaskCreate(blink_start_blink, "blink_start_blink", 2048, NULL, 5, NULL);

  while(1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}




#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#define LED_PIN 22
#define PUSH_BUTTON_PIN 19
#include "main.h"
#include "blink_fn.h"

void app_main(void) {
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);   
    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);
int val=0;
    while(1) {       
        if (gpio_get_level(PUSH_BUTTON_PIN) == 1){
          if(val==0){
            val=1;
          }
          else{
            val=0;
          }
          vTaskDelay(pdMS_TO_TICKS(1000));
          while(val==1)
          {
            if(gpio_get_level(PUSH_BUTTON_PIN) == 1)
            {
              val=0;
              break;
            }
            gpio_set_level(LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
          }
          
        } 
        // else
        // {
        //     gpio_set_level(LED_PIN, 0);
        //     vTaskDelay(pdMS_TO_TICKS(1000));        
        // }

        vTaskDelay(1);
    }
}
*/