//Another way of approach to the Button controlled LED lighting using ESP-32
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_PIN  23
#define PUSH_BUTTON_PIN  19

void app_main(void)
{
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);   
    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);
    // int val=0;
    while(1)
    {
     if(gpio_get_level(PUSH_BUTTON_PIN)==1 )
         {
            // val=1;
              while(1)
              {
                gpio_set_level(LED_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(300));
                gpio_set_level(LED_PIN, 0);
                vTaskDelay(pdMS_TO_TICKS(300));
                if(gpio_get_level(PUSH_BUTTON_PIN)==1)
                {
                  gpio_set_level(LED_PIN, 0);
                   vTaskDelay(pdMS_TO_TICKS(300));
                  break;
                }
            }
          }
      // if(gpio_get_level(PUSH_BUTTON_PIN)==1 && val==1)
      //   {
      //     value=0;
      //       gpio_set_level(LED_PIN, 0);
      //   }
         }

        vTaskDelay(1);
    }