/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file    : main.c
* @brief   : Main program body for 1st STM32
*
* This STM32 reads the MQ-3 alcohol sensor, displays BAC on LCD,
* and sends the BAC value to the 2nd STM32 over UART.
*
* UART communication:
*   1st STM32 PA2 / USART2_TX -> 2nd STM32 PA3 / USART2_RX
*   1st STM32 PA3 / USART2_RX -> 2nd STM32 PA2 / USART2_TX
*   GND -> GND
******************************************************************************
*/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"
#include "lcd.h"
#include "mq3read.h"
#include "delay.h"

#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

MQ3_Data_t mq3;

char lcd_buffer[32];
char uart_msg[32];

/* Variables for receiving RESET from STM32 #2 */
char resetBuffer[16];
uint8_t resetIndex = 0;
uint8_t resetByte;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

void SendBAC_ToSecondSTM32(float bac_percent);
uint8_t CheckForResetCommand(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*
  Sends BAC to STM32 #2 over UART.

  Example sent message:
  0.092\n

  This avoids using %.3f because STM32 printf float support
  can be disabled by default.
*/
void SendBAC_ToSecondSTM32(float bac_percent)
{
    int bac_milli = (int)(bac_percent * 1000.0f + 0.5f);

    snprintf(uart_msg,
             sizeof(uart_msg),
             "%d.%03d\n",
             bac_milli / 1000,
             bac_milli % 1000);

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)uart_msg,
                      strlen(uart_msg),
                      HAL_MAX_DELAY);
}

/*
  Checks if STM32 #2 sent RESET over UART.

  Expected message:
  RESET\n

  Returns:
  1 if RESET was received
  0 if not received
*/
uint8_t CheckForResetCommand(void)
{
    if (HAL_UART_Receive(&huart2, &resetByte, 1, 0) == HAL_OK)
    {
        if (resetByte == '\n' || resetByte == '\r')
        {
            resetBuffer[resetIndex] = '\0';
            resetIndex = 0;

            if (strcmp(resetBuffer, "RESET") == 0)
            {
                return 1;
            }
        }
        else
        {
            if (resetIndex < sizeof(resetBuffer) - 1)
            {
                resetBuffer[resetIndex++] = resetByte;
            }
            else
            {
                resetIndex = 0;
            }
        }
    }

    return 0;
}

/* USER CODE END 0 */

/**
* @brief  The application entry point.
* @retval int
*/
int main(void)
{
    /* MCU Configuration--------------------------------------------------------*/

    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_USART2_UART_Init();

    SysTick_Init();

    LCD_init();

    /* USER CODE BEGIN 2 */

    LCD_command(0x01);
    delay_us(5000);

    LCD_command(0x80);
    LCD_write_string("MQ-3 BAC Sensor");

    LCD_command(0xC0);
    LCD_write_string("UART Ready");

    delay_us(1000000);

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /*
          Check if STM32 #2 sent RESET.
          This happens after the memory test finishes.
        */
        if (CheckForResetCommand())
        {
            LCD_command(0x01);
            delay_us(5000);

            LCD_command(0x80);
            LCD_write_string("RESET Received");

            LCD_command(0xC0);
            LCD_write_string("Restarting...");

            delay_us(1000000);
        }

        /*
          Read MQ-3 sensor data.
          mq3.bac_percent should contain the BAC value.
        */
        MQ3_Read(&hadc1, &mq3);

        /*
          Convert values to integers for clean display formatting.
          Example:
          mq3.bac_percent = 0.092
          bac_milli = 92
          displayed as 0.092%
        */
        int bac_milli = (int)(mq3.bac_percent * 1000.0f + 0.5f);
        int mv = (int)(mq3.adc_voltage * 1000.0f);

        /*
          Display BAC on LCD line 1.
        */
        LCD_command(0x01);
        delay_us(5000);

        LCD_command(0x80);

        snprintf(lcd_buffer,
                 sizeof(lcd_buffer),
                 "BAC:%d.%03d%%",
                 bac_milli / 1000,
                 bac_milli % 1000);

        LCD_write_string(lcd_buffer);

        /*
          Display ADC voltage on LCD line 2.
        */
        LCD_command(0xC0);

        snprintf(lcd_buffer,
                 sizeof(lcd_buffer),
                 "ADC:%4dmV",
                 mv);

        LCD_write_string(lcd_buffer);

        /*
          Send BAC value to STM32 #2.
          STM32 #2 will start the memory test if BAC > 0.08.
        */
        SendBAC_ToSecondSTM32(mq3.bac_percent);

        /*
          Send/read about twice per second.
        */
        delay_us(500000);

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
* @brief System Clock Configuration
* @retval None
*/
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

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
* @brief  This function is executed in case of error occurrence.
* @retval None
*/
void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
/**
* @brief Reports the name of the source file and source line number
* where the assert_param error has occurred.
*/
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
