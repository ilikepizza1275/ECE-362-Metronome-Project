#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "sd_interface.h"

#define SD_MISO 16
#define SD_CS   17
#define SD_SCK  18
#define SD_MOSI 19

#define SD_SPI spi0

void init_spi_sdcard(void) {
    //sets SCK, MOSI, and MISO to use SPI
    gpio_set_function(SD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SD_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SD_MISO, GPIO_FUNC_SPI);

    //sets CS as a normal GPIO output
    gpio_init(SD_CS);
    gpio_set_dir(SD_CS, GPIO_OUT);

    //sets CS high so the SD card starts inactive
    gpio_put(SD_CS, 1);

    //starts SPI at 400 kHz for SD card initialization
    spi_init(SD_SPI, 400000);

    //sets SPI to 8 bit, CPOL 0, CPHA 0
    spi_set_format(SD_SPI, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

void disable_sdcard(void) {
    uint8_t data = 0xFF;

    //sets CS high to disable the SD card
    gpio_put(SD_CS, 1);

    //sends 0xFF to give the SD card extra clock cycles
    spi_write_blocking(SD_SPI, &data, 1);

    //temporarily changes MOSI back to GPIO
    gpio_set_function(SD_MOSI, GPIO_FUNC_SIO);
    gpio_set_dir(SD_MOSI, GPIO_OUT);

    //holds MOSI high while SD card is inactive
    gpio_put(SD_MOSI, 1);
}

void enable_sdcard(void) {
    //gives SPI control of MOSI again
    gpio_set_function(SD_MOSI, GPIO_FUNC_SPI);

    //sets CS low to enable the SD card
    gpio_put(SD_CS, 0);
}

void sdcard_io_high_speed(void) {
    //changes SPI speed to 12 MHz after initialization
    spi_set_baudrate(SD_SPI, 12000000);
}

void init_sdcard_io(void) {
    //initializes the SPI pins and settings
    init_spi_sdcard();

    //starts with the SD card disabled
    disable_sdcard();
}