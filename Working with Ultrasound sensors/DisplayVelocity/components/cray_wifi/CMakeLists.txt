idf_component_register(SRCS "cray_wifi.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_wifi nvs_flash)