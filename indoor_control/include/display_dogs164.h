#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"
#include "esp_log.h"

#define MODE_COMMAND 0x00
#define MODE_DATA 0x40

#define COMMAND_CLEAR_DISPLAY 0x01
#define COMMAND_RETURN_HOME 0x02
#define COMMAND_ENTRY_MODE_SET 0x04
#define ENTRY_MODE_LEFT_TO_RIGHT 0x02
#define ENTRY_MODE_SHIFT_INCREMENT 0x01
#define COMMAND_SHIFT 0x10
#define COMMAND_DISPLAY_SHIFT_LEFT 0x08
#define COMMAND_DISPLAY_SHIFT_RIGHT 0x0C
#define COMMAND_CURSOR_SHIFT_LEFT 0x00
#define COMMAND_CURSOR_SHIFT_RIGHT 0x04

#define ADDRESS_CGRAM 0x40
#define ADDRESS_DDRAM 0x80
#define ADDRESS_DDRAM_DOGS164_TOP_OFFSET 0x04

#define COMMAND_8BIT_4LINES_RE1_IS0 0x3A
#define COMMAND_8BIT_4LINES_RE0_IS0_DH1 0x3C
#define COMMAND_8BIT_4LINES_RE0_IS1 0x39
#define COMMAND_8BIT_4LINES_RE0_IS1_DH1 0x3D
#define COMMAND_8BIT_4LINES_RE0_IS0 0x38

// Command from extended set (RE = 1, IS = 0)
#define COMMAND_BS1_1 0x1E
#define COMMAND_POWER_DOWN_DISABLE 0x02
#define COMMAND_TOP_VIEW 0x05
#define COMMAND_BOTTOM_VIEW 0x06
#define COMMAND_4LINES 0x09
#define COMMAND_3LINES_TOP 0x1F
#define COMMAND_3LINES_MIDDLE 0x17
#define COMMAND_3LINES_BOTTOM 0x13
#define COMMAND_2LINES 0x1B

// Command from extended set (RE = 0, IS = 1)
#define COMMAND_DISPLAY 0x08
#define COMMAND_DISPLAY_ON 0x04
#define COMMAND_DISPLAY_OFF 0x00
#define COMMAND_CURSOR_ON 0x02
#define COMMAND_CURSOR_OFF 0x00
#define COMMAND_BLINK_ON 0x01
#define COMMAND_BLINK_OFF 0x00

// Command from extended set (RE = 1, IS = 1)
#define COMMAND_SHIFT_SCROLL_ENABLE 0x10
#define COMMAND_SHIFT_SCROLL_ALL_LINES 0x0F
#define COMMAND_SHIFT_SCROLL_LINE_1 0x01
#define COMMAND_SHIFT_SCROLL_LINE_2 0x02
#define COMMAND_SHIFT_SCROLL_LINE_3 0x04
#define COMMAND_SHIFT_SCROLL_LINE_4 0x08

#define COMMAND_BS0_1 0x1B
#define COMMAND_INTERNAL_DIVIDER 0x13
#define COMMAND_CONTRAST_DEFAULT_DOGS164 0x6B
#define COMMAND_POWER_CONTROL_DOGS164 0x56
#define COMMAND_POWER_ICON_CONTRAST 0x5C
#define COMMAND_FOLLOWER_CONTROL_DOGS164 0x6C
#define COMMAND_ROM_SELECT 0x72
#define COMMAND_ROM_A 0x00
#define COMMAND_ROM_B 0x04
#define COMMAND_ROM_C 0x08

esp_err_t init_reset_display_pin(void);
esp_err_t set_i2c(void);
esp_err_t display_send_command(uint8_t);
esp_err_t display_send_data(uint8_t);
esp_err_t set_cursor(uint8_t, uint8_t);
esp_err_t display_write_char(char);
esp_err_t display_write_string(const char *);
esp_err_t display_init(void);