#ifndef SSD1306_FONTS_H_
#define SSD1306_FONTS_H_

#include <linux/types.h>

typedef struct {
	const uint8_t FontWidth;
	uint8_t FontHeight; 
	const uint16_t *data;
} FontDef;

extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;

#endif