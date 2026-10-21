/* MQ-135: estimativas para ESP-IDF. PPM e empirico e requer calibracao. */
#ifndef MQ135_H
#define MQ135_H
#ifdef __cplusplus
extern "C" {
#endif
#define MQ135_ADC_MAX_12_BIT 4095
#define MQ135_CLEAN_AIR_RATIO 3.6f
float mq135_calculate_resistance(int adc_raw, int adc_max, float load_resistance);
float mq135_calibrate_r0(float rs_average, float clean_air_ratio);
float mq135_estimate_co2_ppm(float rs, float r0);
#ifdef __cplusplus
}
#endif
#endif
