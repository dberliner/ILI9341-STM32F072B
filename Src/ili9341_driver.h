#ifndef _ILI9341_DRIVER_H
#define _ILI9341_DRIVER_H

#include "ili9341.h"

const ili9341_hw_intf_t* stm32f072b_ili9341_setup_driver(SPI_HandleTypeDef *hspi);

#endif