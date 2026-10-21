
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "dht.h"
#include "driver/adc.h"
#include "mq135.h"

static const char *TAG = "SENSOR_AMBIENTAL";

/* Wi-Fi */
#define WIFI_SSID "Rojas_wgo_fibra"
#define WIFI_PASS "jr131415"

/* MQTT */
#define MQTT_BROKER_URI "mqtt://192.168.0.118:1883"
#define MQTT_TOPIC_DADOS "ambientedata"
#define MQTT_TOPIC_DISPOSITIVO "esp32"
#define TOTAL_MENSAGENS_TESTE 100

/* DHT22 */
#define DHT_GPIO GPIO_NUM_21

/* MQ135 */
#define MQ135_ADC_CHANNEL ADC1_CHANNEL_6
#define RL 10.0f

/* Identificacao do dispositivo */
static char device_id[20];
static char ip_str[16];

/* Estado do MQ135 */
static float R0 = 0;

/* Cliente MQTT */
static esp_mqtt_client_handle_t mqtt_client = NULL;
static int mensagens_dados_publicadas = 0;

/* Registra a tentativa e o resultado local da publicação MQTT. */
static int publicar_mensagem_experimental(
    esp_mqtt_client_handle_t cliente,
    const char *topico,
    const char *payload,
    int qos
)
{
    if (cliente == NULL || topico == NULL || payload == NULL)
    {
        ESP_LOGE(TAG, "[EXPERIMENTO][MQTT_ENVIO_FALHA] cliente, topico ou payload nulo");
        return -1;
    }

    if (strcmp(topico, MQTT_TOPIC_DADOS) == 0 &&
        mensagens_dados_publicadas >= TOTAL_MENSAGENS_TESTE)
    {
        return -2;
    }

    int mensagem_id = esp_mqtt_client_publish(
        cliente, topico, payload, 0, qos, 0
    );

    if (mensagem_id < 0)
    {
        ESP_LOGE(TAG, "[EXPERIMENTO][MQTT_ENVIO_FALHA] topico=%s payload=%s",
                 topico, payload);
    }
    else
    {
        ESP_LOGI(TAG, "[EXPERIMENTO][MQTT_ENVIO] topico=%s mensagem_id=%d qos=%d payload=%s",
                 topico, mensagem_id, qos, payload);
    }

    if (mensagem_id >= 0 && strcmp(topico, MQTT_TOPIC_DADOS) == 0)
    {
        mensagens_dados_publicadas++;
        ESP_LOGI(TAG, "[EXPERIMENTO][CONTAGEM] mensagens=%d/%d",
                 mensagens_dados_publicadas, TOTAL_MENSAGENS_TESTE);
        if (mensagens_dados_publicadas == TOTAL_MENSAGENS_TESTE)
            ESP_LOGI(TAG, "[EXPERIMENTO][FIM] total_mensagens=%d", mensagens_dados_publicadas);
    }

    return mensagem_id;
}



/* =========================================================
 * CALCULO DA RESISTENCIA DO MQ135
 * ========================================================= */

static float calcular_rs(int raw)
{
    return mq135_calculate_resistance(
        raw,
        MQ135_ADC_MAX_12_BIT,
        RL
    );
}


/* =========================================================
 * CALIBRACAO DO MQ135
 * ========================================================= */

static void calibrar_sensor(void)
{
    float rs = 0;
    const int amostras = 50;

    for (int i = 0; i < amostras; i++)
    {
        int raw = adc1_get_raw(MQ135_ADC_CHANNEL);

        rs += calcular_rs(raw);

        vTaskDelay(pdMS_TO_TICKS(200));
    }

    rs /= amostras;

    R0 = mq135_calibrate_r0(
        rs,
        MQ135_CLEAN_AIR_RATIO
    );
}


/* =========================================================
 * LEITURA E PUBLICACAO DOS SENSORES
 * ========================================================= */

