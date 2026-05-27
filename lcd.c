#include "main.h"
#include "lcd.h"
#include "delay.h"

static void LCD_delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        delay_us(1000);
    }
}

static void LCD_write_nibble(uint8_t nibble)
{
    LCD_PORT->BRR = LCD_DATA_BITS;

    if (nibble & 0x10) LCD_PORT->BSRR = GPIO_PIN_4;
    if (nibble & 0x20) LCD_PORT->BSRR = GPIO_PIN_5;
    if (nibble & 0x40) LCD_PORT->BSRR = GPIO_PIN_6;
    if (nibble & 0x80) LCD_PORT->BSRR = GPIO_PIN_7;
}

void LCD_config(void)
{
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIODEN);

    GPIOD->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5 |
                      GPIO_MODER_MODE6 | GPIO_MODER_MODE7);

    GPIOD->MODER |=  (GPIO_MODER_MODE4_0 | GPIO_MODER_MODE5_0 |
                      GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0);

    GPIOD->OTYPER &= ~(GPIO_OTYPER_OT4 | GPIO_OTYPER_OT5 |
                       GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7);

    GPIOD->PUPDR &= ~(GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5 |
                      GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7);

    GPIOA->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE6 |
                      GPIO_MODER_MODE7);

    GPIOA->MODER |=  (GPIO_MODER_MODE5_0 | GPIO_MODER_MODE6_0 |
                      GPIO_MODER_MODE7_0);

    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT5 | GPIO_OTYPER_OT6 |
                       GPIO_OTYPER_OT7);

    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD5 | GPIO_PUPDR_PUPD6 |
                      GPIO_PUPDR_PUPD7);

    LCD_CTRL_PORT->BRR = LCD_RS | LCD_RW | LCD_EN;
    LCD_PORT->BRR = LCD_DATA_BITS;
}

void LCD_pulse_ENA(void)
{
    LCD_CTRL_PORT->BSRR = LCD_EN;
    delay_us(50);

    LCD_CTRL_PORT->BRR = LCD_EN;
    delay_us(50);
}

void LCD_4b_command(uint8_t command)
{
    LCD_CTRL_PORT->BRR = LCD_RS | LCD_RW;

    LCD_write_nibble(command & 0xF0);
    LCD_pulse_ENA();
}

void LCD_command(uint8_t command)
{
    LCD_CTRL_PORT->BRR = LCD_RS | LCD_RW;

    LCD_write_nibble(command & 0xF0);
    LCD_pulse_ENA();

    LCD_write_nibble((command << 4) & 0xF0);
    LCD_pulse_ENA();

    LCD_delay_ms(2);
}

void LCD_write_char(uint8_t letter)
{
    LCD_CTRL_PORT->BSRR = LCD_RS;
    LCD_CTRL_PORT->BRR = LCD_RW;

    LCD_write_nibble(letter & 0xF0);
    LCD_pulse_ENA();

    LCD_write_nibble((letter << 4) & 0xF0);
    LCD_pulse_ENA();

    LCD_delay_ms(2);
}

void LCD_write_string(char *letters)
{
    while (*letters)
    {
        LCD_write_char((uint8_t)*letters);
        letters++;
    }
}

void LCD_init(void)
{
    LCD_config();

    LCD_delay_ms(100);

    LCD_CTRL_PORT->BRR = LCD_RS | LCD_RW | LCD_EN;
    LCD_PORT->BRR = LCD_DATA_BITS;

    LCD_4b_command(0x30);
    LCD_delay_ms(10);

    LCD_4b_command(0x30);
    LCD_delay_ms(10);

    LCD_4b_command(0x30);
    LCD_delay_ms(10);

    LCD_4b_command(0x20);
    LCD_delay_ms(10);

    LCD_command(0x28);
    LCD_command(0x0C);
    LCD_command(0x06);
    LCD_command(0x01);

    LCD_delay_ms(5);
}
