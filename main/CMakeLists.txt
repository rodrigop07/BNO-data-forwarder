cmake_minimum_required(VERSION 3.5)
file(GLOB SRCS_C "*.c")
file(GLOB SRCS_CPP "*.cpp")
idf_component_register(SRCS
                    ${SRCS_C}
                    ${SRCS_CPP}
                    INCLUDE_DIRS
                    "."
                    REQUIRES driver esp_driver_gpio esp_driver_i2c
                    PRIV_REQUIRES i2c_config oled_printf oled_setup rinku404__bno085)