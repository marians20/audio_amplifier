/*
  Streaming Music from Bluetooth
  
  Copyright (C) 2020 Phil Schatzmann
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  https://github.com/pschatzmann/ESP32-A2DP
  https://github.com/pschatzmann/arduino-audio-tools
*/

// ==> Example which shows how to use the built in ESP32 I2S >= 3.0.0

/*
Pin mapping summary:

| ESP32 Pin | I2S Signal | PCM5102A Pin | Description                          |
|-----------|------------|--------------|--------------------------------------|
| GPIO26    | BCK        | BCK          | Bit Clock                            |
| GPIO25    | WS         | LRCK         | Word Select (Left/Right Clock)       |
| GPIO22    | SDOUT      | DIN          | Audio Data Output (from ESP32 to DAC)|
| 3.3V / 5V | —          | VCC          | Power Supply                         |
| GND       | —          | GND          | Ground                               |

*/

#include "ESP_I2S.h"
#include "BluetoothA2DPSink.h"

const uint8_t I2S_SCK = 26;    // BCK
const uint8_t I2S_WS = 25;     // LRCK
const uint8_t I2S_SDOUT = 22;  // DIN

#define SAMPLE_RATE 44100

// const uint8_t I2S_SCK = 5;       /* Audio data bit clock */
// const uint8_t I2S_WS = 25;       /* Audio data left and right clock */
// const uint8_t I2S_SDOUT = 26;    /* ESP32 audio data output (to speakers) */
I2SClass i2s;

BluetoothA2DPSink a2dp_sink(i2s);

void avrc_rn_play_pos_callback(uint32_t play_pos) {
  Serial.printf("Play position is %d (%d seconds)\n", play_pos, (int)round(play_pos / 1000.0));
}

void avrc_rn_playstatus_callback(esp_avrc_playback_stat_t playback) {
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

void avrc_rn_track_change_callback(uint8_t *id) {
  Serial.println("Track Change bits:");
  for (uint8_t i = 0; i < 8; i++) {
    Serial.printf("\tByte %d : 0x%x \n", i, id[i]);
  }
  //An example of how to project the pointer value directly as a uint8_t
  uint8_t track_change_flag = *id;
  Serial.printf("\tFlag value: %d\n", track_change_flag);
}

bool checkI2sError() {
  int err = i2s.lastError();
  if (err != 0) {
    Serial.printf("I2S Error #%d\n", err);
    return true;
  }

  return false;
}

// Then somewhere in your sketch:
void read_data_stream(const uint8_t *data, uint32_t length) {
  i2s.write(data, length);
  checkI2sError();
  if (length <= 0) {
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

  Serial.printf("L:%d, R:%d\n", left, right);
}

void playSineWave(float frequency, int duration_ms) {
  const int samples = SAMPLE_RATE * duration_ms / 1000;
  const float amplitude = 3000;
  const float twoPiF = 2 * PI * frequency / SAMPLE_RATE;

  for (int i = 0; i < samples; i++) {
    int16_t sample = (int16_t)(amplitude * sin(i * twoPiF));
    uint32_t sample_stereo = ((uint32_t)sample << 16) | (uint16_t)sample;
    i2s.write((const char *)&sample_stereo, sizeof(sample_stereo));
    checkI2sError();
  }
}


void setup() {
  Serial.begin(115200);
  i2s.setPins(I2S_SCK, I2S_WS, I2S_SDOUT);
  if (!i2s.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
    Serial.println("Failed to initialize I2S!");
    while (1)
      ;  // do nothing
  } else {
    Serial.println("I2S initialized!");
  }


  delay(1000);
  playSineWave(440.0, 3000);  // Play 440 Hz tone for 3 seconds


  // a2dp_sink.set_stream_reader(read_data_stream, false);
  // a2dp_sink.set_avrc_rn_playstatus_callback(avrc_rn_playstatus_callback);
  // a2dp_sink.set_avrc_rn_play_pos_callback(avrc_rn_play_pos_callback);
  // a2dp_sink.start("MyMusic");
}

void loop() {
  delay(6000);
  playSineWave(440.0, 3000);  // Play 440 Hz tone for 3 seconds
}