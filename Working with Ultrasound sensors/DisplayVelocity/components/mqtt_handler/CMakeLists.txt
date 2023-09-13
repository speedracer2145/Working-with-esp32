idf_component_register(SRCS "mqtt_handler.c"
                    INCLUDE_DIRS "." "include"
                    REQUIRES mqtt esp_wifi)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")