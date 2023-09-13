#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"

#include "esp_log.h"

#include "mqtt_handler.h"


static const char* TAG = "mqtt_handler";

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)  {
  // static int actual_len = 0;
    // your_context_t *context = event->context;Share Routing Tables
  esp_mqtt_event_handle_t event = event_data;
  switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
		  ESP_LOGI(TAG,"MQTT_EVENT_CONNECTED\n");
		  // msg_id = esp_mqtt_client_subscribe(event->client, "tum/caps/#", 1);
      // printf("sent subscribe successful, msg_id=%d\n", msg_id);
      break;

    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(TAG,"MQTT_EVENT_DISCONNECTED\n");
      break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG,"MQTT_EVENT_SUBSCRIBED, msg_id=%d\n", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG,"MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG,"MQTT_EVENT_PUBLISHED, msg_id=%d\n", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG,"MQTT_EVENT_DATA\n");
        ESP_LOGI(TAG,"TOPIC=%.*s\r\n", event->topic_len, event->topic);
        ESP_LOGI(TAG,"DATA=%.*s\r\n", event->data_len, event->data);
        ESP_LOGI(TAG,"ID=%d, total_len=%d, data_len=%d, current_data_offset=%d\n", event->msg_id, event->total_data_len, event->data_len, event->current_data_offset);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG,"MQTT_EVENT_ERROR\n");
        break;

	  case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGI(TAG,"MQTT_EVENT_BEFORE_CONNECT\n");
        break;

	  default:
        ESP_LOGI(TAG,"Other event id:%d\n", (esp_mqtt_event_id_t)event_id);
        break;
    }
}

static esp_mqtt_client_handle_t mqtt_client = NULL; 

void mqtt_driver_init(const char* host, const int port) {
  esp_mqtt_client_config_t cfg = {
    .broker.address = {
      .hostname = host,
      .port = port,
      .transport = MQTT_TRANSPORT_OVER_TCP,
      
    },
    .session.protocol_ver = MQTT_PROTOCOL_V_3_1_1,
  };

  mqtt_client = esp_mqtt_client_init(&cfg);
  esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
  esp_mqtt_client_start(mqtt_client);
}

void mqtt_driver_publish(const char* topic, const char* message, const int size) {
  while(esp_mqtt_client_publish(mqtt_client, topic, message, size, 1, 0) < 0);
}