cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Select the article-mapped NonSecure experiment.
# 3-31 map to the U585 article demos. TrustZone topics stay in Secure-side code.
set(U585_ACTIVE_DEMO "3" CACHE STRING "Active U585 article demo")
set_property(CACHE U585_ACTIVE_DEMO PROPERTY STRINGS 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31)

# Compiler options
set(STM32_MCU_FLAGS  "-mcpu=cortex-m33 -mfpu=fpv4-sp-d16 -mfloat-abi=hard " )

#Linker options
set(STM32_LINKER_SCRIPT STM32U585xx_FLASH_ns.ld)
set(STM32_LINKER_OPTION  )

# Set the project name
set(CMAKE_PROJECT_NAME U585_NS)
project(${CMAKE_PROJECT_NAME})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Include mx-generated
include("mx-generated.cmake")


# In order to use CMake for cross-compiling
set(CMAKE_CXX_COMPILER_FORCED "true")

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    U585_ACTIVE_DEMO=${U585_ACTIVE_DEMO}
)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    App/Inc
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    App/Src/u585_app.c
    App/Src/u585_board.c
    App/Src/u585_log.c
    App/Src/u585_usart1.c
    App/Src/u585_i2c2.c
    App/Src/u585_spi2.c
    App/Src/u585_adc1.c
    App/Src/u585_dac1.c
    App/Src/u585_adf1.c
    App/Src/u585_ospi.c
    App/Src/u585_gpdma.c
    App/Src/exp03_led_button.c
    App/Src/exp04_debug_fault.c
    App/Src/exp05_clock_tree.c
    App/Src/exp06_power_supply.c
    App/Src/exp07_low_power_current.c
    App/Src/exp08_rtc_wakeup_backup.c
    App/Src/exp09_gpio_exti.c
    App/Src/exp10_tim_pwm_input_capture.c
    App/Src/exp11_uart_printf_log.c
    App/Src/exp12_i2c_sensor_bus.c
    App/Src/exp13_spi_bus.c
    App/Src/exp14_adc_dma.c
    App/Src/exp15_dac_output.c
    App/Src/exp16_gpdma_transfer.c
    App/Src/exp17_hts221_sensor.c
    App/Src/exp18_lps22hh_sensor.c
    App/Src/exp19_ism330dhcx_imu.c
    App/Src/exp20_iis2mdc_compass.c
    App/Src/exp21_vl53l5cx_tof.c
    App/Src/exp22_pdm_microphone.c
    App/Src/exp23_ospi_flash_xip.c
    App/Src/exp24_ospi_psram_cache.c
    App/Src/exp25_st25dv_nfc.c
    App/Src/exp26_usb_ucpd_device.c
    App/Src/exp27_wifi_emw3080.c
    App/Src/exp28_freertos_queue_log.c
    App/Src/exp29_mqtt_cloud.c
    App/Src/exp30_icache_fetch.c
    App/Src/exp31_rng_aes_pka.c
)

# Extra HAL modules required by Phase-B NonSecure peripherals.
target_sources(STM32_Drivers PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_uart_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rtc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_tim_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_i2c_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_spi_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_adc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_dac.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_dac_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_mdf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_ospi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_ll_dlyb.c
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined libraries
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()






