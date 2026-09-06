#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include <inttypes.h>
#include <stdio.h>

#include "i2c_config.h"
#include "imu_config.h"
#include "oled_printf.h"
#include "oled_setup.h"
#include "nvs_flash.h"

static const char TAG[] = "main";
extern lv_disp_t *local_disp;

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    enable_vext_rail(); // rail de energia da placa, uma vez, antes de qualquer I2C

    i2c_master_bus_handle_t i2c_bus = NULL;

    initialize_i2c(&i2c_bus, PIN_NUM_SDA, PIN_NUM_SCL); // barramento do OLED

    configure_oled_screen(&i2c_bus);

    oled_printf_init(local_disp);

    imu_config_init(NULL); // sobe seu proprio barramento (I2C_NUM_1), ver imu_config.c

    ESP_LOGI(TAG, "Enter in the main loop...");

    float angx, angy, angz;
    float accx, accy, accz;

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << GPIO_NUM_35),
        .pull_down_en = 0,
        .pull_up_en = 1
    };
    gpio_config(&io_conf);
    
    bool use_global_accel = false;
    int last_button_state = 1;

    while(1){
        int button_state = gpio_get_level(GPIO_NUM_35);
        if(button_state == 0 && last_button_state == 1) {
            use_global_accel = !use_global_accel;
        }
        last_button_state = button_state;

        if(imu_get_data(&angx, &angy, &angz, &accx, &accy, &accz)){
            if (use_global_accel) {
                imu_get_global_accel(&accx, &accy, &accz);
            }
            printf("%s: %.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n", use_global_accel ? "Global" : "Linear", angx, angy, angz, accx, accy, accz);
#ifdef CONFIG_APP_ENABLE_OLED_OUTPUT
            printf_oled("Ang: %.1f %.1f %.1f\n%s: %.2f %.2f %.2f", angx, angy, angz, use_global_accel ? "GLB" : "LIN", accx, accy, accz);
#endif
        }else{
#ifdef CONFIG_APP_ENABLE_OLED_OUTPUT
            printf_oled("IMU Lendo...");
#endif
        }
        vTaskDelay(pdMS_TO_TICKS(CONFIG_APP_SAMPLE_DELAY_MS));
    }
}