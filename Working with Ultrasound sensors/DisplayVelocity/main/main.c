#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "rom/ets_sys.h"

#include "ssd1306.h"
#include "ultrasonic_driver.h"
#include "cray_wifi.h"
#include "mqtt_handler.h"

int distance = 0;
float velocity;

void initDisplay(){
	ssd1306_128x64_i2c_init();
	ssd1306_setFixedFont(ssd1306xled_font6x8);
}

void sayHello(){
    char buf[200];
    
	ssd1306_clearScreen();
    sprintf(buf, "Hello");
    ssd1306_printFixedN(0,  30, buf, STYLE_NORMAL,2);
	vTaskDelay(1000/portTICK_PERIOD_MS);
	ssd1306_clearScreen();
}


void updateDisplay(void *pvParameters){
	char buf[256];

    while (true){
		ssd1306_clearScreen();
		sprintf(buf,"GROUP-8%.2f",velocity);
		ssd1306_printFixedN(0,  0, buf, STYLE_NORMAL,1);
		mqtt_driver_publish("iiitkottayam/iot", buf, strlen(buf));
		vTaskDelay(CONFIG_DISPLAY_DELAY/portTICK_PERIOD_MS);
    }
}

void countup(void *pvParameters){
    while (true){
		int tmp_distance = measure_distance_cm(GPIO_NUM_19);
		vTaskDelay(CONFIG_COUNTING_DELAY/portTICK_PERIOD_MS);
		int time = CONFIG_COUNTING_DELAY;
		velocity = abs(tmp_distance - distance)*1.0 / (time/1000.0);
		distance = tmp_distance;
    }
}


void app_main(void){
	initDisplay();
	sayHello();

	cray_wifi_init();
  cray_wifi_start();
  
  mqtt_driver_init("192.168.135.139", 1883);

	esp_log_level_set("ultrasonic_driver", ESP_LOG_NONE);

	xTaskCreate(updateDisplay, "update_display_task", 2048, NULL, 2, NULL);
	xTaskCreate(countup, "counting_task", 2048, NULL, 2, NULL);

}


