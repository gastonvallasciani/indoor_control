#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"
#include "esp_log.h"

#include "display_dogs164.h"

#define LED_PIN 18

// comandos display
static const char *TAG = "I2C";

uint8_t led_level = 0;

static esp_err_t init_led_pin(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    return ESP_OK;
}

static esp_err_t blink_led(void)
{
    led_level = !led_level;
    gpio_set_level(LED_PIN, led_level);
    return ESP_OK;
}

void app_main()
{
    bool state = false;
    init_led_pin();
    ESP_LOGI(TAG, "Inicializando I2C");
    ESP_ERROR_CHECK(set_i2c()); // inicio el i2c
    ESP_LOGI(TAG, "Inicializando DISPLAY");
    display_set_screen_full_start(67, 13, 55); // funcion de inicio de pantalla con el valor de potencia
    ESP_LOGI(TAG, "Termina inicializacion del DISPLAY");
    /*display_set_power(75, ARROW_DOWN); // funcion de ejemplo con el nuevo valor de potencia y si disminuye o aumenta
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    display_set_power(30, ARROW_DOWN);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    display_set_power(10, ARROW_DOWN);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    display_set_power(90, ARROW_UP);
    vTaskDelay(1000 / portTICK_PERIOD_MS);*/

    while (true)
    {
        ESP_LOGI("wait", "...");
        blink_led();
        vTaskDelay(3500 / portTICK_PERIOD_MS);
        if (state == false)
        {
            display_set_screen_config(14, 30, 20, 30);
            state = true;
        }
        else
        {
            display_set_screen_full_start(67, 13, 55);
            state = false;
        }
    }
}