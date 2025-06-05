#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "BluetoothA2DPSink.h"

static const char *TAG = "BT_MUSIC";

BluetoothA2DPSink a2dp_sink;

extern "C" void app_main() {
    ESP_LOGI(TAG, "Starting Bluetooth A2DP Sink...");
    a2dp_sink.start("ESP32_Music_Player");
}
