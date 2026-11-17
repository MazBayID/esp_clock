#ifndef JAM_AUDIO_SYSTEM_H
#define JAM_AUDIO_SYSTEM_H

#include "../config/config.h"

// --- FUNGSI AUDIO BEEP (8-Bit Style) ---
void beep(int times) {
  size_t bytes_written;
  int16_t sample = 0;

  for (int t = 0; t < times; t++) {
    for (int i = 0; i < 16000 * 0.08; i++) {
      sample = (i % 16 < 8) ? 4000 : -4000;
      i2s_write(I2S_NUM_0, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
    }
    sample = 0;
    for (int i = 0; i < 16000 * 0.05; i++) {
      i2s_write(I2S_NUM_0, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
    }
  }
}

// --- SETUP I2S ---
void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = true
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

#endif
