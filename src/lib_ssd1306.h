

#ifndef SSD1306_H_
#define SSD1306_H_

#include <linux/types.h>
#include "ssd1306_fonts.h"

#define SSD1306_ADDR (0x3C)

#define SSD1306_HEIGHT          (32)
#define SSD1306_WIDTH           (128)
#define SSD1306_WHITE           (0xFF)
#define SSD1306_BLACK           (0)

void SSD1306_init(void);
void SSD1306_clear(uint8_t color);
void SSD1306_refresh(void);
void SSD1306_pixel_draw(uint8_t x, uint8_t y, uint8_t color);
char SSD1306_putc(char ch, FontDef Font, uint8_t color);
char SSD1306_print(char* str, FontDef Font, uint8_t color);
void SSD1306_cursor_set(uint8_t x, uint8_t y);


#endif