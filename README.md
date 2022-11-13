# ESP32-dualcore-i2s
Proof of concept of dual core sound synthezis

This sketch creates 2 tasks -- one for each core, each task generates a Sin() signal of a given frequence, one of the tasks then sums the buffers and writes the result to i2s DMA buffer. Data safety is based on a single binary semaphore.
