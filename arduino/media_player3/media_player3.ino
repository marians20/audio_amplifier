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
| —         | —          | XSMT         | 3V3                                  |

*/

#include <AudioTools.h>
#include <BluetoothA2DPSink.h>
#include "vumeter.h"


// Vumeter parameters
#define PIN 0

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 16

// Luminosity from 0 to 255
#define Luminosity 16
#define BeatLuminosity 128

class SampleSummary {
public:
  uint32_t left = 0;
  uint32_t right = 0;
  size_t count = 0;

  uint32_t average() {
    return (left + right) / 2;
  }
};

class SampleSummaryCalculator {
public:
  SampleSummary getSampleSummary(const uint8_t* data, size_t len) {
    SampleSummary result;
    result.count = len / 2;
    int16_t *values = (int16_t *)data;

    for (int j = 0; j < len / 2; j += 2) {
      result.left += abs(values[j]);
      result.right += abs(values[j + 1]);
    }

    result.left /= len;
    result.right /= len;

    return result;
  }
};

// Beat detection class using raw amplitude analysis
class BeatDetector {
public:
  float sensitivity = 1.5;         // Multiplier for beat threshold
  unsigned long debounceMs = 100;  // Minimum time between beats
  unsigned long lastBeatTime = 0;

  static const int historySize = 50;
  float history[historySize] = { 0 };
  int index = 0;

  bool detect(SampleSummary data) {
    // Update moving average history
    float currentAvg = data.average();
    history[index] = currentAvg;
    index = (index + 1) % historySize;

    // Compute average of history
    float avgSum = 0;
    for (int i = 0; i < historySize; i++) {
      avgSum += history[i];
    }

    float movingAvg = avgSum / historySize;

    // Adaptive threshold
    float threshold = movingAvg * sensitivity;

    // Beat detection
    if (currentAvg > threshold && millis() - lastBeatTime > debounceMs) {
      lastBeatTime = millis();
      return true;
    }

    return false;
  }
};


// Custom I2S stream with beat detection and LED blink
class BeatDetectI2SStream : public I2SStream {
public:
  BeatDetector detector;
  SampleSummaryCalculator sampleSummaryCalculator;
  Vumeter *vumeter;

  int ledPin = 2;
  unsigned long ledOnTime = 0;
  bool ledState = false;

  BeatDetectI2SStream() {
    I2SStream();
    vumeter = new Vumeter(NUMPIXELS, PIN, Luminosity);
  }

  void begin(I2SConfig cfg) {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    I2SStream::begin(cfg);
  }

  size_t write(const uint8_t* data, size_t len) override {
    size_t written = I2SStream::write(data, len);

    SampleSummary sampleSummary = sampleSummaryCalculator.getSampleSummary(data, len);
    vumeter->show(sampleSummary.left, sampleSummary.right);
    if (detector.detect(sampleSummary)) {
      digitalWrite(ledPin, HIGH);
      ledOnTime = millis();
      ledState = true;
    }

    // Turn off LED after 30 ms
    if (ledState && millis() - ledOnTime > 30) {
      digitalWrite(ledPin, LOW);
      ledState = false;
    }

    return written;
  }
};

class DebugBluetoothA2DPSink : public BluetoothA2DPSink {
public:
  DebugBluetoothA2DPSink(audio_tools::AudioStream &output) : BluetoothA2DPSink(output) {
  }

  void start(const char *name, bool auto_reconect) override {
    set_avrc_rn_playstatus_callback(avrc_rn_playstatus_callback);
    set_avrc_rn_play_pos_callback(avrc_rn_play_pos_callback);
    BluetoothA2DPSink::start(name, auto_reconect);
  }
private:
  static void avrc_rn_play_pos_callback(uint32_t play_pos) {
    Serial.printf("Play position is %d (%d seconds)\n", play_pos, (int)round(play_pos / 1000.0));
  }

  static void avrc_rn_playstatus_callback(esp_avrc_playback_stat_t playback) {
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

  static void avrc_rn_track_change_callback(uint8_t* id) {
    Serial.println("Track Change bits:");
    for (uint8_t i = 0; i < 8; i++) {
      Serial.printf("\tByte %d : 0x%x \n", i, id[i]);
    }
    //An example of how to project the pointer value directly as a uint8_t
    uint8_t track_change_flag = *id;
    Serial.printf("\tFlag value: %d\n", track_change_flag);
  }
};


BeatDetectI2SStream i2s;
// I2SStream i2s;
DebugBluetoothA2DPSink a2dp_sink(i2s);

// AnalogAudioStream analogOutput;
// BluetoothA2DPSink a2dp_sink(analogOutput);

void beginI2s() {
  Serial.printf("Initializing I2S...");
  auto cfg = i2s.defaultConfig();
  cfg.pin_bck = 26;
  cfg.pin_ws = 25;
  cfg.pin_data = 22;
  i2s.begin(cfg);
  if (1 != 1) {
    Serial.println("[FAIL]");
    while (1)
      ;
  } else {
    Serial.println("[ OK ]");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Bluetooth A2DP Sink with Beat Detection...");
  beginI2s();
  // analogOutput.begin();
  a2dp_sink.start("ESP32-BT-Audio", true);
}

void loop() {
  // Nothing needed here
}
