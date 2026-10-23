#include "spi.h"

#include <stdio.h>
#include <hardware/gpio.h>
#include <hardware/spi.h>

#define GPIO_SPI_CLK     2
#define GPIO_SPI_MOSI    3
#define GPIO_SPI_MISO    4
#define GPIO_SPI_CS      5

#define SPI             spi0
#define SPI_FREQUENCY   10e6


static inline void SPI_chipSelect(bool enable){
    gpio_put(GPIO_SPI_CS, enable);
}

void SPI_init(){
    spi_init(SPI, SPI_FREQUENCY);

    gpio_set_function(GPIO_SPI_CLK, GPIO_FUNC_SPI);
    gpio_set_function(GPIO_SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(GPIO_SPI_MISO, GPIO_FUNC_SPI);

    gpio_init(GPIO_SPI_CS);
    gpio_set_dir(GPIO_SPI_CS, GPIO_OUT);

    spi_set_format(SPI, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    SPI_chipSelect(false);
}

void SPI_write32(uint16_t address, uint32_t value){
    address = address << 6;

    uint8_t data[6] = {
        address >> 8,
        address >> 0,
        value >> 24,
        value >> 16,
        value >> 8,
        value >> 0,
    };

    SPI_chipSelect(true);
    spi_write_blocking(SPI, data, 6);
    SPI_chipSelect(false);
}

uint32_t SPI_read32(uint16_t address){
    address = (address << 6) | (1 << 15);
    uint8_t data[2] = {
        address >> 8,
        address >> 0,
    };
    uint8_t rec[4] = {0};

    SPI_chipSelect(true);
    spi_write_blocking(SPI, data, 2);
    spi_read_blocking(SPI, 0x00, rec, 4);
    SPI_chipSelect(false);

    return rec[3] << 24 | rec[2] << 16 | rec[1] << 8 | rec[0] << 0;
}

void SPI_write(uint16_t address, uint32_t *values, size_t size){
    address = address << 6;
    uint8_t data[2] = {
        address >> 8,
        address >> 0,
    };

    SPI_chipSelect(true);
    spi_write_blocking(SPI, data, 2);
    for (uint i = 0; i < size; i++){
        uint32_t value = values[i];
        if (i + address >= 0xC0){
            uint8_t data[4] = {
                value >> 24,
                value >> 16,
                value >>  8,
                value >>  0,
            };
            spi_write_blocking(SPI, data, 4);
        } else {
            uint8_t data[2] = {
                value >>  8,
                value >>  0,
            };
            spi_write_blocking(SPI, data, 2);
        }
        printf("SPI send: %i/%i\r", i+1, size);
    }
    printf("\n");
    SPI_chipSelect(false);
}

void SPI_read(uint16_t address, uint32_t *value, size_t size){
    address = (address << 6) | (1 << 15);
    uint8_t data[2] = {
        address >> 8,
        address >> 0,
    };
    uint8_t rec[4];

    SPI_chipSelect(true);
    spi_write_blocking(SPI, data, 2);
    for (uint i = 0; i < size; i++){
        if (i + address >= 0xC0){
            spi_read_blocking(SPI, 0x00, rec, 4);
            value[i] = ~(rec[0] << 24 | rec[1] << 16 | rec[2] << 8 | rec[3] << 0);
        } else {
            spi_read_blocking(SPI, 0x00, rec, 2);
            value[i] = ~(rec[0] << 8 | rec[1] << 0);
        }
        printf("SPI read: %i/%i\r", i+1, size);
    }
    printf("\n");
    SPI_chipSelect(false);
}
