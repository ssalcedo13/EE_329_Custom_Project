#include "mq3read.h"
#include <math.h>

float MQ3_GetSensorVoltage(uint16_t adc_raw)
{
    float adc_voltage;

    adc_voltage = ((float)adc_raw / MQ3_ADC_MAX_COUNTS) * MQ3_ADC_REF_VOLTAGE;

    return adc_voltage * MQ3_DIVIDER_MULTIPLIER;
}

float MQ3_CalculateRs(float sensor_voltage)
{
    if (sensor_voltage <= 0.01f)
    {
        return 0.0f;
    }

    return MQ3_RL_VALUE * ((MQ3_VC_SUPPLY / sensor_voltage) - 1.0f);
}

float MQ3_CalculatePPM(float ratio)
{
    float ppm;

    if (ratio <= 0.0f)
    {
        return 0.0f;
    }

    ppm = 3.0f * powf(ratio, -1.431f) * 1000.0f;

    if (ppm < 5.0f)
    {
        ppm = 0.0f;
    }

    return ppm;
}

float MQ3_CalculateMgL(float ppm)
{
    /*
     * Rough conversion used for display only:
     * 500 ppm alcohol vapor ≈ 1.0 mg/L
     */
    return ppm / 500.0f;
}

float MQ3_CalculateBAC(float mg_L)
{
    /*
     * Very rough estimate:
     * 0.38 mg/L breath alcohol ≈ 0.08% BAC
     */
    return mg_L * 0.009f;
}

void MQ3_Read(ADC_HandleTypeDef *hadc, MQ3_Data_t *data)
{
    uint32_t adc_sum = 0;
    uint16_t raw = 0;

    for (uint16_t i = 0; i < MQ3_SAMPLE_COUNT; i++)
    {
        HAL_ADC_Stop(hadc);
        HAL_ADC_Start(hadc);

        while (HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY) != HAL_OK)
        {
        }

        raw = (uint16_t)HAL_ADC_GetValue(hadc);
        adc_sum += raw;

        delay_us(500);
    }

    HAL_ADC_Stop(hadc);

    data->adc_raw = (uint16_t)(adc_sum / MQ3_SAMPLE_COUNT);

    data->adc_voltage =
        ((float)data->adc_raw / 4095.0f) * 3.3f;

    data->sensor_voltage = data->adc_voltage;

    data->rs = MQ3_CalculateRs(data->sensor_voltage);
    data->ratio = data->rs / MQ3_R0_VALUE;
    data->ppm = MQ3_CalculatePPM(data->ratio);
    data->mg_L = MQ3_CalculateMgL(data->ppm);
    data->bac_percent = MQ3_CalculateBAC(data->mg_L);
}
