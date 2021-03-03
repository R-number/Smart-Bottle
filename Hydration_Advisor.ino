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

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
RTC_DS3231 rtc;
Adafruit_SSD1351 oled = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

int flag = 0;

DateTime devTime(2021, 3, 2, 22, 0, 0);
float waterTarget = 1500;
float waterVolume = 0;
float waterDrank = 0;

void setup() {
    Serial.begin(115200);
    setupFSR();
    setupRTC(devTime);
    setupAccel();
    setupOLED();
    //setupBluetooth(); 
}

void updateOLED() {
    char drank[6];
    char target[7];
    uint8_t mood_position[2];
    // 250.00/1500.00

    sprintf(drank, "%f", waterDrank);
    sprintf(target, "%f", waterTarget);

    oled.fillScreen(BLACK);
    oled.setCursor(0, 0);
    oled.setTextColor(YELLOW);
    oled.print(drank);
    oled.print("/");
    oled.print(target);
    mood_position[0] = (SCREEN_WIDTH / 2);
    mood_position[1] = (SCREEN_HEIGHT / 2);
    oled.fillCircle(mood_position[0] - 20, mood_position[1], 5, YELLOW);
    oled.fillCircle(mood_position[0] + 20, mood_position[1], 5, YELLOW);
    if (waterDrank / waterTarget >= 0.75) {
        //
    }
    else {
        //
    }
}

void loop() {
    float input, volume; 
    static uint8_t accel, previous_accel;
    static uint32_t accelInterval = 0;
    static uint32_t displayInterval;
    static bool displayFlag;

    if (Serial.available() > 0) {
        input = Serial.parseFloat();

        if (millis() - accelInterval >= 2000) {// Read accelerometer with two second intervals.
            accelInterval = millis();
            Serial.println("Reading Accelerometer");
            accel = readAccel();
            Serial.print("Finished: accel = ");
            Serial.println(accel);
        }

        if (accel == 0) {
            previous_accel = 0;
            if (previous_accel != 0)
                oled.fillScreen(BLACK);
        }
        else if (accel == 1) {
            previous_accel = 1;
            if (previous_accel != 1) {
                displayFlag = true;
                displayInterval = millis();
                oled.fillScreen(BLACK);
                char text[] = "Surface must be level!";
                oled.setTextColor(YELLOW);
                oled.setCursor(0, 0);
                oled.print(text);
            }
            if (displayFlag) {
                if (millis() - displayInterval >= 5000) {// Display message for 5 seconds.
                    displayFlag = false;
                    oled.fillScreen(BLACK);
                }
            }
        }
        else if (accel == 2) {
            previous_accel = 2;
            if (previous_accel != 2) {
                displayFlag = true;
                volume = readFSR();
                if (volume < waterVolume)
                    waterDrank += waterVolume - volume;
                waterVolume = volume;
                updateOLED();
            }
            if (displayFlag) {
                if (millis() - displayInterval >= 20000) {// Display message for 20 seconds.
                    displayFlag = false;
                    oled.fillScreen(BLACK);
                }
            }
        }
    }
}

float readFSR() {

    uint32_t readFSR, readVin;
    float voltageFSR, voltageVin;
    float volume;
    float weight;
    float mass;
    float bottleMass = 0.290;// Combined mass of bottle and puck.
    int R = 2000;
    float Vcc = 3.23;
    float fsrResistance;
    double fsrConductance;
    float constant = 11004.9;
    float fsrForce;

    digitalWrite(52, HIGH);
    delay(250);
    // readVin = analogRead(pin_Vin);
    //voltageVin = (float)(readVin / ADC_Scale);// Connect Vin to A1/D55
    readFSR = analogRead(54);
    voltageFSR = ((float)readFSR / ADC_Scale) * Vcc;

    /*fsrResistance = ((Vcc * R) / voltageFSR) - R;
    fsrConductance = 1.0 / fsrResistance;
    fsrForce = fsrConductance * constant;
    mass = (fsrForce / 9.81) - bottleMass;
    volume = mass * 1000;*/

    if (voltageFSR < 1.05) {
        volume = 0;
    }
    else if (voltageFSR <= 1.26) {
        volume = 100;
    }
    else if (voltageFSR <= 1.34) {
        volume = 200;
    }
    else if (voltageFSR <= 1.44) {
        volume = 300;
    }
    else if (voltageFSR < 1.54) {
        volume = 400;
    }
    else {
        volume = 500;
    }
    digitalWrite(52, LOW);

    return volume;
}

