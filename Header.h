#pragma once

// Pin Assignments
const uint32_t pin_FSR = 54;
const uint32_t pin_Vin = 52;
const uint16_t pin_LED = 8;

// Settings
const uint8_t ADC_Word = 12;
const uint32_t ADC_Scale = pow(2, ADC_Word) - 1;

// Misc
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };