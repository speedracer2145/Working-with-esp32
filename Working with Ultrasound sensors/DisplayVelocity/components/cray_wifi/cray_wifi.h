#ifndef CRAY_WIFI_H
#define CRAY_WIFI_H

#include "esp_err.h"

typedef enum {
  CRAY_WIFI_FINISH,
  CRAY_WIFI_DISCONNECT_ONLY,
} cray_wifi_t;

esp_err_t cray_wifi_init();
esp_err_t cray_wifi_start();

esp_err_t cray_wifi_stop(cray_wifi_t action);

esp_err_t unregister_event_handler();

esp_err_t register_event_handler();
#endif