idf_component_register(
    SRCS "main.cpp" "esp_websocket_client.c" "connect.c" "stdin_out.c"
    INCLUDE_DIRS "tcp_transport" "esp-tls"
    REQUIRES LovyanGFX lvgl
)
