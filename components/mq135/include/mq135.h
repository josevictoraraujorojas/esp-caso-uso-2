/*
 * MQ-135 - componente C para ESP-IDF
 *
 * Converte uma leitura ADC em resistencia Rs e calcula uma estimativa
 * empirica de concentracao equivalente de CO2 a partir de Rs/R0.
 *
 * IMPORTANTE:
 * - O resultado em ppm e uma estimativa, nao uma medicao seletiva de CO2.
 * - R0 deve ser obtido/calibrado em condicoes de referencia adequadas.
 * - A resistencia de carga e a resistencia calculada devem usar a mesma unidade.
 * - A conversao ADC para tensao e a calibracao do ADC podem melhorar a precisao.
 *
 * Equacao empirica e coeficientes de referencia:
 * GeorgK/MQ135: https://github.com/GeorgK/MQ135
 */

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

#endif /* MQ135_H */
