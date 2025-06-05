/*
  Streaming data from Bluetooth to callback method
  
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
*/

// ==> Example to use built in DAC of ESP32

// #include "ESP_I2S.h"
#include "AudioTools.h"
#include "BluetoothA2DPSink.h"

// const uint8_t I2S_SCK = 26;   /* Audio data bit clock */
// const uint8_t I2S_WS = 25;    /* Audio data left and right clock */
// const uint8_t I2S_SDOUT = 22; /* ESP32 audio data output (to speakers) */

// I2SClass i2s;
// BluetoothA2DPSink a2dp_sink(i2s);

I2SStream out;
BluetoothA2DPSink a2dp_sink(out);

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

// Then somewhere in your sketch:
void read_data_stream(const uint8_t *data, uint32_t length) {
  // process all data
  int16_t *values = (int16_t *)data;
  uint32_t left = 0;
  uint32_t right = 0;
  if (length <= 0) {
    return;
  }

  for (int j = 0; j < length / 2; j += 2) {
    // print the 2 channel values
    // Serial.print(values[j]);
    // Serial.print(",");
    // Serial.println(values[j + 1]);
    left += abs(values[j]);
    right += abs(values[j + 1]);
  }

  left /= length;
  right /= length;

  Serial.printf("L:%d, R:%d\n", left, right);
}


void setup() {
  Serial.begin(115200);

  // output to callback and no I2S
  // a2dp_sink.set_stream_reader(read_data_stream, false);
  a2dp_sink.set_auto_reconnect(true);

  // i2s.setPins(I2S_SCK, I2S_WS, I2S_SDOUT);
  // if (!i2s.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
  //   Serial.println("Failed to initialize I2S!");
  //   while (1)
  //     ;  // do nothing
  // }

  // a2dp_sink.set_avrc_rn_playstatus_callback(avrc_rn_playstatus_callback);
  // a2dp_sink.set_avrc_rn_play_pos_callback(avrc_rn_play_pos_callback);
  a2dp_sink.start("MyMusic", false);

  // Serial.println("Connect e.g. from your Phone");
  // while (!a2dp_sink.is_connected())
  //   delay(1000);
  // Serial.println("Play or pause music to test callbacks.");
}


void loop() {
  delay(1000);
}
