# Componente MQ-135 para ESP-IDF

Componente C simples para integrar o MQ-135 a projetos ESP-IDF sem depender da API Arduino.

## Funcoes

- `mq135_calculate_resistance()`: estima Rs a partir da leitura bruta do ADC.
- `mq135_calibrate_r0()`: estima R0 usando a media de Rs e uma relacao de referencia.
- `mq135_estimate_co2_ppm()`: aplica a equacao empirica de referencia para estimar ppm equivalente de CO2.

## Uso

Inclua `mq135.h` e adicione `mq135` em `REQUIRES` no CMakeLists.txt do componente principal.

```c
#include "mq135.h"

float rs = mq135_calculate_resistance(adc_raw, MQ135_ADC_MAX_12_BIT, 10.0f);
float r0 = mq135_calibrate_r0(rs_media, MQ135_CLEAN_AIR_RATIO);
float ppm_estimado = mq135_estimate_co2_ppm(rs, r0);
```

A resistencia de carga (10.0 no exemplo) deve corresponder ao circuito real e usar a mesma unidade que Rs. O valor de R0 deve ser calibrado em condicoes conhecidas; a relacao 3.6 e uma aproximacao comum, nao uma calibracao universal.

## Limitacoes

O MQ-135 e sensivel a diferentes gases. A equacao baseada em `116.6020682 * pow(Rs/R0, -2.769034857)` e uma estimativa empirica frequentemente associada ao CO2, nao uma medicao seletiva nem adequada para alegar precisao laboratorial sem validacao contra um instrumento de referencia. Temperatura, umidade, aquecimento, variacao entre sensores, modulo/carga e conversao ADC afetam o resultado. O componente nao aplica fator adicional `* 100`.

## Referencia

Equacao e coeficientes de referencia: [GeorgK/MQ135](https://github.com/GeorgK/MQ135), biblioteca originalmente voltada ao Arduino. Este componente e uma implementacao C para ESP-IDF e nao incorpora a API Arduino.
