//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "sdkconfig.h"

#include "../include/global_manager.h"
#include "../include/jumpers_manager.h"
#include "../include/pote_input_manager.h"
//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------
static SemaphoreHandle_t global_manager_semaph;
static global_manager_t global_manager_info;
//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
device_mode_t global_manager_find_device_mode(void)
{
    device_mode_t device_mode;

    if((is_jp2_reloj_connected() == false) && (is_jp3_teclas_connected() == false) 
        && (is_jp1_dspy_connected() == false))
    {
        device_mode = MODE_1; // FASE 1
    }
    else if((is_jp2_reloj_connected() == false) && (is_jp3_teclas_connected() == false) 
        && (is_jp1_dspy_connected() == true))
    {
        device_mode = MODE_2; // FASE 2 con pote
    }
    else if((is_jp2_reloj_connected() == false) && (is_jp3_teclas_connected() == true) 
        && (is_jp1_dspy_connected() == true))
    {
        device_mode = MODE_3; // FASE 2 con tecla
    }
    else if((is_jp2_reloj_connected() == true) && (is_jp3_teclas_connected() == true) 
        && (is_jp1_dspy_connected() == true))
    {
        device_mode = MODE_4; // FASE 3 con reloj montado y teclas
    }
    else
    {
        device_mode = MODE_1; // FASE 1
    }

    return(device_mode);
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void global_manager_init(void)
{
    device_mode_t device_mode;
    global_manager_semaph = xSemaphoreCreateBinary(); 

    if(global_manager_semaph != NULL)
        xSemaphoreGive(global_manager_semaph);
    else
        assert(0);

    jumpers_manager_init();

    device_mode = global_manager_find_device_mode();
    global_manager_set_device_mode(device_mode);

    global_manager_set_pwm_mode(MANUAL);

    pote_input_manager_init();


}
//------------------------------------------------------------------------------

uint8_t global_manager_set_pwm_manual_percentage(uint8_t pwm_manual_per_value)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        global_manager_info.pwm_manual_percent_power = pwm_manual_per_value;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_get_pwm_manual_percentage(uint8_t* pwm_manual_per_value)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        *pwm_manual_per_value = global_manager_info.pwm_manual_percent_power;
        xSemaphoreGive(global_manager_semaph);
        return 1;
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------

uint8_t global_manager_set_device_mode(device_mode_t device_mode)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        global_manager_info.device_mode = device_mode;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------

uint8_t global_manager_get_device_mode(device_mode_t *device_mode)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        *device_mode = global_manager_info.device_mode;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------

uint8_t global_manager_set_pwm_mode(pwm_mode_t pwm_mode)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {     
        global_manager_info.nv_info.pwm_mode = pwm_mode;
        xSemaphoreGive(global_manager_semaph);
        return 1; 
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//------------------------------------------------------------------------------
uint8_t global_manager_get_pwm_mode(pwm_mode_t* pwm_mode)
{
    if(xSemaphoreTake(global_manager_semaph, 10 / portTICK_PERIOD_MS))
    {
        *pwm_mode = global_manager_info.nv_info.pwm_mode;
        xSemaphoreGive(global_manager_semaph);
        return 1;
    }
    xSemaphoreGive(global_manager_semaph);
    return 0;
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------