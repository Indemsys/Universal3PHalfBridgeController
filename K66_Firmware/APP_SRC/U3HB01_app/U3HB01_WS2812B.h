#ifndef U3HB01_WS2812B_H
#define U3HB01_WS2812B_H

// цвета в формате RGB (00000000 RRRRRRRR GGGGGGGG BBBBBBBB)
#define COLOR_SKEEP      0x80000000 // Установка этого цвета пропускается
#define COLOR_NONE       0x00000000
#define COLOR_RED        0x00FF0000
#define COLOR_RED_HALF   0x00800000
#define COLOR_GREEN      0x0000FF00
#define COLOR_GREEN_HALF 0x00008000
#define COLOR_BLUE       0x000000FF
#define COLOR_WHITE      0x00FFFFFF

#define  HSV_NONE       0x0000000 // hue = 0,   sat = 000, value = 000
#define  HSV_RED        0x000FFFF // hue = 0,   sat = 255, value = 255
#define  HSV_RED_GREEN  0x03CFFFF // hue = 60,  sat = 255, value = 255
#define  HSV_GREEN      0x078FFFF // hue = 120, sat = 255, value = 255
#define  HSV_GREEN_BLUE 0x0B4FFFF // hue = 180, sat = 255, value = 255
#define  HSV_BLUE       0x0F0FFFF // hue = 240, sat = 255, value = 255
#define  HSV_BLUE_RED   0x12CFFFF // hue = 300, sat = 255, value = 255


void WS2812B_Demo_DMA(void);
void WS2812B_periodic_refresh(void);

#endif // LEDSC_WS2812B_H



