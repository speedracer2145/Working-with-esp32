#ifndef ULTRASONIC_H
#define ULTRASONIC_H

enum {
  GPIO_LOW = 0,
  GPIO_HIGH = 1,
};

/**
 * @brief Measures the distance to an object using
 *        an ultrasonic sensor
 *
 * @param GPIO_NUM  GPIO pin number where the snesor is connected
 * @return long     Distance to the object in centimeters
 */
long measure_distance_cm(int GPIO_NUM);

#endif