static void mqtt_task(void *pvParameters)
{
    float humidity = 0;
    float temperature = 0;

    char msg_temp[128];
    char msg_umid[128];
    char msg_ar[128];
    char erro[64];

    adc1_config_width(ADC_WIDTH_BIT_12);

    adc1_config_channel_atten(
        MQ135_ADC_CHANNEL,
        ADC_ATTEN_DB_11
    );

    /* Aguarda o aquecimento do MQ135. */
    vTaskDelay(pdMS_TO_TICKS(300000));

    calibrar_sensor();

    while (mensagens_dados_publicadas < TOTAL_MENSAGENS_TESTE)
    {
        if (mqtt_client != NULL)
        {
            if (dht_read_float_data(
                    DHT_TYPE_AM2301,
                    DHT_GPIO,
                    &humidity,
                    &temperature
                ) == ESP_OK)
            {
                int raw = adc1_get_raw(MQ135_ADC_CHANNEL);

                float rs = calcular_rs(raw);

                float ppm = mq135_estimate_co2_ppm(rs, R0);
                if (!isfinite(ppm))
                {
                    ppm = 0.0f;
                }

                /* Monta as mensagens JSON. */
                snprintf(
                    msg_temp,
                    sizeof(msg_temp),
                    "{\"valor\":%.1f,\"campo\":\"temperatura\","
                    "\"sensor\":\"dht22\",\"esp32Mac\":\"%s\"}",
                    temperature,
                    device_id
                );

                snprintf(
                    msg_umid,
                    sizeof(msg_umid),
                    "{\"valor\":%.1f,\"campo\":\"umidade\","
                    "\"sensor\":\"dht22\",\"esp32Mac\":\"%s\"}",
                    humidity,
                    device_id
                );

                snprintf(
                    msg_ar,
                    sizeof(msg_ar),
                    "{\"valor\":%.2f,\"campo\":\"qualidade_ar\","
                    "\"sensor\":\"mq135\",\"esp32Mac\":\"%s\"}",
                    ppm,
                    device_id
                );

                /* Publica temperatura. */
                publicar_mensagem_experimental(mqtt_client, MQTT_TOPIC_DADOS, msg_temp, 0);

                /* Publica umidade. */
                publicar_mensagem_experimental(mqtt_client, MQTT_TOPIC_DADOS, msg_umid, 0);

                /* Publica qualidade do ar. */
                publicar_mensagem_experimental(mqtt_client, MQTT_TOPIC_DADOS, msg_ar, 0);
            }
            else
            {
                /* Publica erro caso a leitura do DHT falhe. */
                snprintf(
                    erro,
                    sizeof(erro),
                    "{\"erro\":\"falha no dht\","
                    "\"esp32Id\":\"%s\"}",
                    device_id
                );

                publicar_mensagem_experimental(mqtt_client, MQTT_TOPIC_DADOS, erro, 0);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    ESP_LOGI(TAG, "[EXPERIMENTO][TAREFA_ENCERRADA] total_mensagens=%d", mensagens_dados_publicadas);
    vTaskDelete(NULL);
}


/* =========================================================
 * EVENTOS MQTT
 * ========================================================= */

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
)
{
    esp_mqtt_event_handle_t event = event_data;

    switch (event_id)
    {
        case MQTT_EVENT_CONNECTED:
        {
            mqtt_client = event->client;

            esp_mqtt_client_subscribe(
                mqtt_client,
                MQTT_TOPIC_DADOS,
                1
            );

            char mensagem[256];

            snprintf(
                mensagem,
                sizeof(mensagem),
                "{\"ip\":\"%s\",\"nome\":\"esp32_sensor\","
                "\"macAddress\":\"%s\"}",
                ip_str,
                device_id
            );

            publicar_mensagem_experimental(mqtt_client, MQTT_TOPIC_DISPOSITIVO, mensagem, 0);

            xTaskCreate(
                mqtt_task,
                "mqtt_task",
                4096,
                NULL,
                5,
                NULL
            );

            break;
        }

        default:
            break;
    }
}


/* =========================================================
 * INICIALIZACAO MQTT
 * ========================================================= */

static void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI
    };

    esp_mqtt_client_handle_t client =
        esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(
        client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL
    );

    esp_mqtt_client_start(client);
}


/* =========================================================
 * EVENTOS WI-FI
 * ========================================================= */

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START ||
            event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            esp_wifi_connect();
        }
    }
    else if (event_base == IP_EVENT &&
             event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = event_data;

        snprintf(
            ip_str,
            sizeof(ip_str),
            IPSTR,
            IP2STR(&event->ip_info.ip)
        );

        mqtt_app_start();
    }
}


/* =========================================================
 * INICIALIZACAO DO SISTEMA
 * ========================================================= */

void app_main(void)
{
    ESP_LOGI(TAG, "[EXPERIMENTO][INICIO] inicializando sensores ambientais");
    uint8_t mac[6];

    ESP_ERROR_CHECK(
        esp_read_mac(mac, ESP_MAC_WIFI_STA)
    );

    snprintf(
        device_id,
        sizeof(device_id),
        "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2],
        mac[3], mac[4], mac[5]
    );

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t configuracao = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&configuracao));

    const wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config)
    );

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            wifi_event_handler,
            NULL,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            wifi_event_handler,
            NULL,
            NULL
        )
    );

    ESP_ERROR_CHECK(esp_wifi_start());
}
