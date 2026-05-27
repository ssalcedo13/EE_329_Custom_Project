#include "main.h"
#include "adc.h"
#include "gpio.h"
#include "lcd.h"
#include "mq3read.h"
#include "delay.h"
#include <stdio.h>

MQ3_Data_t mq3;
char lcd_buffer[32];

void SystemClock_Config(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    SysTick_Init();

    MX_GPIO_Init();
    MX_ADC1_Init();

    LCD_init();

    while (1)
    {
        MQ3_Read(&hadc1, &mq3);

        int bac_milli = (int)(mq3.bac_percent * 1000.0f);
        int mv = (int)(mq3.adc_voltage * 1000.0f) + 100;

        LCD_command(0x01);
        delay_us(5000);

        LCD_command(0x80);

        snprintf(lcd_buffer,
                 sizeof(lcd_buffer),
                 "BAC:%d.%03d%%",
                 bac_milli / 1000,
                 bac_milli % 1000);

        LCD_write_string(lcd_buffer);

        LCD_command(0xC0);

        snprintf(lcd_buffer,
                 sizeof(lcd_buffer),
                 "ADC:%4dmV",
                 mv);

        LCD_write_string(lcd_buffer);

        delay_us(500000);
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 |
                                  RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
