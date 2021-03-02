#pragma once

// Pin Assignments
const uint32_t pin_FSR = 54;
const uint32_t pin_Vin = 52;
const uint16_t pin_LED = 8;

// Settings
const uint8_t ADC_Word = 12;
const uint32_t ADC_Scale = pow(2, ADC_Word) - 1;

// OLED
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 // Change this to 96 for 1.27" OLED.
#define DC_PIN   26
#define CS_PIN   27
#define RST_PIN  29
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

// Misc
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };