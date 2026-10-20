# Monitoramento Ambiental — Caso de Uso 2

Firmware desenvolvido com **ESP-IDF** para aquisição de variáveis ambientais usando ESP32 e sensores conectados ao sistema de monitoramento do segundo caso de uso do TCC no IF Goiano – Campus Urutaí.

## Objetivo

Coletar medições ambientais em um ambiente de criação de suínos e encaminhá-las por MQTT para que a aplicação de backend possa receber e armazenar os dados para consulta e análise.

## Sensores

A montagem descrita para este caso de uso contempla:

| Sensor | Informação | Observações |
|---|---|---|
| DHT22 / AM2301 | Temperatura e umidade relativa do ar | Comunicação digital; GPIO 21 na configuração documentada |
| MQ-135 | Sinal relacionado à qualidade do ar | Saída analógica conectada ao GPIO 34 (ADC1_CH6); conversão em concentração depende da calibração |
| LDR | Intensidade luminosa relativa | Leitura analógica usada como indicação de luminosidade |

As leituras do MQ-135 não devem ser tratadas como medição laboratorial precisa de um gás específico sem calibração e validação adequadas. O LDR também fornece uma indicação relativa, salvo se houver calibração fotométrica.

## Fluxo de funcionamento

1. O ESP32 inicializa os periféricos e os drivers dos sensores.
2. Lê as variáveis ambientais disponíveis.
3. Conecta-se à rede Wi-Fi.
4. Publica os dados no broker MQTT, usando o tópico `ambientedata` na configuração documentada.
5. A aplicação consumidora recebe as mensagens e pode encaminhá-las aos bancos de dados usados pelo sistema.

## Comunicação

- **Microcontrolador:** ESP32
- **Framework:** ESP-IDF
- **Transporte:** Wi-Fi
- **Mensageria:** MQTT
- **Tópico de medições:** `ambientedata`

O formato exato do JSON deve ser conferido no código do firmware e mantido em sincronia com a aplicação que consome as mensagens.

## Requisitos

- Placa ESP32
- Sensores DHT22/AM2301, MQ-135 e LDR conforme a montagem utilizada
- ESP-IDF instalado e configurado
- Rede Wi-Fi e broker MQTT acessíveis
- Ligações elétricas e alimentação compatíveis com os sensores

## Configuração

Revise as credenciais Wi-Fi, o endereço e a porta do broker MQTT e os parâmetros de leitura dos sensores antes de executar o firmware. Mantenha credenciais privadas fora do controle de versão.

Verifique também se os GPIOs definidos no código correspondem à montagem física. No ESP32, entradas ADC não toleram tensões acima do limite elétrico do chip; use o circuito de condicionamento apropriado.

## Compilação e gravação

Em um terminal configurado com ESP-IDF, na raiz do projeto:

```bash
idf.py set-target esp32
idf.py build
idf.py -p PORTA flash monitor
```

Substitua `PORTA` pela porta serial da placa. Se o alvo já estiver configurado, não é necessário defini-lo novamente.

## Papel no TCC

Este firmware realiza a aquisição no caso de uso de monitoramento ambiental. As mensagens MQTT formam a entrada da cadeia de processamento, que integra dispositivos IoT, uma aplicação de backend e os mecanismos de armazenamento do projeto.

## Limitações

- A qualidade das medições depende do sensor, das condições físicas e da calibração.
- A estimativa de concentração a partir do MQ-135 depende de aquecimento, calibração e condições ambientais.
- A disponibilidade de envio depende da rede Wi-Fi e do broker MQTT.
