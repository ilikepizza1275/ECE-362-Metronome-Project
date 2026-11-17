#ifndef SD_INTERFACE_H
#define SD_INTERFACE_H

void init_spi_sdcard(void);
void disable_sdcard(void);
void enable_sdcard(void);
void sdcard_io_high_speed(void);
void init_sdcard_io(void);

#endif