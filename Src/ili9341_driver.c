#include "ili9341.h"
#include "main.h"
#include <string.h>

static SPI_HandleTypeDef *_hspi = NULL;

// Forward decl
static void barrier(void *_unused);

/* This device only has 16k ram and it would take ~178k to buffer a full colored frame */
/* Rows of 240 18-byte pixels are byte alligned (540 bytes) */
#define _BUFF_CNT 2
static uint8_t _vrambuf[_BUFF_CNT][ILI9341_MAX_X*2] = { 0 };
static uint16_t _vrambuf_len[_BUFF_CNT] = { 0 };
static uint8_t _vram_active_buf = 0;

#define _ACTIVE_VRBUF _vrambuf[_vram_active_buf]
#define _ACTIVE_VBLEN _vrambuf_len[_vram_active_buf]
#define _ACTIVE_VRBUF_SWAP() _ACTIVE_VBLEN=0; _vram_active_buf=(_vram_active_buf==_BUFF_CNT-1) ? (_vram_active_buf=0) : (++_vram_active_buf);

static void reset_pin(ili9341_reset_e status) {
  if (status == RESET_HIGH_NOTSET) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
  } else {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
  }
}

static void dc_pin(ili9341_dc_e status) {
  if (status == DC_LOW_CMD) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
  } else {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
  }
}

static void cs_pin(ili9341_cs_e status) {
  if (status == CS_LOW_ON) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
  } else {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
  }
}

static void delay(uint32_t delay_us) {
  /* TODO Use a microsecond level delay */
  /* Round up to the next millisecond */
  if (delay_us > 0 && delay_us % 1000 != 0) {
    delay_us += 1000;
  }
  HAL_Delay(delay_us / 1000);
}

static void commit(void* _unused) {
  (void) _unused;

  if (!_ACTIVE_VBLEN) return;

  /* Wait until sending is possible */
  HAL_StatusTypeDef status;
  while ((status = HAL_SPI_Transmit_DMA(_hspi, _ACTIVE_VRBUF, _ACTIVE_VBLEN)) == HAL_BUSY) ;

  _ACTIVE_VRBUF_SWAP();

}

static void sendbuf(const ili9341_buf_t *buf) {
  HAL_StatusTypeDef status;

  if (!buf || !buf->buf || !buf->len) {
    return;
  }

  /* Finish anything buffered or in flight */
  barrier(NULL);

  while ((status = HAL_SPI_Transmit_DMA(_hspi, buf->buf, buf->len)) == HAL_BUSY) ;
}

static void sendbyte(uint8_t b) {
  if (_ACTIVE_VBLEN == sizeof(_vrambuf[0])) {
    commit(NULL);
  }

  _ACTIVE_VRBUF[_ACTIVE_VBLEN++] = b;
}

static void barrier(void *_unused) {
  (void) _unused;
  if (_ACTIVE_VBLEN) {
    commit(NULL);
  }
  while (_hspi->State != HAL_SPI_STATE_READY) ;
}

static ili9341_hw_intf_t stm32f072b_ili9341_driver = {
  .reset_pin=reset_pin,
  .dc_pin=dc_pin,
  .cs_pin=cs_pin,
  .delay=delay,
  .sendbuf=sendbuf,
  .sendbyte=sendbyte,
  .commit=commit,
  .barrier=barrier
};

const ili9341_hw_intf_t* stm32f072b_ili9341_setup_driver(SPI_HandleTypeDef *hspi) {
  _hspi = hspi;
  return &stm32f072b_ili9341_driver;
}