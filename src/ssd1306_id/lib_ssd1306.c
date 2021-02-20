#include "lib_ssd1306.h"
#include "ssd1306_fonts.h"

#include <linux/i2c.h>

extern struct i2c_client* ssd1306_i2c_client;

uint8_t current_x;
uint8_t current_y;
uint8_t buffer[SSD1306_HEIGHT*SSD1306_WIDTH/8 + 1] = {0};

static void SSD1306_cmd_send(uint8_t cmd)
{
    char cmd_buff[2] = {};

    cmd_buff[0] = 0x00;
    cmd_buff[1] = cmd;

    i2c_master_send(ssd1306_i2c_client, cmd_buff, 2);
}

static void SSD1306_data_send(uint8_t* data, size_t data_size)
{
    data[0] = 0x40;
    i2c_master_send(ssd1306_i2c_client, data, data_size);
}

void SSD1306_clear(uint8_t color)
{
	uint16_t index = 0;

	for(index = 1 ; index <= 128*32/8 ; index++){
		buffer[index] = color;
	}
}

void SSD1306_refresh(void)
{
	SSD1306_cmd_send(0x21);
	SSD1306_cmd_send(0x00);
	SSD1306_cmd_send(127);
	SSD1306_cmd_send(0x22);
	SSD1306_cmd_send(0);
	SSD1306_cmd_send(3);

	SSD1306_data_send(buffer, SSD1306_HEIGHT * SSD1306_WIDTH / 8 + 1);
}

void SSD1306_init(void)
{
    SSD1306_cmd_send(0xAE);
    SSD1306_cmd_send(0xD5);
    SSD1306_cmd_send(0x80);
    SSD1306_cmd_send(0xA8);
    SSD1306_cmd_send(0x1F);
    SSD1306_cmd_send(0xDF);
    SSD1306_cmd_send(0x00);
    SSD1306_cmd_send(0x40);
    SSD1306_cmd_send(0x8D);
    SSD1306_cmd_send(0x14);
    SSD1306_cmd_send(0x20);
    SSD1306_cmd_send(0x00);
    SSD1306_cmd_send(0xA1);
    SSD1306_cmd_send(0xC8);
    SSD1306_cmd_send(0xDA);
    SSD1306_cmd_send(0x02);
    SSD1306_cmd_send(0x81);
    SSD1306_cmd_send(0x2F);
    SSD1306_cmd_send(0xD9);
    SSD1306_cmd_send(0xF1);
    SSD1306_cmd_send(0xDB);
    SSD1306_cmd_send(0x40);
    SSD1306_cmd_send(0x2E);
    SSD1306_cmd_send(0xA4);
    SSD1306_cmd_send(0xA6);
    SSD1306_cmd_send(0x8D);
    SSD1306_cmd_send(0x14);
    SSD1306_cmd_send(0xAF);

    SSD1306_clear(SSD1306_BLACK);
    SSD1306_refresh();
}

void SSD1306_pixel_draw(uint8_t x, uint8_t y, uint8_t color) {
    if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        // Don't write outside the buffer
        return;
    }

    // Draw in the right color
    if(color == SSD1306_WHITE) {
        buffer[1 + x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    } else {
        buffer[1 + x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

char SSD1306_putc(char ch, FontDef Font, uint8_t color) {
    uint32_t i, b, j;

    // Check remaining space on current line
    if (SSD1306_WIDTH <= (current_x + Font.FontWidth) && SSD1306_HEIGHT <= (current_y + Font.FontHeight))
    {
    	current_x = 0;
    	current_y = 0;
	}
	else if(SSD1306_WIDTH <= (current_x + Font.FontWidth)){
		current_x = 0;
		current_y += Font.FontHeight;
	}

    // Use the font to write
    for(i = 0; i < Font.FontHeight; i++) {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for(j = 0; j < Font.FontWidth; j++) {
            if((b << j) & 0x8000)  {
                SSD1306_pixel_draw(current_x + j, (current_y + i),color);
            } else {
                SSD1306_pixel_draw(current_x + j, (current_y + i),~color);
            }
        }
    }

    // The current space is now taken
    current_x += Font.FontWidth;

    // Return written char for validation
    return ch;
}

// Write full string to screenbuffer
char SSD1306_print(char* str, FontDef Font, uint8_t color) {
    // Write until null-byte
    while (*str) {
        if (SSD1306_putc(*str, Font, color) != *str) {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}

// Position the cursor
void SSD1306_cursor_set(uint8_t x, uint8_t y) {
    current_x = x;
    current_y = y;
}