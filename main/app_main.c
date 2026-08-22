#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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

    while(1){
        if(imu_get_data(&angx, &angy, &angz, &accx, &accy, &accz)){
            printf("%.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n", angx, angy, angz, accx, accy, accz);
#ifdef CONFIG_APP_ENABLE_OLED_OUTPUT
            printf_oled("Ang: %.1f %.1f %.1f\nAcc: %.2f %.2f %.2f", angx, angy, angz, accx, accy, accz);
#endif
        }else{
#ifdef CONFIG_APP_ENABLE_OLED_OUTPUT
            printf_oled("IMU Lendo...");
#endif
        }
        vTaskDelay(pdMS_TO_TICKS(CONFIG_APP_SAMPLE_DELAY_MS));
    }
}