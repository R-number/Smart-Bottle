#include "BTComms.h"
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_Sensor.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include "Header.h"
#include "BTComms.h"

RTC_DS3231 rtc;

char input[11];
nibbles_t unixIn;

DateTime devTime(2021, 3, 2, 22, 0, 0);
uint8_t waterStreak = 240;
uint8_t waterRank = 0;

bool exerciseFlag = 0;


void setup() {
    Serial.begin(115200);
    setupRTC(devTime);
    setupBluetooth(); 
}

void loop() {

    loopBluetooth();
    
}


void setupRTC(DateTime currentTime) {
    rtc.begin();
    rtc.adjust(currentTime);
}

void setupBluetooth() {
    Serial1.begin(9600);
    Serial.println("Ready to connect\nDefualt password is 1234 or 000");
}

void loopBluetooth() {
    int i = 0;
    int process = 0;
    int digit1;
    int digit2;

    while(Serial1.available() > 0) {    //Get String
        if (i == 0) {
            Serial.println("Start");
        }
        Serial.print(i);
        Serial.print(": ");
        input[i] = Serial1.read();
        Serial.println(input[i]);
        
        i = i + 1;
        if (input[i-1] == 'Z') {
            delay(10);
            input[i] = Serial1.read();
            delay(10);
            input[i+1] = Serial1.read();
            process = 1;
            break;
        }
        delay(10);
    }
    if (process) { //Process String
        process = 0;
        if (input[0] == 'T') {  // Sync Time
            Serial.println("Nibbles:");
            unixIn.NIBBLE0 = asciiToHex(input[8]);
            unixIn.NIBBLE1 = asciiToHex(input[7]);
            unixIn.NIBBLE2 = asciiToHex(input[6]);
            unixIn.NIBBLE3 = asciiToHex(input[5]);
            unixIn.NIBBLE4 = asciiToHex(input[4]);
            unixIn.NIBBLE5 = asciiToHex(input[3]);
            unixIn.NIBBLE6 = asciiToHex(input[2]);
            unixIn.NIBBLE7 = asciiToHex(input[1]);

            Serial.print("Old Time: ");
            printTime();
            rtc.adjust(unixIn.VAL);
            Serial.print("Time Updated, Current Time: ");
            printTime();

        }
        else if (input[0] == 'E') { //Exercised
            exerciseFlag = true;
            Serial.println("Exercise flag set to true");
        }
        else if (input[0] == 'R') { //Sync Rank
            Serial.print("Old Rank: ");
            Serial.println(waterRank);
            digit1 = asciiToHex(input[1]);
            digit2 = asciiToHex(input[2]);
            waterRank = (16*digit1) + digit2;
            Serial.print("New Rank: ");
            Serial.println(waterRank);
        }
        else if (input[0] == 'W') { //Send Streak
            Serial1.write("W");
            Serial1.write(hexToAscii(waterStreak / 16));
            Serial1.write(hexToAscii(waterStreak % 16));
            Serial1.write("\n");
        }
    }
}

char hexToAscii(uint8_t d)
{
    if (d < 10)
        d += '0';
    else
        d += ('A' - 10);
    return d;
}

uint8_t asciiToHex(char c)
{
    if (c > '9')
        c -= ('A' - 10);
    else
        c -= '0';
    return c;
}

void printTime() {
    Serial.print(rtc.now().year());
    Serial.print(" ");
    Serial.print(rtc.now().month());
    Serial.print(" ");
    Serial.print(rtc.now().day());
    Serial.print(" ");
    Serial.print(rtc.now().hour());
    Serial.print(" ");
    Serial.print(rtc.now().minute());
    Serial.print(" ");
    Serial.print(rtc.now().second());
    Serial.println(" ");
    return;
}