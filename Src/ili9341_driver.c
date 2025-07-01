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

static void commit(void* _unused) {
  (void) _unused;
  HAL_SPI_Transmit(_hspi, _vrambuf, _vrambuf_len, 1000);
  _vrambuf_len = 0;
}

void sendbyte(uint8_t b) {
  if (_vrambuf_len == ILI9341_MAX_X) {
    commit(NULL);
  }

  _vrambuf[_vrambuf_len++] = b;
}

static ili9341_hw_intf_t stm32f072b_ili9341_driver = {
  .reset_pin=reset_pin,
  .dc_pin=dc_pin,
  .cs_pin=cs_pin,
  .delay=delay,
  .sendbyte=sendbyte,
  .commit=commit
};

const ili9341_hw_intf_t* stm32f072b_ili9341_setup_driver(SPI_HandleTypeDef *hspi) {
  _hspi = hspi;
  return &stm32f072b_ili9341_driver;
}