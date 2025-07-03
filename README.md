# ILI9341-STM32F072B #

## Overview ##

This is a demonstration of a ILI9341 LCD using SPI connected to a STM32F072B Discovery board. My purpose for this project was to work in a redicilously resource constrained environment, and to that end it works quite well.

## Connections ##

| Board | LCD |
| ------------- | ------------- |
| PB5 | DISP_DC |
| PB6 | SPI_CS |
| PB15 | SPI_MOSI |
| PB14 | SPI_MISO |
| PC13 | PI_SCK |
| PB12 | DISP_RESET |
| PA9 | USART1_TX |
| PA10 | USART1_RX |
| 3V | VCC |
| 3V | LCD |
| GND | GND |

## Demonstration ##

<img src="img/img.jpg" />

## Note on speed ##

Full screen writes take about 600ms if dealing with arbitrary pixels. The bottleneck here is the speed of memory writes, a buffer transfered via the DMA repetitively (to draw a pattern or solid color) will alow for ~3fps.