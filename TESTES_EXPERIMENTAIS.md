# Procedimento experimental — monitoramento ambiental

## Objetivo
Validar os payloads publicados pelo ESP32 com sensores ambientais antes de avaliar a persistência no backend.

## Execução
Compile e grave o firmware, conecte os sensores de acordo com a montagem real e aguarde o aquecimento/calibração previsto no firmware. Capture o tópico de medições:

    mosquitto_sub -h HOST_DO_BROKER -p 1883 -t ambientedata > captura.jsonl

Encerre a captura com Ctrl+C e execute:

    py tests\validar_experimento.py --autoteste
    py tests\validar_experimento.py --arquivo captura.jsonl --saida resultado.json

O script verifica valor, campo, sensor e esp32Mac. Os valores campo aceitos são temperatura, umidade e qualidade_ar, associados respectivamente aos sensores dht22, dht22 e mq135. Mensagens de erro do DHT são relatadas como falhas.

## Protocolo sugerido
Realize cinco repetições, anotando início/fim, mensagens recebidas, mensagens válidas e eventuais falhas. Sempre registre condições relevantes de ambiente e calibração. Não interprete o valor calculado pelo MQ-135 como concentração laboratorialmente precisa sem calibração independente.

## Limitação
O validador não comprova a exatidão metrológica dos sensores nem testa fisicamente os GPIOs. Ele analisa apenas as mensagens capturadas no broker.
