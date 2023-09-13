#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "rom/ets_sys.h"
#include<math.h>
#include "ssd1306.h"
#include "ultrasonic_driver.h"

int count,d1=0,d2=0,d=0;
float time=0,v=0;

void initDisplay(){
	ssd1306_128x64_i2c_init();
	ssd1306_setFixedFont(ssd1306xled_font6x8);
}

void sayHello(){
    char buf[200];
    
	ssd1306_clearScreen();
    sprintf(buf, "Hello");
    ssd1306_printFixedN(0,  30, buf, STYLE_NORMAL,1);
	vTaskDelay(CONFIG_COUNTING_DELAY/portTICK_PERIOD_MS);
	ssd1306_clearScreen();
}


void updateDisplay(void *pvParameters){
	char buf[256];

    while (true){
		ssd1306_clearScreen();
		sprintf(buf,"%.2f",v);
		ssd1306_printFixedN(0,  30, buf, STYLE_NORMAL,1);
		vTaskDelay(CONFIG_COUNTING_DELAY/portTICK_PERIOD_MS);
    }
}

void countup(void *pvParameters){
    while (true){
		time=CONFIG_COUNTING_DELAY;
		d2=measure_distance_cm(18);
        v=(abs(d2-d1))/(time/1000);
		d1=d2;
		printf("Speed: %.2f cm/s\n",v);
		vTaskDelay(CONFIG_COUNTING_DELAY/portTICK_PERIOD_MS);
    }
}


void app_main(void){     
	initDisplay();
	sayHello();
	xTaskCreate(updateDisplay, "update_display_task", 2048, NULL, 2, NULL);
	xTaskCreate(countup, "counting_task", 2048, NULL, 2, NULL);
   
	
}


