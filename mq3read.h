#ifndef INC_MQ3READ_H_
#define INC_MQ3READ_H_

#include "main.h"
#include <stdint.h>

#define MQ3_ADC_MAX_COUNTS       4095.0f
#define MQ3_ADC_REF_VOLTAGE      3.3f
#define MQ3_VC_SUPPLY            5.0f
#define MQ3_RL_VALUE             4700.0f
#define MQ3_DIVIDER_MULTIPLIER   1.0f
#define MQ3_SAMPLE_COUNT         10
#define MQ3_R0_VALUE             8000.0f

typedef struct
{
    uint16_t adc_raw;
    float adc_voltage;
    float sensor_voltage;
    float rs;
    float ratio;
    float ppm;
    float mg_L;
    float bac_percent;
} MQ3_Data_t;

void MQ3_Read(ADC_HandleTypeDef *hadc, MQ3_Data_t *data);

float MQ3_GetSensorVoltage(uint16_t adc_raw);
float MQ3_CalculateRs(float sensor_voltage);
float MQ3_CalculatePPM(float ratio);
float MQ3_CalculateMgL(float ppm);
float MQ3_CalculateBAC(float mg_L);

#endif /* INC_MQ3READ_H_ */
