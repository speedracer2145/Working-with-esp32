#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "ssd1306.h"
#include "esp_system.h"
#include "rom/ets_sys.h"
#include "main.h"
#include "blink_fn.h"
#include "driver/gpio.h"
#define PUSH_BUTTON_PIN 19

int count;
void initDisplay(){
	ssd1306_128x64_i2c_init();
	ssd1306_setFixedFont(ssd1306xled_font5x7);
}

void sayHello(){
    char buf[200];
    
	ssd1306_clearScreen();
  sprintf(buf, "ALOK");
  ssd1306_printFixedN(0,  30, buf, STYLE_NORMAL,2);
	vTaskDelay(1000/portTICK_PERIOD_MS);
	ssd1306_clearScreen();
}


void countup(void *pvParameters){
	 gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);
	//  gpio_set_pull_mode(PUSH_BUTTON_PIN, GPIO_PULLUP_ONLY);
	  char buf[200];
    while (true){
      if(gpio_get_level(PUSH_BUTTON_PIN)==1)
		{
			ssd1306_clearScreen();
		count=count+1;
		sprintf(buf, "%d", count);
		ssd1306_printFixedN(0,  10, buf, STYLE_NORMAL,2);
		vTaskDelay(200/portTICK_PERIOD_MS);
			if(count>=10 && count<20)
	       {
	       
			ssd1306_clearScreen();
	        sprintf(buf, "STOP IT");
	        ssd1306_printFixedN(0,  30, buf, STYLE_NORMAL,2);
	        // vTaskDelay(500/portTICK_PERIOD_MS);	       
		   }
		   else if(count>=20)
	       {
			ssd1306_clearScreen();
	        sprintf(buf, "ERROR!!!");
	        ssd1306_printFixedN(0,  30, buf, STYLE_NORMAL,2);
			vTaskDelay(3000/portTICK_PERIOD_MS);
	         break;
		}
    }
}
}


void app_main(void){
	initDisplay();
	// sayHello();
	xTaskCreate(countup, "counting_task", 2048, NULL, 2, NULL);

}