uint8_t readAccel() {
    sensors_event_t event;
    int x, y, z;
    int i;
    bool isStationary = true;

    accel.getEvent(&event);
    x = event.acceleration.x;
    y = event.acceleration.y;
    z = event.acceleration.z;
    for (i = 1; i < 5; i++) {
        accel.getEvent(&event);
        if ((x != event.acceleration.x) || (y != event.acceleration.y) || (z != event.acceleration.z))
            isStationary = false;
        x = event.acceleration.x;
        y = event.acceleration.y;
        z = event.acceleration.z;
        delay(100);// 100 x 5 = 500 ms read time.
    }
    if (isStationary) {
        if ((x == 0) && (y == 0) && (z == 9))
            return 2;// Stationary and Orientated
        else
            return 1;// Stationary and Tilted
    }
    else
        return 0;// Not Stationary

    /*Serial.print("X1: ");
    Serial.print(X1);
    Serial.print(", ");
    Serial.print("Y1: ");
    Serial.print(Y1);
    Serial.print(", ");
    Serial.print("Z1: ");
    Serial.print(Z1);
    Serial.print("  ");
    Serial.print("  m/s^2   ");*/
    /*          ^ Z
                |
                + - - > Y
               /
              /
            /_ X
    */
}

void setupFSR() {
    Serial.begin(9600);
    pinMode(pin_FSR, INPUT);
    pinMode(pin_Vin, OUTPUT);
    analogReadResolution(ADC_Word);
}

void setupRTC(DateTime currentTime) {
    rtc.begin();
    rtc.adjust(currentTime);
}

void setupAccel() {
    Serial.println();
    Serial.println("Accelerometer: wire.begin");
    Wire.begin();
    if (!accel.begin())
    {
        Serial.println("Valid accelerometer sensor not found!");
        while (1);
    }
}

void setupOLED() {
    Serial.print("OLED setup.");
    oled.begin();

    // You can optionally rotate the display by running the line below.
    // Note that a value of 0 means no rotation, 1 means 90 clockwise,
    // 2 means 180 degrees clockwise, and 3 means 270 degrees clockwise.
    //tft.setRotation(1);
    // NOTE: The test pattern at the start will NOT be rotated!  The code
    // for rendering the test pattern talks directly to the display and
    // ignores any rotation.
    oled.fillScreen(BLACK);
    return;
}

void setupBluetooth() {
    Serial1.begin(9600);
    pinMode(pin_LED, OUTPUT);
    Serial.println("Ready to connect\nDefualt password is 1234 or 000");
}

void loopBluetooth() {
    if (Serial1.available())
        flag = Serial1.read();
    if (flag == 1) {
        digitalWrite(pin_LED, HIGH);
        Serial.println("LED On");
    }
    else if (flag == 0) {
        digitalWrite(pin_LED, HIGH);
        Serial.println("LED Off");
    }
    //if (Serial.available())
        //Serial1.write(Serial.read());
}

