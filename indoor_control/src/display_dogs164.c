#include <stdio.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "driver/i2c.h"
#include "esp_log.h"

#include "../include/display_dogs164.h"
// #include "../include/display_manager.h"
//  #include "../include/board_def.h"

#define RESET_PIN_DISPLAY 13

#define DISPLAY_ADDRESS 0x3C      // caso 7 bits 78, o sino 0x3C caso 8 bits
#define I2C_MASTER_SCL_IO 22      // Pin SCL
#define I2C_MASTER_SDA_IO 21      // Pin SDA
#define I2C_MASTER_FREQ_HZ 400000 // Frecuencia I2C 400kHz
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

esp_err_t init_reset_display_pin(void)
{
    gpio_reset_pin(RESET_PIN_DISPLAY);
    gpio_set_direction(RESET_PIN_DISPLAY, GPIO_MODE_OUTPUT);
    return ESP_OK;
}

esp_err_t set_i2c(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = false,
        .scl_pullup_en = false,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        .clk_flags = 0};

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));

    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));

    return ESP_OK;
}

esp_err_t display_send_command(uint8_t command)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (DISPLAY_ADDRESS << 1) | I2C_MASTER_WRITE, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, true)); // Indica que es un comando
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, command, true));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}

esp_err_t display_send_data(uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (DISPLAY_ADDRESS << 1) | I2C_MASTER_WRITE, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x40, true)); // Indica que es un comando
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, data, true));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}

esp_err_t set_cursor(uint8_t row, uint8_t col)
{
    display_send_command(0x80 | (row * 0x20 + col));
    return ESP_OK;
}

esp_err_t display_write_char(char c)
{
    display_send_data(c);

    return ESP_OK;
}

esp_err_t display_write_string(const char *str)
{
    while (*str)
    {
        display_write_char(*str++);
    }
    return ESP_OK;
}

esp_err_t display_clean_arrow(void)
{
    set_cursor(1, 5);
    display_write_string(" ");
    return ESP_OK;
}

esp_err_t display_clean_power_and_bar(void)
{
    uint8_t i;
    for (i = 0; i < 4; i++)
    {
        set_cursor(1, i);
        display_write_string(" ");
    }
    for (i = 6; i < 16; i++)
    {
        set_cursor(1, i);
        display_write_string(" ");
    }
    return ESP_OK;
}

esp_err_t display_power_bar(uint8_t power)
{
    uint8_t full_bars = 0;
    uint8_t minor_bars = 0;
    uint8_t pixel_bars = 0;
    uint8_t i;
    if (power > 100)
    {
        power = 100;
    }

    pixel_bars = power / 2;
    full_bars = pixel_bars / 5;
    minor_bars = pixel_bars % 5;

    // imprimo las barras completas
    for (i = 6; i < 6 + full_bars; i++)
    {
        set_cursor(1, i);
        display_send_data(FIVE_BAR);
    }
    // imprimo la barra incompleta restante
    switch (minor_bars)
    {
    case 1:
        set_cursor(1, 6 + full_bars);
        display_send_data(ONE_BAR);
        break;
    case 2:
        set_cursor(1, 6 + full_bars);
        display_send_data(TWO_BAR);
        break;
    case 3:
        set_cursor(1, 6 + full_bars);
        display_send_data(THREE_BAR);
        break;
    case 4:
        set_cursor(1, 6 + full_bars);
        display_send_data(FOUR_BAR);
        break;
    default:
        break;
    }

    return ESP_OK;
}

