idf_component_register(SRCS "ledc_basic_example_main.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_ledc esp_driver_gptimer esp_rom esp_timer)
