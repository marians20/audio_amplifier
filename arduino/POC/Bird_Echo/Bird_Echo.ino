#include <driver/i2s.h>
#include <math.h>
#include <esp_heap_caps.h>
#include <esp_wifi.h>
#include <esp_bt.h>

#define SAMPLE_RATE     44100
#define I2S_NUM         I2S_NUM_0
#define BUFFER_SIZE     256
#define AMPLITUDE       15000

#define ECHO_MS         250
#define ECHO_FB         0.4f

int16_t* bufferA = nullptr;
int16_t* bufferB = nullptr;
int16_t* echo_buffer = nullptr;
int echo_len = 0;
int echo_index = 0;

void audioTask(void* param) {
  float phase = 0.0f, lfo_phase = 0.0f;
  float freq = 1000, lfo_freq = 5.0f, pan = 0.5f;
  bool useA = true;

  while (true) {
    int16_t* buf = useA ? bufferA : bufferB;
    for (int i = 0; i < BUFFER_SIZE; i++) {
      float freq_sweep = 300.0f * sinf(lfo_phase);
      float step = 2.0f * PI * (freq + freq_sweep) / SAMPLE_RATE;
      phase += step;
      if (phase >= 2 * PI) phase -= 2 * PI;

      lfo_phase += 2.0f * PI * lfo_freq / SAMPLE_RATE;
      if (lfo_phase >= 2 * PI) lfo_phase -= 2 * PI;

      int16_t dry = (int16_t)(sinf(phase) * AMPLITUDE);

      int16_t echoL = echo_buffer[echo_index * 2];
      int16_t echoR = echo_buffer[echo_index * 2 + 1];
      int16_t outL = dry * (1.0f - pan) + echoL * ECHO_FB;
      int16_t outR = dry * pan + echoR * ECHO_FB;

      buf[i * 2]     = outR;
      buf[i * 2 + 1] = outL;

      echo_buffer[echo_index * 2]     = outL;
      echo_buffer[echo_index * 2 + 1] = outR;
      echo_index = (echo_index + 1) % echo_len;
    }

    size_t bytes_written;
    i2s_write(I2S_NUM, buf, BUFFER_SIZE * 4, &bytes_written, portMAX_DELAY);
    useA = !useA;

    static int count = 0;
    if (++count % 20 == 0) {
      freq = 800 + rand() % 1200;
      lfo_freq = 4.0f + ((rand() % 100) / 25.0f);
      pan = ((float)(rand() % 100) / 100.0f);
    }

    vTaskDelay(0);
  }
}

void setup() {
  Serial.begin(115200);

  esp_bt_controller_disable();
  esp_wifi_stop();

  bufferA = (int16_t*)heap_caps_malloc(BUFFER_SIZE * 2 * sizeof(int16_t), MALLOC_CAP_DMA);
  bufferB = (int16_t*)heap_caps_malloc(BUFFER_SIZE * 2 * sizeof(int16_t), MALLOC_CAP_DMA);
  if (!bufferA || !bufferB) {
    Serial.println("Buffer allocation failed.");
    while (true) delay(1000);
  }

  echo_len = (SAMPLE_RATE * ECHO_MS / 1000) & ~(BUFFER_SIZE - 1);
  echo_buffer = (int16_t*)heap_caps_malloc(echo_len * 2 * sizeof(int16_t), MALLOC_CAP_DMA);
  if (!echo_buffer) {
    Serial.println("Echo buffer allocation failed.");
    while (true) delay(1000);
  }
  memset(echo_buffer, 0, echo_len * 2 * sizeof(int16_t));

  i2s_config_t config = {
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

  i2s_pin_config_t pins = {
    .bck_io_num = 26,
    .ws_io_num = 25,
    .data_out_num = 22,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM, &config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pins);

  xTaskCreatePinnedToCore(audioTask, "audioTask", 8192, NULL, 3, NULL, 0);
}

void loop() {
  // Just the sound of nature...
}