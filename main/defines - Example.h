#pragma once
#define CONFIG_EXAMPLE_CONNECT_WIFI
#define CONFIG_WEBSOCKET_URI "wss://hassio.local/api/websocket"

#define CONFIG_EXAMPLE_WIFI_SSID "My WIFI SSID"
#define CONFIG_EXAMPLE_WIFI_PASSWORD "My WIFI Password"

// Declarations for the event source
#define TASK_ITERATIONS_COUNT        10      // number of times the task iterates
#define TASK_PERIOD                  500     // period of the task loop in milliseconds

enum {
    TASK_ITERATION_EVENT                     // raised during an iteration of the loop within the task
};