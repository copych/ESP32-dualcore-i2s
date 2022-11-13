#include "driver/i2s.h"
#include <WiFi.h>

#define SAMPLE_RATE     44100
#define DMA_BUF_LEN     64
#define DMA_NUM_BUF     2
#define WAVE1_FREQ_HZ    440.0f
#define WAVE2_FREQ_HZ    500.0f
#define TWOPI            (float)(PI*2.0f)
#define PHASE_INC1       (float)(TWOPI * WAVE1_FREQ_HZ / (float)SAMPLE_RATE)
#define PHASE_INC2       (float)(TWOPI * WAVE2_FREQ_HZ / (float)SAMPLE_RATE)
#define I2S_BCLK_PIN     5
#define I2S_WCLK_PIN     19
#define I2S_DOUT_PIN     18

const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number

TaskHandle_t Task0;
TaskHandle_t Task1;
SemaphoreHandle_t xSemaphore = NULL;

// Accumulated phase
static float p1 = 0.0f;
static float p2 = 0.0f;

// Output buffers (2ch interleaved)
 static uint16_t out_buf0[DMA_BUF_LEN * 2];
 static uint16_t out_buf1[DMA_BUF_LEN * 2];


static void write_buffer0() {
    float samp = 0.0f;
    size_t bytes_written;

    for (int i=0; i < DMA_BUF_LEN; i++) {
      samp = (sinf(p1)+1.0f) * 4000; 
      
      // Increment and wrap phase
      p1 += PHASE_INC1;
      if (p1 >= TWOPI) {
        p1 -= TWOPI ;
      }
      
      out_buf0[i*2] = (uint16_t)(samp * 0.6f);
      out_buf0[i*2+1] = (uint16_t)(samp * 0.4f);
    }
    
      // See if we can obtain the semaphore.  If the semaphore is not available
      // wait 10 ticks to see if it becomes free.
      while ( xSemaphoreTake( xSemaphore, ( TickType_t ) 1 ) != pdTRUE ) {
       // taskYIELD();
      }
      // We were able to obtain the semaphore and can now access the
      // shared resource.
      for (int i=0; i < DMA_BUF_LEN; i++) { // sum buffers and output the result
        out_buf0[i*2] += out_buf1[i*2];
        out_buf0[i*2+1] += out_buf1[i*2+1] ;
      }
    // We have finished accessing the shared resource.  Release the
    // semaphore.
    
    // now out_buf0 is ready
    i2s_write(i2s_num, out_buf0, sizeof(out_buf0), &bytes_written, portMAX_DELAY);
    
    xSemaphoreGive( xSemaphore );  

}


static void write_buffer1()
{
    while ( xSemaphoreTake( xSemaphore, ( TickType_t ) 1 ) != pdTRUE ) {
     // taskYIELD();
    }
    float samp = 0.0f;
    size_t bytes_written;

    for (int i=0; i < DMA_BUF_LEN; i++) {
        samp = (sinf(p2)+1.0f) * 4000; 

        // Increment and wrap phase
        p2 += PHASE_INC2;
        if (p2 >= TWOPI)
            p2 -= TWOPI ;
        
        out_buf1[i*2] = (uint16_t)(samp * 0.4f);
        out_buf1[i*2+1] = (uint16_t)(samp * 0.6f);
    }
    xSemaphoreGive( xSemaphore );

    taskYIELD();
}

static void audio_task0(void *userData)
{
    vSemaphoreCreateBinary( xSemaphore );
    while(1) {
        write_buffer0();
    }
}

static void audio_task1(void *userData)
{
    while(1) {
        write_buffer1();
    }
}

void setup(void)
{
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);
  btStop();
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX ),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_I2S),
    
      .dma_buf_count = DMA_NUM_BUF,
      .dma_buf_len = DMA_BUF_LEN,
      .use_apll = false,
  };
  
  i2s_pin_config_t i2s_pin_config = {
      .bck_io_num = I2S_BCLK_PIN,
      .ws_io_num =  I2S_WCLK_PIN,
      .data_out_num = I2S_DOUT_PIN
  };

  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);

  i2s_set_pin(i2s_num, &i2s_pin_config); 

  xTaskCreatePinnedToCore(
                  audio_task0,  // function 
                  "Task0",      // name 
                  1024,         // stack size 
                  NULL,         // params 
                  1,            // priority 
                  &Task0,       // descriptor 
                  0);           // Core ID 
                      
  xTaskCreatePinnedToCore(
                  audio_task1,  // function 
                  "Task1",      // name 
                  1024,         // stack size 
                  NULL,         // params 
                  1,            // priority 
                  &Task1,       // descriptor 
                  1);           // Core ID 
     
}

void loop() {
  taskYIELD();
}
