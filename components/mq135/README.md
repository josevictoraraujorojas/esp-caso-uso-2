# Componente MQ-135 para ESP-IDF

Componente C sem API Arduino:

- `mq135_calculate_resistance()`: estima Rs a partir da leitura ADC.
- `mq135_calibrate_r0()`: estima R0 usando a média de Rs e uma relação de referência.
- `mq135_estimate_co2_ppm()`: aplica a equação empírica `116.6020682 * pow(Rs/R0, -2.769034857)`.

A resistência de carga deve corresponder ao circuito real e usar unidade consistente. `3.6` é uma relação aproximada, não uma calibração universal. O MQ-135 responde a vários gases; o resultado não é uma medição seletiva nem precisa de CO2 sem validação contra instrumento de referência. Não aplicar fator adicional `* 100` sem justificativa experimental.

Referência para os coeficientes: [GeorgK/MQ135](https://github.com/GeorgK/MQ135), biblioteca originalmente para Arduino. Este componente é uma implementação C para ESP-IDF.
