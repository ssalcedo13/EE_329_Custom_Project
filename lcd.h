#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "main.h"
#include <stdint.h>

#define LCD_PORT        GPIOD
#define LCD_DATA_BITS   (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)

#define LCD_CTRL_PORT   GPIOA
#define LCD_EN          GPIO_PIN_7
#define LCD_RW          GPIO_PIN_6
#define LCD_RS          GPIO_PIN_5

#define LCD_CLR         0x01
#define LCD_HOME        0x02
#define LCD_LINE1_ADDR  0x80
#define NEW_LINE        0xC0

void LCD_config(void);
void LCD_init(void);
void LCD_pulse_ENA(void);
void LCD_4b_command(uint8_t command);
void LCD_command(uint8_t command);
void LCD_write_char(uint8_t letter);
void LCD_write_string(char *letters);

#endif
