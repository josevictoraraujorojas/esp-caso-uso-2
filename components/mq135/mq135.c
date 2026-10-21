#include "mq135.h"

#include <math.h>

#define MQ135_CO2_A 116.6020682f
#define MQ135_CO2_B -2.769034857f

float mq135_calculate_resistance(int adc_raw, int adc_max, float load_resistance)
{
    if (adc_max <= 1 || load_resistance <= 0.0f) {
        return NAN;
    }

    if (adc_raw <= 0) {
        adc_raw = 1;
    } else if (adc_raw >= adc_max) {
        adc_raw = adc_max - 1;
    }

    return (((float)adc_max / (float)adc_raw) - 1.0f) * load_resistance;
}

float mq135_calibrate_r0(float rs_average, float clean_air_ratio)
{
    if (!isfinite(rs_average) || !isfinite(clean_air_ratio) ||
        rs_average <= 0.0f || clean_air_ratio <= 0.0f) {
        return NAN;
    }

    return rs_average / clean_air_ratio;
}

float mq135_estimate_co2_ppm(float rs, float r0)
{
    if (!isfinite(rs) || !isfinite(r0) || rs <= 0.0f || r0 <= 0.0f) {
        return NAN;
    }

    const float ratio = rs / r0;
    if (!isfinite(ratio) || ratio <= 0.0f) {
        return NAN;
    }

    return MQ135_CO2_A * powf(ratio, MQ135_CO2_B);
}
