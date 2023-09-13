#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "cray_wifi.h"

#define WIFI_CONNECTED BIT0
#define WIFI_FAILED    BIT1

static const char* TAG = "cray_wifi";
static int wifi_retry_index = 0; 

static EventGroupHandle_t wifi_event_group_handle;
SemaphoreHandle_t cray_wifi_semaphr = NULL;
static int cray_wifi_initialized = 0;
static int cray_wifi_connected = 0;

wifi_config_t wifi_config;

esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;

wifi_config_t initialize_wifi_config()
{
  ESP_LOGI(TAG, "ESP32 Connecting to [%s]", CONFIG_ESP_WiFi_SSID);
  wifi_config_t ret = {
    .sta = {
      .ssid = CONFIG_ESP_WiFi_SSID,
      .password = CONFIG_ESP_WiFi_PASSWORD,
      .listen_interval = 1,
      .threshold.authmode = WIFI_AUTH_WPA2_PSK,
      .pmf_cfg = {
        .capable = true,
        .required = false
      },
    },
  };

  return ret;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
  if((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_START))
    esp_wifi_connect();
  else if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_DISCONNECTED))
  {
    if(wifi_retry_index < CONFIG_ESP_WiFi_MAX_RETRY)
    {
      esp_wifi_connect();
      wifi_retry_index++;
      ESP_LOGW(TAG,"Retrying to connect...");
    }
    else
      xEventGroupSetBits(wifi_event_group_handle, WIFI_FAILED);
    
    ESP_LOGE(TAG, "Failed to connect to WiFi AP...");
  }
  else if ((event_base == IP_EVENT) && (event_id == IP_EVENT_STA_GOT_IP))
  {
    ip_event_got_ip_t* event =  (ip_event_got_ip_t *) event_data;
    ESP_LOGI(TAG, "ESP32's IP=" IPSTR, IP2STR(&event->ip_info.ip));
    wifi_retry_index = 0;
    xEventGroupSetBits(wifi_event_group_handle, WIFI_CONNECTED);
  }
}

static esp_err_t initialize_wifi_struct()
{
  esp_err_t ret = ESP_OK;
  wifi_event_group_handle = xEventGroupCreate();
  ret = esp_netif_init();
  if(ret)
  {
    ESP_LOGI(TAG, "esp_netif_init() ret=[%d]", ret);
    return ret;
  }

  ret = esp_event_loop_create_default();
  if(ret)
  {
    ESP_LOGI(TAG, "esp_event_loop_create_default() ret=[%d]", ret);
    return ret;
  }

  esp_netif_create_default_wifi_sta();

  wifi_config = initialize_wifi_config();
  return ret;
}

static esp_err_t cray_wifi_connect_wait()
{
  EventBits_t bits = xEventGroupWaitBits(wifi_event_group_handle,
                                        WIFI_CONNECTED | WIFI_FAILED,
                                        pdFALSE, pdFALSE,
                                        portMAX_DELAY);
  
  if(bits & WIFI_CONNECTED)
  {
    ESP_LOGI(TAG, "Successfully connected to [%s]", wifi_config.sta.ssid);
    return ESP_OK;
  }
  else if(bits & WIFI_FAILED)
  {
    ESP_LOGI(TAG, "Cannot connect to [%s]", wifi_config.sta.ssid);
    return ESP_FAIL;
  }
  else
  {
    ESP_LOGI(TAG, "Unhandled event");
    return ESP_FAIL;
  }
} 

static esp_err_t cray_wifi_connect()
{
  esp_err_t ret = ESP_OK;
  ret = esp_wifi_start();
  if(ret)
  {
    ESP_LOGI(TAG, "esp_wifi_start() ret=[%d]", ret);
    return ret;
  }

  ret = cray_wifi_connect_wait();
  if(ret)
  {
    ESP_LOGI(TAG, "cray_wifi_connect_wait() ret=[%d]", ret);
    return ret;
  }
  return ret;
}

