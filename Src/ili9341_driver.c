#include "ili9341.h"
#include "main.h"
#include <string.h>

static SPI_HandleTypeDef *_hspi = NULL;

/* This device only has 16k ram and it would take ~178k to buffer a full colored frame */
/* Rows of 240 18-byte pixels are byte alligned (540 bytes) */
static uint8_t _vrambuf[ILI9341_MAX_X*2] = { 0 };
static uint8_t _vrambuf_len = 0;

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

void sendbyte(uint8_t b) {
  HAL_SPI_Transmit(_hspi, &b, 1, 1000);
}

static void commit(void* _unused) {
  (void) _unused;
  HAL_SPI_Transmit(_hspi, _vrambuf, _vrambuf_len, 1000);
  _vrambuf_len = 0;
}

/* Writes the 18 least significant bits to the outgoing buffer */
static void sendPx(uint32_t b) {
  uint8_t *mem_byte = _vrambuf + _vrambuf_len;
  uint8_t R=0, G=0, B=0;
  uint8_t colorBuf[2] = { 0 };

  /* TODO Support 666 color scheme if that's what is enabled */
  ILI9341_RGB565_DECODE(b, R, G, B)
  /* Sent first: The entire R buffer (5 bits) */
  colorBuf[0] = R<<3;
  /* Sent second: The most significant bits of G (3 bits) */
  colorBuf[0] |= (G>>3) & 0x7;
  /* Sent third: The least significant bits of G (3 bits) */
  colorBuf[1] = (G & 0x7)<<5;
  /* Sent fourth: The B buffer (5 bits) */
  colorBuf[1] |= B & 0x1F;
  memcpy(mem_byte, colorBuf, sizeof(colorBuf));

  if ((_vrambuf_len+=2) == ILI9341_MAX_X) {
    commit(NULL);
  }
}


static ili9341_hw_intf_t stm32f072b_ili9341_driver = {
  .reset_pin=reset_pin,
  .dc_pin=dc_pin,
  .cs_pin=cs_pin,
  .delay=delay,
  .sendbyte=sendbyte,
  .sendpx=sendPx,
  .commit=commit,
  .async=true
};

const ili9341_hw_intf_t* stm32f072b_ili9341_setup_driver(SPI_HandleTypeDef *hspi) {
  _hspi = hspi;
  return &stm32f072b_ili9341_driver;
}