esp_err_t display_set_screen(uint8_t power)
{
    char *master = "MASTER";
    char *lumenar = "LUMENAR";
    char numero[6];
    sprintf(numero, "%u%%", power);

    // hago secuencia de reset
    init_reset_display_pin();
    gpio_set_level(RESET_PIN_DISPLAY, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    // comandos para configurar el display
    display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_4LINES);
    display_send_command(COMMAND_BOTTOM_VIEW); //
    display_send_command(COMMAND_BS1_1);       //
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS1);
    display_send_command(COMMAND_BS0_1); //
    display_send_command(COMMAND_FOLLOWER_CONTROL_DOGS164);
    display_send_command(COMMAND_POWER_CONTROL_DOGS164);
    display_send_command(COMMAND_CONTRAST_DEFAULT_DOGS164);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0);
    display_send_command(COMMAND_CLEAR_DISPLAY);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    display_send_command(COMMAND_SHIFT_SCROLL_ALL_LINES); //
    display_send_command(COMMAND_DISPLAY_SHIFT_RIGHT);
    // aca defino que el display sea con dos lineas grandes
    display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_2LINES);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0_DH1);
    // limpio todo lo de la linea de datos
    display_clean_arrow();
    display_clean_power_and_bar();
    // escribo
    set_cursor(0, 0);
    display_write_string(master); // escribo master
    set_cursor(0, 9);
    display_write_string(lumenar); // escribo lumenar
    set_cursor(1, 0);
    display_write_string(numero); // escribo el valor de potencia
    // selecciono la ROM A para la flecha
    display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_ROM_SELECT);
    display_send_data(COMMAND_ROM_A);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0_DH1);
    // muestro la barra de potencia
    display_power_bar(power);

    return ESP_OK;
}

esp_err_t display_set_power(uint8_t power, char vege_flora)
{
    static uint8_t last_power = 0xFF; // Guardar el valor previo del power

    // limpio todo lo de la linea de datos
    char numero[6];

    set_cursor(1, 5);
    display_write_char(vege_flora);

    // Si el valor de power ha cambiado, actualiza la barra y el número
    if (power != last_power)
    {
        display_clean_power_and_bar();
        if (power > 100)
        {
            power = 100;
        }
        sprintf(numero, "%u%%", power);
        set_cursor(1, 0);
        display_write_string(numero);
        display_power_bar(power);
        last_power = power;
    }

    return ESP_OK;
}

esp_err_t display_set_vege_flora(char vege_flora)
{
    set_cursor(1, 4);
    display_write_char(vege_flora);

    return ESP_OK;
}

esp_err_t display_init()
{
    char *lumenar = "LUMENAR";

    char *iniciando = "Iniciando...";

    init_reset_display_pin();
    gpio_set_level(RESET_PIN_DISPLAY, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(RESET_PIN_DISPLAY, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_4LINES);
    display_send_command(COMMAND_BOTTOM_VIEW); //
    display_send_command(COMMAND_BS1_1);       //
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS1);
    display_send_command(COMMAND_BS0_1); //
    display_send_command(COMMAND_FOLLOWER_CONTROL_DOGS164);
    display_send_command(COMMAND_POWER_CONTROL_DOGS164);
    display_send_command(COMMAND_CONTRAST_DEFAULT_DOGS164);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0);
    display_send_command(COMMAND_DISPLAY | COMMAND_DISPLAY_ON | COMMAND_CURSOR_OFF | COMMAND_BLINK_OFF);
    display_send_command(COMMAND_CLEAR_DISPLAY);
    display_send_command(COMMAND_ENTRY_MODE_SET | ENTRY_MODE_LEFT_TO_RIGHT);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_ROM_SELECT);
    display_send_data(COMMAND_ROM_A);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0_DH1);
    display_send_command(COMMAND_8BIT_4LINES_RE1_IS0);
    display_send_command(COMMAND_3LINES_MIDDLE);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0_DH1);
    set_cursor(1, 4);
    display_write_string(lumenar);
    vTaskDelay(400 / portTICK_PERIOD_MS);
    set_cursor(2, 2);
    display_write_string(iniciando);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    return ESP_OK;
}

