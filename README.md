# ESP32-dualcore-i2s
Proof of concept of dual core multithread sound synthezis.

This sketch creates 2 tasks -- one for each core. Each task generates a Sin() signal of a given frequency, one of the tasks then sums the buffers and writes the result to i2s DMA buffer. Data safety is based on a single binary semaphore.

Sampling rate is 44100Hz, samples are 16bit stereo.

Project was compiled in ArduinoIDE with ESP core 2.0.5.

Initial code for a single core was taken from here: https://github.com/infrasonicaudio/esp32-i2s-synth-example
