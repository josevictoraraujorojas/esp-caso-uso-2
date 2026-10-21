# Procedimento experimental — monitoramento ambiental

Este procedimento acompanha as leituras dos sensores do Caso de Uso 2, desde a publicação MQTT pelo ESP32 até o recebimento e a persistência no backend.

## 1. Observar o ESP32

No terminal ESP-IDF, com o dispositivo conectado:

```powershell
idf.py -p COM5 monitor
```

Troque `COM5` pela porta serial correta. Depois do período de aquecimento e calibração implementado no firmware, procure as linhas:

```text
[EXPERIMENTO][MQTT_ENVIO] topico=ambientedata mensagem_id=... payload={...}
```

São publicadas mensagens separadas para temperatura, umidade e qualidade do ar. Registre cada payload e os eventuais erros indicados por `[EXPERIMENTO][MQTT_ENVIO_FALHA]`.

## 2. Conferir o broker MQTT

Em outro terminal:

```powershell
mosquitto_sub -h HOST_DO_BROKER -p 1883 -t ambientedata -v
```

Substitua o host pelo endereço do broker usado no firmware. Compare os payloads exibidos pelo broker com os logs do ESP32.

## 3. Conferir o backend e o InfluxDB

No console Java do backend do Caso de Uso 2, procure estas marcas:

- `[EXPERIMENTO][MQTT_RECEBIDO]`: payload e tópico recebidos;
- `[EXPERIMENTO][INFLUX_INICIO]`: início da escrita no measurement `medicoes_ambientais`;
- `[EXPERIMENTO][INFLUX_PERSISTIDO]`: escrita retornou sem erro e registra valor, timestamp e IDs associados;
- `[EXPERIMENTO][FALHA_PROCESSAMENTO]` e `[EXPERIMENTO][INFLUX_FALHA]`: indicam problemas.

No InfluxDB Data Explorer, selecione o bucket `caso_uso_2`, measurement `medicoes_ambientais` e o intervalo observado. Confirme que os registros podem ser recuperados.

O Neo4j armazena informações de contexto e eventos ambientais conforme as regras da aplicação. Um evento ambiental é gravado quando os limites são ultrapassados e ainda não existe evento ativo equivalente; portanto, não se espera um evento novo para cada medição.

## Dados a registrar

Faça repetições sob condições registradas. Para cada uma, anote o período, as mensagens emitidas e recebidas, os registros persistidos e as falhas. A leitura do MQ-135 é uma estimativa dependente da calibração, não uma medição laboratorial de concentração sem validação independente.


## Limite do ensaio e salvamento dos logs

O firmware ambiental limita o ensaio a 100 publicações de dados no tópico `ambientedata`. Temperatura, umidade e qualidade do ar são mensagens separadas, portanto 100 publicações não equivalem a 100 ciclos completos dos três sensores. Ao alcançar o limite, registra `[EXPERIMENTO][FIM]` e encerra a tarefa de publicação. O contador local não prova que todas as mensagens chegaram ao broker ou foram persistidas.

Para gravar a saída serial em um arquivo no computador, abra o PowerShell na pasta do projeto e execute (ajuste a porta serial conforme necessário):

```powershell
idf.py -p COM5 flash monitor 2>&1 | Tee-Object -FilePath experimento-ambiental.log
```

O arquivo `experimento-ambiental.log` será criado na pasta atual. Se o firmware já estiver gravado, use `idf.py -p COM5 monitor 2>&1 | Tee-Object -FilePath experimento-ambiental.log` para não regravar a placa. Guarde o arquivo de cada execução separadamente e não reutilize o mesmo nome se quiser preservar o ensaio anterior.

Para salvar também as mensagens observadas no broker, em outro PowerShell execute:

```powershell
mosquitto_sub -h HOST_DO_BROKER -p 1883 -t ambientedata -v 2>&1 | Tee-Object -FilePath experimento-broker.log
```

Substitua `HOST_DO_BROKER` pelo host configurado no firmware. O arquivo do broker deve ser separado do log serial.
