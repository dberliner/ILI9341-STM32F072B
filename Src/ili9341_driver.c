#include "ili9341.h"
#include "main.h"

static SPI_HandleTypeDef *_hspi = NULL;

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

static ili9341_hw_intf_t stm32f072b_ili9341_driver = {
  .reset_pin=reset_pin,
  .dc_pin=dc_pin,
  .cs_pin=cs_pin,
  .delay=delay,
  .sendbyte=sendbyte,
  .commit=NULL
};

const ili9341_hw_intf_t* stm32f072b_ili9341_setup_driver(SPI_HandleTypeDef *hspi) {
  _hspi = hspi;
  return &stm32f072b_ili9341_driver;
}