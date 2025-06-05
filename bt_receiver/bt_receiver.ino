#include <BluetoothA2DPSink.h>
#include <Adafruit_NeoPixel.h>
#include "freertos/queue.h"

#define NEOPIXEL_PIN 4
#define NUM_LEDS 16

Adafruit_NeoPixel strip(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
BluetoothA2DPSink a2dp_sink;

QueueHandle_t vuMeterQueue;

void setup() {
  esp_task_wdt_deinit();  // Disable watchdog
  Serial.begin(115200);
  Serial.println("Starting ESP32 Bluetooth Music Player...");
  strip.begin();
  strip.show();

  // Create queue for VU meter data
  vuMeterQueue = xQueueCreate(10, sizeof(int16_t));
  if (vuMeterQueue == NULL) {
    Serial.println("Failed to create VU meter queue!");
  } else {
    Serial.println("VU meter queue created.");
  }

  xTaskCreatePinnedToCore(vuMeterTask, "VU Meter", 2048, NULL, 1, NULL, 1);

  // Start Bluetooth A2DP Sink
i2s_pin_config_t my_pin_config = {
  .bck_io_num = 26,    // Bit Clock
  .ws_io_num = 25,     // Word Select (LRCK)
  .data_out_num = 22,  // Data Out
  .data_in_num = I2S_PIN_NO_CHANGE  // Not used
};

a2dp_sink.set_pin_config(my_pin_config);

  // a2dp_sink.set_output_device(BT_SPEAKER_I2S);  // Default is I2S
  a2dp_sink.set_output_device(BT_SPEAKER_INTERNAL_DAC);

  a2dp_sink.set_stream_reader(read_audio_stream, true);
  a2dp_sink.start("ESP32 Music Player");
  Serial.println("Bluetooth A2DP Sink started.");
}

void loop() {
  delay(1000);  // Idle loop
}

void read_audio_stream(const uint8_t *data, uint32_t len) {
  Serial.printf("Audio stream received. Length: %u bytes\n", len);

  int16_t *samples = (int16_t *)data;
  int sample_count = len / 2;

  int16_t max_sample = 0;
  for (int i = 0; i < sample_count; i++) {
    int16_t sample = abs(samples[i]);
    if (i < 10) {
      Serial.printf("Sample[%d]: %d\n", i, samples[i]);  // Print first 10 samples
    }
    if (sample > max_sample) max_sample = sample;
  }

  Serial.printf("Max sample: %d\n", max_sample);

  if (!xQueueSend(vuMeterQueue, &max_sample, 0)) {
    Serial.println("VU meter queue full, dropping sample.");
  }
}


void vuMeterTask(void *param) {
  int16_t level;
  while (true) {
    if (xQueueReceive(vuMeterQueue, &level, portMAX_DELAY)) {
      int led_count = map(level, 0, 32767, 0, NUM_LEDS);
      Serial.printf("VU Level: %d -> LEDs: %d\n", level, led_count);
      for (int i = 0; i < NUM_LEDS; i++) {
        if (i < led_count) {
          strip.setPixelColor(i, strip.Color(0, 255, 0));
        } else {
          strip.setPixelColor(i, 0);
        }
      }
      strip.show();
    }
  }
}
