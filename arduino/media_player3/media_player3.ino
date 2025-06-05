#include <AudioTools.h>
#include <BluetoothA2DPSink.h>

class SampleSummary {
public:
  uint32_t leftSum = 0;
  uint32_t rightSum = 0;
  size_t count = 0;

  uint32_t sum() {
    return leftSum + rightSum;
  }

  float average() {
    return (float)sum() / (2 * count);
  }

  float leftAverage() {
    return (float) leftSum / count;
  }

  float rightAverage() {
    return (float) rightSum / count;
  }
};

class SampleSummaryCalculator {
public:
  SampleSummary getSampleSummary(const uint8_t* data, size_t len) {
    SampleSummary result;
    result.count = len / 2;
    for (int j = 0; j < len / 2; j += 2) {
      result.leftSum += abs(data[j]);
      result.rightSum += abs(data[j + 1]);
    }

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

  int ledPin = 2;
  unsigned long ledOnTime = 0;
  bool ledState = false;

  void begin(I2SConfig cfg) {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    I2SStream::begin(cfg);
  }

  size_t write(const uint8_t* data, size_t len) override {
    size_t written = I2SStream::write(data, len);

    SampleSummary sampleSummary = sampleSummaryCalculator.getSampleSummary(data, len);

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


BeatDetectI2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

// AnalogAudioStream analogOutput;
// BluetoothA2DPSink a2dp_sink(analogOutput);

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Bluetooth A2DP Sink with Beat Detection...");

  auto cfg = i2s.defaultConfig();
  cfg.pin_bck = 26;
  cfg.pin_ws = 25;
  cfg.pin_data = 22;
  cfg.sample_rate = 48000;
  cfg.bits_per_sample = 32;
  cfg.channels = 2;

  i2s.begin(cfg);
  // analogOutput.begin();
  a2dp_sink.start("ESP32-BT-Audio");
}

void loop() {
  // Nothing needed here
}
