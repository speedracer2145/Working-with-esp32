#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "ultrasonic_driver.h"

static const char *TAG = "ultrasonic_driver";

int _echo_count_max = 0;
int _echo_noend_max = 0;

#define ROUNDTRIP 58
#define TRIGGER_LOW_DELAY 4
#define TRIGGER_HIGH_DELAY 12
#define REPETITIONS 5


static uint32_t get_time_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec;
}

#define TIMEOUT_EXPIRED(start, len) ((get_time_us() - (start)) >= (len))

uint64_t my_micros()
{
  uint64_t cycles = xthal_get_ccount();
  uint64_t m = (cycles / ets_get_cpu_frequency());
  return (m);
}

uint64_t micros_diff(uint64_t begin, uint64_t end)
{
  return end - begin;
}

void delay_microseconds(int delay)
{
  uint64_t start = esp_timer_get_time();
  uint64_t end = start + delay;

  while (esp_timer_get_time() < end);
}

uint64_t get_echo(uint32_t pin, uint32_t timeout)
{
  uint64_t begin = esp_timer_get_time();

  // wait for any previous pulse to end
  while (gpio_get_level(pin))
  {
    if (TIMEOUT_EXPIRED(begin, timeout + begin))
    {
      ESP_LOGW(TAG, "Continuously high");
      return 1;
    }
  }

  // wait for the pulse to start
  while (!gpio_get_level(pin))
  {
    if (TIMEOUT_EXPIRED(begin, timeout))
    {
      if (_echo_count_max < 5)
      {
        ESP_LOGW(TAG, "Echo did not start");
        _echo_count_max++;
      }
      return 2;
    }
  }

  uint64_t pulseBegin = esp_timer_get_time();
  _echo_count_max = 0;
  // wait for the pulse to stop
  while (gpio_get_level(pin))
  {
    if (TIMEOUT_EXPIRED(begin, timeout + begin))
    {
      if (_echo_noend_max < 5)
      {
        ESP_LOGW(TAG, "Echo did not end");
        _echo_noend_max++;
      }
      return 3;
    }
  }
  uint64_t pulseEnd = esp_timer_get_time();
  _echo_noend_max = 0;
  //ESP_LOGW(TAG, "micros diff = %lld", micros_diff(pulseBegin, pulseEnd));

  return micros_diff(pulseBegin, pulseEnd);
}

long minDist;
long maxDist;
long avg_dist = 0, range = 0;

/*The measured distance from the range 0 to 400 Centimeters*/
// long measure_distance_cm(int gpio_pin)
// {
//   minDist = 600;
//   maxDist = 0;
//   gpio_set_level(gpio_pin, 0);

//   uint32_t roundtrip = 100000L;

//   ESP_ERROR_CHECK( gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT) );
//   ESP_ERROR_CHECK( gpio_pullup_en(gpio_pin) );
//   gpio_set_level(gpio_pin, 0);
//   ets_delay_us(TRIGGER_LOW_DELAY);
//   gpio_set_level(gpio_pin, 1);
//   ets_delay_us(TRIGGER_HIGH_DELAY);
//   gpio_set_level(gpio_pin, 0);
//   ESP_ERROR_CHECK( gpio_set_direction(gpio_pin, GPIO_MODE_INPUT) );
//   ESP_ERROR_CHECK( gpio_pullup_en(gpio_pin) );

//   for(int i = 0; i < REPETITIONS; ++i){

//     uint32_t begin = get_time_us();
//     while(gpio_get_level(gpio_pin))
//     {
//       if (TIMEOUT_EXPIRED(begin, roundtrip + begin))
//       {
//         ESP_LOGE(TAG, "Continuously high");
//         return ESP_ERR_INVALID_STATE;
//       }
//     }
      
//       uint32_t start = get_time_us();
//     while(!gpio_get_level(gpio_pin))
//     {
//       if (TIMEOUT_EXPIRED(start, 6000))
//       {
//         ESP_LOGE(TAG, "Echo did not start");
//         return ESP_ERR_INVALID_STATE;
//       }
//       }

//     while(gpio_get_level(gpio_pin))
//     {
//       if (TIMEOUT_EXPIRED(start, roundtrip + start))
//       {
//         ESP_LOGE(TAG, "Echo did not end");
//         return ESP_ERR_INVALID_STATE;
//       }
//         }

//     uint32_t end;
//     while((end = get_time_us()) == 0);
//     range = (end-start) / 29 / 2;
//     if (range > 5 && range < 600) {
//       minDist = minDist > range ? range : minDist;
//       maxDist = maxDist < range ? range : maxDist;
//       avg_dist += range;
//     }

//     if (minDist == 600 && maxDist == 0) {
//       minDist = 0; maxDist = 600;
//     } 
    
//   }
//   return avg_dist/REPETITIONS;
// }

long measure_distance_cm(int gpio_pin) {
// esp_err_t ultrasonic_measure_cm(const ultrasonic_t *dev, long *distance) {
  minDist = 600;
  maxDist = 0;
  int not_finished = 0, real_repetitions = 0;
  uint32_t roundtrip = 100000L/**dev->max_distance*ROUNDTRIP*/;
  long avg_dist = 0, range = 0;
  uint64_t duration;

  for(int i = 0; i < REPETITIONS; i++) {
    ESP_ERROR_CHECK( gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT) );
    ESP_ERROR_CHECK( gpio_pullup_en(gpio_pin) );
    gpio_set_level(gpio_pin, GPIO_LOW);
    ets_delay_us(TRIGGER_LOW_DELAY);
    gpio_set_level(gpio_pin, GPIO_HIGH);
    ets_delay_us(TRIGGER_HIGH_DELAY);
    gpio_set_level(gpio_pin, GPIO_LOW);
    ets_delay_us(TRIGGER_LOW_DELAY);
    ESP_ERROR_CHECK( gpio_set_direction(gpio_pin, GPIO_MODE_INPUT) );
    ESP_ERROR_CHECK( gpio_pullup_en(gpio_pin) );

    uint32_t begin;
    while((begin = get_time_us()) == 0);
    while(gpio_get_level(gpio_pin))
    {
      if (TIMEOUT_EXPIRED(begin, roundtrip))
      {
        ESP_LOGE(TAG, "Continuously high");
        not_finished = 1;
        break;
      }
    }
    
    uint32_t start;
    while((start = get_time_us()) == 0);
    while(!gpio_get_level(gpio_pin))
    {
      if (TIMEOUT_EXPIRED(start, ROUNDTRIP*600+start))
      {
        ESP_LOGE(TAG, "Echo did not start");
        not_finished = 1;
        break;
      }
    }

    while(gpio_get_level(gpio_pin))
    {
      if (TIMEOUT_EXPIRED(start, ROUNDTRIP*600+start))
      {
        ESP_LOGE(TAG, "Echo did not end");
        not_finished = 1;
        break;
        
      }
    }
    uint32_t end;
    while((end = get_time_us()) == 0);
    range = not_finished == 1 ? 0 : (end-start) * (0.034/2) ;
    if (range > 5 && range < 700) {
      minDist = minDist > range ? range : minDist;
      maxDist = maxDist < range ? range : maxDist;
      avg_dist += range;
      real_repetitions++;
    }

    if (minDist == 600 && maxDist == 0) {
      minDist = 0; maxDist = 600;
    } 
    delay_microseconds(700);
  }

  // printf("min: %ld  max: %ld, avg: %ld\n", minDist, maxDist, real_repetitions > 0 ? avg_dist / real_repetitions: avg_dist);
  return real_repetitions > 0 ? avg_dist / real_repetitions: avg_dist;
}