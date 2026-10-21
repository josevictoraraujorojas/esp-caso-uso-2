idf_component_register(
    SRCS "hello_world_main.c" 
    INCLUDE_DIRS "."
    REQUIRES esp_wifi nvs_flash esp_netif mqtt dht driver mq135
)