/*void testlines(uint16_t color) {
    tft.fillScreen(BLACK);
    for (uint16_t x = 0; x < tft.width() - 1; x += 6) {
        tft.drawLine(0, 0, x, tft.height() - 1, color);
    }
    for (uint16_t y = 0; y < tft.height() - 1; y += 6) {
        tft.drawLine(0, 0, tft.width() - 1, y, color);
    }

    tft.fillScreen(BLACK);
    for (uint16_t x = 0; x < tft.width() - 1; x += 6) {
        tft.drawLine(tft.width() - 1, 0, x, tft.height() - 1, color);
    }
    for (uint16_t y = 0; y < tft.height() - 1; y += 6) {
        tft.drawLine(tft.width() - 1, 0, 0, y, color);
    }

    tft.fillScreen(BLACK);
    for (uint16_t x = 0; x < tft.width() - 1; x += 6) {
        tft.drawLine(0, tft.height() - 1, x, 0, color);
    }
    for (uint16_t y = 0; y < tft.height() - 1; y += 6) {
        tft.drawLine(0, tft.height() - 1, tft.width() - 1, y, color);
    }

    tft.fillScreen(BLACK);
    for (uint16_t x = 0; x < tft.width() - 1; x += 6) {
        tft.drawLine(tft.width() - 1, tft.height() - 1, x, 0, color);
    }
    for (uint16_t y = 0; y < tft.height() - 1; y += 6) {
        tft.drawLine(tft.width() - 1, tft.height() - 1, 0, y, color);
    }

}

void testdrawtext(char* text, uint16_t color) {
    tft.setCursor(0, 0);
    tft.setTextColor(color);
    tft.print(text);
}

void testfastlines(uint16_t color1, uint16_t color2) {
    tft.fillScreen(BLACK);
    for (uint16_t y = 0; y < tft.height() - 1; y += 5) {
        tft.drawFastHLine(0, y, tft.width() - 1, color1);
    }
    for (uint16_t x = 0; x < tft.width() - 1; x += 5) {
        tft.drawFastVLine(x, 0, tft.height() - 1, color2);
    }
}

void testdrawrects(uint16_t color) {
    tft.fillScreen(BLACK);
    for (uint16_t x = 0; x < tft.height() - 1; x += 6) {
        tft.drawRect((tft.width() - 1) / 2 - x / 2, (tft.height() - 1) / 2 - x / 2, x, x, color);
    }
}

void testfillrects(uint16_t color1, uint16_t color2) {
    tft.fillScreen(BLACK);
    for (uint16_t x = tft.height() - 1; x > 6; x -= 6) {
        tft.fillRect((tft.width() - 1) / 2 - x / 2, (tft.height() - 1) / 2 - x / 2, x, x, color1);
        tft.drawRect((tft.width() - 1) / 2 - x / 2, (tft.height() - 1) / 2 - x / 2, x, x, color2);
    }
}

void testfillcircles(uint8_t radius, uint16_t color) {
    for (uint8_t x = radius; x < tft.width() - 1; x += radius * 2) {
        for (uint8_t y = radius; y < tft.height() - 1; y += radius * 2) {
            tft.fillCircle(x, y, radius, color);
        }
    }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
    for (uint8_t x = 0; x < tft.width() - 1 + radius; x += radius * 2) {
        for (uint8_t y = 0; y < tft.height() - 1 + radius; y += radius * 2) {
            tft.drawCircle(x, y, radius, color);
        }
    }
}

void testtriangles() {
    tft.fillScreen(BLACK);
    int color = 0xF800;
    int t;
    int w = tft.width() / 2;
    int x = tft.height();
    int y = 0;
    int z = tft.width();
    for (t = 0; t <= 15; t += 1) {
        tft.drawTriangle(w, y, y, x, z, x, color);
        x -= 4;
        y += 4;
        z -= 4;
        color += 100;
    }
}

void testroundrects() {
    tft.fillScreen(BLACK);
    int color = 100;

    int x = 0;
    int y = 0;
    int w = tft.width();
    int h = tft.height();
    for (int i = 0; i <= 24; i++) {
        tft.drawRoundRect(x, y, w, h, 5, color);
        x += 2;
        y += 3;
        w -= 4;
        h -= 6;
        color += 1100;
        Serial.println(i);
    }
}

void tftPrintTest() {
    tft.fillScreen(BLACK);
    tft.setCursor(0, 5);
    tft.setTextColor(RED);
    tft.setTextSize(1);
    tft.println("Hello World!");
    tft.setTextColor(YELLOW);
    tft.setTextSize(2);
    tft.println("Hello World!");
    tft.setTextColor(BLUE);
    tft.setTextSize(3);
    tft.print(1234.567);
    delay(1500);
    tft.setCursor(0, 5);
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(0);
    tft.println("Hello World!");
    tft.setTextSize(1);
    tft.setTextColor(GREEN);
    tft.print(p, 6);
    tft.println(" Want pi?");
    tft.println(" ");
    tft.print(8675309, HEX); // print 8,675,309 out in HEX!
    tft.println(" Print HEX!");
    tft.println(" ");
    tft.setTextColor(WHITE);
    tft.println("Sketch has been");
    tft.println("running for: ");
    tft.setTextColor(MAGENTA);
    tft.print(millis() / 1000);
    tft.setTextColor(WHITE);
    tft.print(" seconds.");
}

void mediabuttons() {
    // play
    tft.fillScreen(BLACK);
    tft.fillRoundRect(25, 10, 78, 60, 8, WHITE);
    tft.fillTriangle(42, 20, 42, 60, 90, 40, RED);
    delay(500);
    // pause
    tft.fillRoundRect(25, 90, 78, 60, 8, WHITE);
    tft.fillRoundRect(39, 98, 20, 45, 5, GREEN);
    tft.fillRoundRect(69, 98, 20, 45, 5, GREEN);
    delay(500);
    // play color
    tft.fillTriangle(42, 20, 42, 60, 90, 40, BLUE);
    delay(50);
    // pause color
    tft.fillRoundRect(39, 98, 20, 45, 5, RED);
    tft.fillRoundRect(69, 98, 20, 45, 5, RED);
    // play color
    tft.fillTriangle(42, 20, 42, 60, 90, 40, GREEN);
}*/
