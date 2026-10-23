#ifndef __SPI_H__
#define __SPI_H__

#include <pico/stdio.h>

void SPI_init();
void SPI_write32(uint16_t address, uint32_t value);
uint32_t SPI_read32(uint16_t address);
void SPI_write(uint16_t address, uint32_t *value, size_t size);
void SPI_read(uint16_t address, uint32_t *value, size_t size);


#endif
