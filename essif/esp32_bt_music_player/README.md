# ESP32 Bluetooth Music Player (Minimal C++ Template)

This is a minimal ESP-IDF C++ project that sets up a Bluetooth A2DP Sink using the ESP32 and outputs audio via I2S (e.g., to a PCM5102A DAC).

## ✅ Requirements

- ESP-IDF v5.0 or later
- CMake
- Python 3
- A PCM5102A or compatible I2S DAC

## 🛠️ Build Instructions

1. Clone this project into your ESP-IDF workspace:
   ```bash
   git clone <your_repo_url> esp32_bt_music_player
   cd esp32_bt_music_player
   ```

2. Build and flash
    ```bash
    idf.py set-target esp32
    idf.py build
    idf.py -p /dev/ttyUSB0 flash monitor
    ```