esp_err_t cray_wifi_start()
{
  esp_err_t ret = ESP_OK;
  
  xSemaphoreTake(cray_wifi_semaphr, portMAX_DELAY);
  if(!cray_wifi_initialized && !cray_wifi_connected)
  {
    ret = initialize_wifi_struct();
    if(ret)
    {
      xSemaphoreGive(cray_wifi_semaphr);
      return ret;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if(ret)
    {
      xSemaphoreGive(cray_wifi_semaphr);
      ESP_LOGI(TAG, "esp_wifi_init() ret=[%d]", ret);
      return ret;
    }

    ret = register_event_handler();
    if(ret)
    {
      xSemaphoreGive(cray_wifi_semaphr);
      ESP_LOGI(TAG, "register_event_handler() ret=[%d]", ret);
      return ret;
    }

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if(ret)
    {
      xSemaphoreGive(cray_wifi_semaphr);
      ESP_LOGI(TAG, "esp_wifi_set_mode(WIFI_MODE_STA) ret=[%d]", ret);
      return ret;
    }

    ret = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    if(ret)
    {
      xSemaphoreGive(cray_wifi_semaphr);
      ESP_LOGI(TAG, "esp_wifi_set_config() ret=[%d]", ret);
      return ret;
    }
    cray_wifi_initialized = 1;

    ret = cray_wifi_connect();
    if(ret)
    { 
      xSemaphoreGive(cray_wifi_semaphr);
      ESP_LOGI(TAG, "cray_wifi_start() ret=[%d]", ret);
      return ret;
    }

    cray_wifi_connected = 1;
    xSemaphoreGive(cray_wifi_semaphr);   
  }
  else if (cray_wifi_initialized && !cray_wifi_connected)
  {
    ret = esp_wifi_connect();
    if(ret)
    {
      xSemaphoreGive(cray_wifi_semaphr);
      ESP_LOGI(TAG, "esp_wifi_connect() ret=[%d]", ret);
      return ret;
    }

    ret = cray_wifi_connect_wait();
    if(ret)
    {
      xSemaphoreGive(cray_wifi_semaphr);
      ESP_LOGI(TAG, "cray_wifi_connect_wait() ret=[%d]", ret);
      return ret;
    }
    cray_wifi_connected = 1;
    xSemaphoreGive(cray_wifi_semaphr);
  }
  else if(!cray_wifi_initialized && cray_wifi_connected)
  {
    ESP_LOGI(TAG, "Error behavior... Defaulting...");
    cray_wifi_initialized = 0;
    cray_wifi_connected = 0;
    xSemaphoreGive(cray_wifi_semaphr);
    return ESP_OK;
  }
  else if(cray_wifi_initialized && cray_wifi_connected)
  {
    xSemaphoreGive(cray_wifi_semaphr);
    ESP_LOGI(TAG, "You are connected to the internet... Enjoy your day!!!");
    return ESP_OK;
  }
  else
  {
    xSemaphoreGive(cray_wifi_semaphr);
    ESP_LOGI(TAG, "Unhandled case. You should never be here!!!");
    return ESP_FAIL;
  }
  xSemaphoreGive(cray_wifi_semaphr);
  return ret;
}
esp_err_t cray_wifi_stop(cray_wifi_t action)
{
  esp_err_t ret = ESP_OK;
  if(cray_wifi_semaphr != NULL)
    xSemaphoreTake(cray_wifi_semaphr, portMAX_DELAY);
  else
    return ESP_OK;
    
  if(action == CRAY_WIFI_DISCONNECT_ONLY)
  {
    #if !CONFIG_ESP_WiFi_DISCONNECT_ONLY_DISABLE
      if(cray_wifi_initialized && !cray_wifi_connected)
      {
        // Do nothing...
        xSemaphoreGive(cray_wifi_semaphr);
        return ESP_OK;
      }
      else if(cray_wifi_initialized && cray_wifi_connected)
      {
        ret = esp_wifi_disconnect();
        if(ret)
        {
          xSemaphoreGive(cray_wifi_semaphr);
          ESP_LOGI(TAG, "esp_wifi_disconnect() ret=[%d]", ret);
          return ret;
        }
        cray_wifi_connected = 0;
        xSemaphoreGive(cray_wifi_semaphr);
      }
    #else
      ESP_LOGW(TAG, "WiFi is not disconnected per requested...");
    #endif
  }
  else if(action == CRAY_WIFI_FINISH)
  {
    if(cray_wifi_initialized)
    {
      ret = unregister_event_handler();
      if(ret)
      {
        xSemaphoreGive(cray_wifi_semaphr);
        ESP_LOGI(TAG, "unregister_event_handler() ret=[%d]", ret);
        return ret;
      }

      ret = esp_wifi_stop();
      if(ret)
       {
         xSemaphoreGive(cray_wifi_semaphr);
         ESP_LOGI(TAG, "esp_wifi_stop() ret=[%d]", ret);
         return ret;
       }
      cray_wifi_initialized = 0;
      cray_wifi_connected = 0;
      xSemaphoreGive(cray_wifi_semaphr);
    }
    else if(!cray_wifi_initialized)
    {
      // Do nothing...
      xSemaphoreGive(cray_wifi_semaphr);
      return ESP_OK;
    }
  }
  xSemaphoreGive(cray_wifi_semaphr);
  return ret;
}

esp_err_t unregister_event_handler()
{
  esp_err_t ret;
  ret = esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip);
  ret = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id);
  vEventGroupDelete(wifi_event_group_handle);
  return ret;
}

esp_err_t register_event_handler()
{
  esp_err_t ret;
  ret = esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id);
  ret = esp_event_handler_instance_register(IP_EVENT,
                                                      IP_EVENT_STA_GOT_IP,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      &instance_got_ip);
  
  return ret;
}

esp_err_t cray_wifi_init()
{
  esp_err_t ret = ESP_OK; 
  
  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ret =  nvs_flash_erase();
    nvs_flash_init();
  }
  
  cray_wifi_semaphr = xSemaphoreCreateMutex();
  if(cray_wifi_semaphr != NULL)
    return ret;
  return ESP_FAIL;
}
