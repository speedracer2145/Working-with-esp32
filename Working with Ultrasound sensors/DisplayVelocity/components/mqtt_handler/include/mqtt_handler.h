#ifndef __MQTT_HANDLER_H__
#define __MQTT_HANDLER_H__

void mqtt_driver_init(const char* host, const int port);

void mqtt_driver_publish(const char* topic, const char* message, const int size);

#endif