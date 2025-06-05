/*

  https://github.com/pschatzmann/ESP32-A2DP
  https://github.com/pschatzmann/arduino-audio-tools


Pin mapping summary:

| ESP32 Pin | I2S Signal | PCM5102A Pin | Description                          |
|-----------|------------|--------------|--------------------------------------|
| GPIO26    | BCK        | BCK          | Bit Clock                            |
| GPIO25    | WS         | LRCK         | Word Select (Left/Right Clock)       |
| GPIO22    | SDOUT      | DIN          | Audio Data Output (from ESP32 to DAC)|
| 3.3V / 5V | —          | VCC          | Power Supply                         |
| GND       | —          | GND          | Ground                               |

*/

#include <AudioTools.h>
#include <BluetoothA2DPSink.h>

I2SStream i2s;  // I2S output stream
// BluetoothA2DPSink a2dp_sink(i2s);  // A2DP sink using AudioTools I2S output

BluetoothA2DPSink a2dp_sink;

void play_pos_callback(uint32_t play_pos) {
  Serial.printf("Play position is %d (%d seconds)\n", play_pos, (int)round(play_pos / 1000.0));
}

void playstatus_callback(esp_avrc_playback_stat_t playback) {
  switch (playback) {
    case esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_STOPPED:
      Serial.println("Stopped.");
      break;
    case esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING:
      Serial.println("Playing.");
      break;
    case esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PAUSED:
      Serial.println("Paused.");
      break;
    case esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_FWD_SEEK:
      Serial.println("Forward seek.");
      break;
    case esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_REV_SEEK:
      Serial.println("Reverse seek.");
      break;
    case esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_ERROR:
      Serial.println("Error.");
      break;
    default:
      Serial.printf("Got unknown playback status %d\n", playback);
  }
}

void track_change_callback(uint8_t *id) {
  Serial.println("Track Change bits:");
  for (uint8_t i = 0; i < 8; i++) {
    Serial.printf("\tByte %d : 0x%x \n", i, id[i]);
  }
  //An example of how to project the pointer value directly as a uint8_t
  uint8_t track_change_flag = *id;
  Serial.printf("\tFlag value: %d\n", track_change_flag);
}

bool checkI2sError() {
  int err = i2s.getWriteError();
  if (err != 0) {
    Serial.printf("I2S Error #%d\n", err);
    return true;
  }

  return false;
}

void read_data_stream(const uint8_t *data, uint32_t length) {
  if (length == 0) {
    Serial.println("Empty data received.");
    return;
  }

  int16_t *values = (int16_t *)data;
  uint32_t left = 0;
  uint32_t right = 0;

  for (int j = 0; j < length / 2; j += 2) {
    left += abs(values[j]);
    right += abs(values[j + 1]);
  }

  left /= length;
  right /= length;

  Serial.printf("L:%d, R:%d\n, Len:", left, right, length);

  size_t bytes_written = 0;
  esp_err_t result = i2s.write(I2S_NUM_0, data, length, &bytes_written, portMAX_DELAY);

  if (result != ESP_OK) {
    Serial.printf("I2S write failed: %d\n", result);
  } else if (bytes_written == 0) {
    Serial.println("I2S write succeeded but wrote 0 bytes!");
  } else {
    Serial.printf("%d bytes written.\n", bytes_written);
  }
}

  void initI2s() {
    auto cfg = i2s.defaultConfig();
    cfg.pin_bck = 26;   // BCK
    cfg.pin_ws = 25;    // LRCK
    cfg.pin_data = 22;  // DIN
    // cfg.sample_rate = 44100;
    // cfg.bits_per_sample = 16;
    // cfg.channels = 2;
    Serial.print("Initializing I2S...");
    if (!i2s.begin(cfg)) {  // Start I2S
      Serial.println("[FAIL]");
      while (1)
        ;  // do nothing
    } else {
      Serial.println("[ OK ]");
    }
  }

  void setup() {
    Serial.begin(115200);
    Serial.println("Starting Bluetooth A2DP Sink with PCM5102A...");

    initI2s();

    a2dp_sink.set_stream_reader(read_data_stream, false);
    a2dp_sink.set_avrc_rn_playstatus_callback(playstatus_callback);
    a2dp_sink.set_avrc_rn_play_pos_callback(play_pos_callback);
    a2dp_sink.set_auto_reconnect(true);
    a2dp_sink.start("ESP32-BT-Audio", false);  // Start Bluetooth sink
  }

  bool flag;

  void loop() {
    // Nothing to do here
    flag = !flag;
  }
