# ESP32-dualcore-i2s
Proof of concept for ESP32 dual core multithread sound synthezis.

This sketch creates 2 tasks -- one for each of two Xtensa LX6 cores. Each task generates a Sin() signal of a given frequency. One of the tasks then sums the buffers and writes the result to i2s DMA buffer. Data safety is based on a xTaskNotifyGive() / ulTaskNotifyTake().

Sampling rate is 44100Hz, samples are 16bit stereo.

Project was compiled in ArduinoIDE with ESP core 2.0.5.

Hardware is ESP32 DevKit with a PCM5102 extenal DAC connected to GPIOs 5, 18 and 19.

Initial code for a single core and internal DAC was taken from here: https://github.com/infrasonicaudio/esp32-i2s-synth-example