esp_err_t display_set_screen_one(screen_t *screen, uint8_t power, char vege_flora, bool dia, bool modo, struct tm time)
{ // en los argumentos de la funcion falta si auto o manual y si vege o flora, que variable?
  // bool dia, 1 si, 0 no.
  // bool modo, 1 manual, 0 auto.
  // para el tiempo, uso la time.h. revisar como es la estructura.
    char *phy = "PHY-03";
    char *lumenar = "LUMENAR";
    char *ppf = "PPF";
    char *p = "P";
    char *w = "W";
    char *automatic = "AUTO";
    char *manual = "MAN";
    char *ddots = ":";
    char *dias = "DIA";
    char hour[4];
    char min[4];
    char numero[6];
    char numeroppf[6];
    float ppfn = power * 2.97;

    *screen = SCREEN_ONE;

    sprintf(numero, "%u%%", power);
    sprintf(hour, "%u", time.tm_hour);
    sprintf(min, "%u", time.tm_min);
    sprintf(numeroppf, "%.f", ppfn);

    display_send_command(COMMAND_CLEAR_DISPLAY);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0);
    // primera fila
    set_cursor(0, 0);
    display_write_string(phy); // pongo el phy-03
    set_cursor(0, 9);
    display_write_string(lumenar); // pongo la marca lumenar
    // segunda fila
    set_cursor(1, 0);
    display_write_string(numero); // escribo el valor de potencia
    set_cursor(1, 5);
    display_write_char(vege_flora); // escribo la letra si es vege o flora
    display_power_bar(power);       // muestro la barra de potencia
    // tercera fila
    set_cursor(2, 0);
    display_write_string(ppf); // escribo la palabra ppf
    set_cursor(2, 3);
    display_write_string(ddots); // escribo los dos puntos
    set_cursor(2, 4);
    display_write_string(numeroppf); // escribo el numero del ppf
    set_cursor(2, 9);
    display_write_string(p); // escribo la letra P de la potencia total
    set_cursor(2, 10);
    display_write_string("00000"); // los 4 digitos de la potencia total (pueden ser 5?)
    set_cursor(2, 15);
    display_write_string(w); // escribo la W de la unidad de potencia
    // cuarta fila
    set_cursor(3, 0);
    display_write_string(dias);
    set_cursor(3, 3);
    display_write_string(ddots);
    set_cursor(3, 4);
    display_write_string("NO");
    set_cursor(3, 7);
    display_write_string(manual);
    set_cursor(3, 11);
    display_write_string(hour);
    set_cursor(3, 13);
    display_write_string(ddots);
    set_cursor(3, 14);
    display_write_string(min);

    /*screen_one_line_one();
    screen_one_line_two();
    screen_one_line_three();
    screen_one_line_four();*/
    return ESP_OK;
}

esp_err_t screen_one_line_three(struct tm time, bool dia, bool modo)
{
    char *dia_m;
    char *modo_m;
    char hour[4];
    char min[4];
    if (dia == true)
    {
        dia_m = "SI";
    }
    else
    {
        dia_m = "NO";
    }
    if (modo == true)
    {
        modo_m = "MAN";
    }
    else
    {
        modo_m = "AUTO";
    }

    sprintf(hour, "%u", time.tm_hour);
    sprintf(min, "%u", time.tm_min);
    set_cursor(3, 0);
    display_write_string("DIA");
    set_cursor(3, 3);
    display_write_string(":");
    set_cursor(3, 4);
    display_write_string(dia_m);
    set_cursor(3, 7);
    display_write_string(modo_m);
    set_cursor(3, 11);
    display_write_string(hour);
    set_cursor(3, 13);
    display_write_string(":");
    set_cursor(3, 14);
    display_write_string(min);
    return ESP_OK;
}

esp_err_t screen_two_line(uint8_t line, struct tm time_i, struct tm time_f)
{
    char *h = "ER";
    char *ini = "i";
    char *fin = "f";
    char houri[4];
    char hourf[4];
    char mini[4];
    char minf[4];

    switch (line)
    {
    case 0:
        h = "H1";
        break;
    case 1:
        h = "H2";
        break;
    case 2:
        h = "H3";
        break;
    case 3:
        h = "H4";
        break;

    default:
        break;
    }

    sprintf(houri, "%u", time_i.tm_hour);
    sprintf(mini, "%u", time_i.tm_min);
    sprintf(hourf, "%u", time_f.tm_hour);
    sprintf(minf, "%u", time_f.tm_min);

    set_cursor(line, 0);
    display_write_string(h);
    set_cursor(line, 3);
    display_write_string(ini);
    set_cursor(line, 4);
    display_write_string(houri);
    set_cursor(line, 6);
    display_write_string(":");
    set_cursor(line, 7);
    display_write_string(mini);
    set_cursor(line, 10);
    display_write_string(fin);
    set_cursor(line, 11);
    display_write_string(hourf);
    set_cursor(line, 13);
    display_write_string(":");
    set_cursor(line, 14);
    display_write_string(minf);
    return ESP_OK;
}

