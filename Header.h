#pragma once

//Bluetooth
#define B_EN 25
#define B_STATE 23

typedef union
{
    uint32_t VAL;
    struct
    {
        unsigned NIBBLE0 : 4;
        unsigned NIBBLE1 : 4;
        unsigned NIBBLE2 : 4;
        unsigned NIBBLE3 : 4;
        unsigned NIBBLE4 : 4;
        unsigned NIBBLE5 : 4;
        unsigned NIBBLE6 : 4;
        unsigned NIBBLE7 : 4;
    };
} nibbles_t;

// Misc
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };