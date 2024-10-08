//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sdkconfig.h"


#include "esp_log.h"

#include "../include/board_def.h"
#include "../include/display_manager.h"
#include "../include/display_dogs164.h"

static const char *TAG = "I2C";
//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define DEBUG_MODULE 1

#define QUEUE_ELEMENT_QUANTITY 25
//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------
typedef enum{
    CMD_UNDEFINED = 0,
    START_DISPLAY = 1,
    UPDATE_DISPLAY = 2
} display_event_cmds_t;

typedef struct{
    uint8_t pwm_value;
    display_event_cmds_t cmd;
    arrow_t arrow_orientation;
} display_event_t;
//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------
static QueueHandle_t display_manager_queue;

//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
static void display_manager_task(void *arg)
{
    display_event_t display_ev;

    while (true)
    {
        if (xQueueReceive(display_manager_queue, &display_ev, portMAX_DELAY) == pdTRUE)
        {
            switch (display_ev.cmd)
            {
            case CMD_UNDEFINED:
                break;
            case START_DISPLAY:
                display_set_screen(display_ev.pwm_value);
                break;
            case UPDATE_DISPLAY:
                display_set_power(display_ev.pwm_value, display_ev.arrow_orientation);
                break;
            default:
                break;
            }
        }
    }
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void display_manager_init(void)
{
    set_i2c(); 
    ESP_LOGI(TAG, "DISPLAY inicializado");

    display_manager_queue = xQueueCreate(QUEUE_ELEMENT_QUANTITY, sizeof(display_event_t));

    xTaskCreate(display_manager_task, "display_manager_task", configMINIMAL_STACK_SIZE * 10,
        NULL, configMAX_PRIORITIES - 2, NULL);

}
//------------------------------------------------------------------------------
void display_manager_start(uint8_t pwm_value)
{
    display_event_t display_ev;

    display_ev.cmd = START_DISPLAY;
    display_ev.pwm_value = pwm_value;
    xQueueSend(display_manager_queue, &display_ev, 10);
}
//------------------------------------------------------------------------------
void display_manager_refresh(uint8_t pwm_value, arrow_t arrow_orientation)
{
    display_event_t display_ev;

    display_ev.cmd = UPDATE_DISPLAY;
    display_ev.pwm_value = pwm_value;
    display_ev.arrow_orientation = arrow_orientation;
    
    xQueueSend(display_manager_queue, &display_ev, 10);
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------