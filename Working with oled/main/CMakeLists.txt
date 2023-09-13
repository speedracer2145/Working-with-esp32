idf_component_register(SRCS "main.c" 
                    INCLUDE_DIRS "." 
                    REQUIRES driver ssd1306 ultrasonic_driver)