#include <driver/i2s.h>
#include <math.h>
#include <esp_heap_caps.h>

#define SAMPLE_RATE     44100
#define I2S_NUM         I2S_NUM_0
#define BUFFER_SIZE     256
#define AMPLITUDE       16000

int16_t* stereo_buffer = nullptr;

void audioTask(void* param) {
  float phase = 0.0f;
  float lfo_phase = 0.0f;
  float freq = 1000;
  float lfo_freq = 5.0f;
  float pan = 0.5f;

  while (true) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
      float freq_sweep = 300.0f * sinf(lfo_phase); // Chirpy modulation
      float current_freq = freq + freq_sweep;
      float phase_inc = 2.0f * PI * current_freq / SAMPLE_RATE;
      phase += phase_inc;
      if (phase >= 2.0f * PI) phase -= 2.0f * PI;

      lfo_phase += 2.0f * PI * lfo_freq / SAMPLE_RATE;
      if (lfo_phase >= 2.0f * PI) lfo_phase -= 2.0f * PI;

      int16_t sample = (int16_t)(sinf(phase) * AMPLITUDE);
      int16_t left = sample * (1.0f - pan);
      int16_t right = sample * pan;

      stereo_buffer[i * 2]     = right;
      stereo_buffer[i * 2 + 1] = left;
    }

    size_t bytes_written;
    i2s_write(I2S_NUM, stereo_buffer, BUFFER_SIZE * 4, &bytes_written, portMAX_DELAY);

    // Occasionally randomize bird's "mood"
    static uint32_t counter = 0;
    counter++;
    if (counter % 20 == 0) {
      freq = 800 + rand() % 1200;
      lfo_freq = 4.0f + ((float)(rand() % 100) / 25.0f); // LFO in 4–8Hz range
      pan = ((float)(rand() % 100) / 100.0f);            // pan 0.0–1.0
    }

    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(115200);
  stereo_buffer = (int16_t*)heap_caps_malloc(BUFFER_SIZE * 2 * sizeof(int16_t), MALLOC_CAP_DMA);
  if (!stereo_buffer) {
    Serial.println("Failed to allocate buffer");
    while (true) delay(1000);
  }

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 1,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = 26,
    .ws_io_num = 25,
    .data_out_num = 22,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);

  xTaskCreatePinnedToCore(audioTask, "audioTask", 8192, NULL, 1, NULL, 0);
}

void loop() {
  // The bird lives here now 🐦
}