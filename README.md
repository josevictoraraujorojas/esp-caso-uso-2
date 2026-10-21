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


As leituras do MQ-135 não devem ser tratadas como medição laboratorial precisa de um gás específico sem calibração e validação adequadas.

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
- Sensores DHT22/AM2301, MQ-135 conforme a montagem utilizada
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

## Testes experimentais do TCC

### Objetivo e fluxo avaliado

Verificar a leitura dos sensores ambientais e a publicação das mensagens pelo tópico MQTT `ambientedata`, acompanhando a cadeia até o backend do [Caso de Uso 2](https://github.com/josevictoraraujorojas/caso-de-uso-2) e a persistência em InfluxDB. A leitura do MQ-135 é dependente da calibração; não deve ser interpretada como medição laboratorial precisa sem validação.

### Preparação

1. Confira os sensores, GPIOs, alimentação e as credenciais Wi-Fi.
2. Configure o endereço/porta do broker MQTT no firmware.
3. Inicie o broker, InfluxDB, Neo4j e backend do Caso de Uso 2.
4. Identifique a porta serial do ESP32.

### Execução e coleta dos logs

Crie a pasta de resultados se necessário: `New-Item -ItemType Directory -Force -Path "$HOME\TCC-Experimentos"`. No terminal ESP-IDF, na raiz do projeto:

```powershell
idf.py build
idf.py -p COM5 flash
idf.py -p COM5 monitor 2>&1 | Tee-Object -FilePath "C:\Users\Jose\TCC-Experimentos\01-pzem-serial.log"
```

Substitua `COM5` pela porta correta. Encerre o monitor com `Ctrl+]` e finalize a transcrição com `Stop-Transcript` quando retornar ao prompt. Em outro terminal, observe o tópico MQTT:

```powershell
mosquitto_sub -h HOST_DO_BROKER -p 1883 -t ambientedata -v |
  Tee-Object -FilePath "$HOME\TCC-Experimentos\07-mqtt-ambiental.log"
```

Substitua o host pelo endereço real. Inicie o assinante antes do firmware e aguarde o marcador `[EXPERIMENTO][FIM]`.

### Como o teste foi instrumentado

Os logs do firmware usam marcadores com o prefixo `[EXPERIMENTO]` para acompanhar publicação, contagem e encerramento. O limite experimental é de até 100 mensagens de dados publicadas em `ambientedata`; isso não equivale necessariamente a 100 ciclos completos de leitura dos sensores, pois um ciclo pode gerar mais de uma mensagem. O retorno local do cliente MQTT não garante que o broker recebeu cada mensagem.

### Verificação e interpretação

- Compare contagem e erros do firmware com as mensagens observadas pelo assinante MQTT.
- No backend, procure `[EXPERIMENTO][MQTT_RECEBIDO]`, `[EXPERIMENTO][INFLUX_INICIO]`, `[EXPERIMENTO][INFLUX_PERSISTIDO]` e marcadores de falha.
- Consulte o bucket `caso_uso_2`, medição `medicoes_ambientais`, no InfluxDB, limitando a consulta ao intervalo da execução.
- Confira no Neo4j apenas os eventos e relacionamentos esperados para o cenário e para a lógica implementada; não presuma que cada leitura gere um evento no grafo.
- Registre mensagens publicadas, recebidas pelo broker/backend, registros encontrados e falhas.

A persistência deve ser confirmada com consulta ao banco. Registre as condições do ambiente e use um arquivo diferente para cada repetição.