esp_err_t display_set_screen_two(screen_t *screen, struct tm time_i1, struct tm time_i2, struct tm time_i3, struct tm time_i4, struct tm time_f1, struct tm time_f2, struct tm time_f3, struct tm time_f4)
{

    display_send_command(COMMAND_CLEAR_DISPLAY);       // limpio display
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0); // me aseguro qeu se ponga en 4 lineas
    *screen = SCREEN_TWO;
    screen_two_line(0, time_i1, time_f1);
    screen_two_line(1, time_i2, time_f2);
    screen_two_line(2, time_i3, time_f3);
    screen_two_line(3, time_i4, time_f4);

    return ESP_OK;
}

esp_err_t display_set_screen_three(screen_t *screen, struct tm time_pwmi, struct tm time_pwmf, int fpower)
{
    char *pwm = "PWM";
    char *ini = "i";
    char *fin = "f";
    char *total = "Potencia total";
    char houri[4];
    char hourf[4];
    char mini[4];
    char minf[4];
    char fpowerc[6];

    display_send_command(COMMAND_CLEAR_DISPLAY);
    display_send_command(COMMAND_8BIT_4LINES_RE0_IS0);
    *screen = SCREEN_THREE;

    sprintf(houri, "%u", time_pwmi.tm_hour);
    sprintf(mini, "%u", time_pwmi.tm_min);
    sprintf(hourf, "%u", time_pwmf.tm_hour);
    sprintf(minf, "%u", time_pwmf.tm_min);
    sprintf(fpowerc, "%u", fpower);

    set_cursor(0, 0);
    display_write_string(pwm);
    set_cursor(0, 7);
    display_write_string(ini);
    set_cursor(0, 9);
    display_write_string(houri);
    set_cursor(0, 11);
    display_write_string(":");
    set_cursor(0, 12);
    display_write_string(mini);
    set_cursor(1, 7);
    display_write_string(fin);
    set_cursor(1, 9);
    display_write_string(hourf);
    set_cursor(1, 11);
    display_write_string(":");
    set_cursor(1, 12);
    display_write_string(minf);
    set_cursor(2, 0);
    display_write_string(total);
    set_cursor(3, 8);
    display_write_string(fpowerc);
    set_cursor(3, 13);
    display_write_string("W");

    return ESP_OK;
}

esp_err_t screen_three_line(uint8_t line, int fpower, struct tm time_i, struct tm time_f)
{
    char *pwm = "PWM";
    char *ini = "i";
    char *fin = "f";
    char houri[4];
    char hourf[4];
    char mini[4];
    char minf[4];
    char fpowerc[6];

    sprintf(houri, "%u", time_i.tm_hour);
    sprintf(mini, "%u", time_i.tm_min);
    sprintf(hourf, "%u", time_f.tm_hour);
    sprintf(minf, "%u", time_f.tm_min);
    sprintf(fpowerc, "%u", fpower);

    switch (line)
    {
    case 0:
        set_cursor(0, 0);
        display_write_string(pwm);
        set_cursor(0, 7);
        display_write_string(ini);
        set_cursor(0, 9);
        display_write_string(houri);
        set_cursor(0, 11);
        display_write_string(":");
        set_cursor(0, 12);
        display_write_string(mini);
        break;
    case 1:
        set_cursor(1, 7);
        display_write_string(fin);
        set_cursor(1, 9);
        display_write_string(hourf);
        set_cursor(1, 11);
        display_write_string(":");
        set_cursor(1, 12);
        display_write_string(minf);
        break;
    case 3:
        set_cursor(3, 8);
        display_write_string(fpowerc);
        set_cursor(3, 13);
        display_write_string("W");
        break;

    default:
        break;
    }

    return ESP_OK;
}
