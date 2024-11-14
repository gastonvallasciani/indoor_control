#ifndef LED_MANAGER_H__
#define LED_MANAGER_H__
//------------------- INCLUDES -------------------------------------------------
//------------------------------------------------------------------------------
//------------------- MACROS Y DEFINES -----------------------------------------
//------------------------------------------------------------------------------

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE DATOS EXTERNOS ----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES EXTERNAS ------------------------
//------------------------------------------------------------------------------
void led_manager_init(void);

void led_manager_rele_vege_on(void);
void led_manager_rele_vege_off(void);
void led_manager_pwm_output(uint8_t pwm_value);
//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif /* LED_MANAGER_H__ */