cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS components/ESP32-A2DP)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(bt_music_player)
