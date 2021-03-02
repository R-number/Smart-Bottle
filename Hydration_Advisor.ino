
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_Sensor.h>
#include <gfxfont.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include "Header.h"

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
RTC_DS3231 rtc;

void setup() {
    Serial.begin(115200);
    setupFSR();

    //setupRTC();
    setupAccel();
    //setupOLED();
    //setupBluetooth(); 
}

float readFSR() {

    uint32_t readFSR, readVin;
    float voltageFSR, voltageVin;
    float volume;
    float weight;
    float mass;
    float bottleMass = 0.290;// Bottle + Puck
    int R = 2000;
    float Vcc = 3.23;
    float fsrResistance;
    double fsrConductance;
    float constant = 11004.9;
    float fsrForce;

    digitalWrite(52, HIGH);
    delay(500);
    // readVin = analogRead(pin_Vin);
    //voltageVin = (float)(readVin / ADC_Scale);// Connect Vin to A1/D55
    readFSR = analogRead(54);
    voltageFSR = ((float)readFSR / ADC_Scale) * Vcc;
    fsrResistance = ((Vcc * R) / voltageFSR) - R;
    fsrConductance = 1.0 / fsrResistance;

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

    Serial.print("      Volume: ");
    Serial.println(volume);
    digitalWrite(52, LOW);
    delay(500);

    return volume;
}

void loop() {
    uint32_t readFSR, readVin;
    float voltageFSR, voltageVin;
    float volume;
    float weight;

    while (true) {
        if (Serial.available() > 0) {
            weight = Serial.parseFloat();
            Serial.print(weight);
            Serial.print(", ");
            digitalWrite(pin_Vin, HIGH);
            delay(500);
            // readVin = analogRead(pin_Vin);
            //voltageVin = (float)(readVin / ADC_Scale);// Connect Vin to A1/D55
            readFSR = analogRead(pin_FSR);
            voltageFSR = ((float)readFSR / (float)ADC_Scale) * 3.23;
            //Serial.println(voltageFSR);
            // Outputs: actual weight, Vcc, voltage, resistance, conductance, force felt, calculated volume
            volume = mapFSR(voltageFSR, 3.23);
            digitalWrite(pin_Vin, LOW);
            delay(500);
        }
    }
}

void setupFSR() {
    Serial.begin(9600);
    pinMode(pin_FSR, INPUT);
    pinMode(pin_Vin, OUTPUT);
    analogReadResolution(12);
}

void setupRTC() {
    //SDA1, SCL1
    //pinMode(?, ?)
    //pinMode(?, ?)
}

void setupBluetooth() {
    Serial1.begin(9600);
    pinMode(pin_LED, OUTPUT);
    Serial.println("Ready to connect\nDefualt password is 1234 or 000");
}

void setupAccel() {
    Serial.println();

    //ACCEL
    Serial.println("ACCEL - wire.begin ====");
    Wire.begin();//I2C 0, SCL SDA
    if (!accel.begin())
    {
        Serial.println("No valid sensor found");
        while (1);
    }
}

void setupOLED() {
    //Init GPIO
    pinMode(oled_cs, OUTPUT);
    pinMode(oled_rst, OUTPUT);
    pinMode(oled_dc, OUTPUT);

#if INTERFACE_4WIRE_SPI
    //Init SPI
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.begin();

#elif INTERFACE_3WIRE_SPI
    pinMode(oled_sck, OUTPUT);
    pinMode(oled_din, OUTPUT);

#endif

    oled.Device_Init();

    oled.Display_Interface();
    delay(3000);

    oled.Clear_Screen();
    uint8_t text[] = "Hello World!";
    oled.Set_Color(BLUE);
    oled.print_String(20, 50, text, FONT_5X8);
    delay(2000);
    oled.Clear_Screen();

    oled.Set_Color(WHITE);
    oled.Draw_Pixel(50, 50);
    delay(1000);

    lcdTestPattern();
    delay(1000);

    testlines();
    delay(1000);

    testfastlines();
    delay(1000);

    testdrawrects();
    delay(1000);

    testfillrects(BLUE, YELLOW);
    delay(1000);


    oled.Clear_Screen();
    testfillcircles(63, BLUE);
    delay(500);
    testdrawcircles(WHITE);
    delay(1000);

    testroundrects();
    delay(1000);

    testtriangles();
    delay(1000);

    // oled.Fill_Color(BLUE);
}

void loopAccel() {
    sensors_event_t event;
    accel.getEvent(&event);
    Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("");
    Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("");
    Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("");
    Serial.println("m/s^2 ");
    delay(500);
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

void loopRTC() {
    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    //Serial.print(" since midnight 1/1/1970 = ");
    //Serial.print(now.unixtime());
    //Serial.print("s = ");
    //Serial.print(now.unixtime() / 86400L);
    //Serial.println("d");
}

// OLED Functions
void testlines(void) {

    oled.Set_Color(RED);
    oled.Clear_Screen();
    for (uint16_t x = 0; x <= SSD1351_WIDTH - 1; x += 6) {
        oled.Draw_Line(0, 0, x, SSD1351_HEIGHT - 1);
        delay(10);
    }
    for (uint16_t y = 0; y < SSD1351_HEIGHT - 1; y += 6) {
        oled.Draw_Line(0, 0, SSD1351_WIDTH - 1, y);
        delay(10);
    }

    oled.Set_Color(YELLOW);
    oled.Clear_Screen();
    for (uint16_t x = 0; x < SSD1351_WIDTH - 1; x += 6) {
        oled.Draw_Line(SSD1351_WIDTH - 1, 0, x, SSD1351_HEIGHT - 1);
        delay(10);
    }
    for (uint16_t y = 0; y < SSD1351_HEIGHT - 1; y += 6) {
        oled.Draw_Line(SSD1351_WIDTH - 1, 0, 0, y);
        delay(10);
    }

    oled.Set_Color(BLUE);
    oled.Clear_Screen();
    for (uint16_t x = 0; x < SSD1351_WIDTH - 1; x += 6) {
        oled.Draw_Line(0, SSD1351_HEIGHT - 1, x, 0);
        delay(10);
    }
    for (uint16_t y = 0; y < SSD1351_HEIGHT - 1; y += 6) {
        oled.Draw_Line(0, SSD1351_HEIGHT - 1, SSD1351_WIDTH - 1, y);
        delay(10);
    }

    oled.Set_Color(GREEN);
    oled.Clear_Screen();
    for (uint16_t x = 0; x < SSD1351_WIDTH - 1; x += 6) {
        oled.Draw_Line(SSD1351_WIDTH - 1, SSD1351_HEIGHT - 1, x, 0);
        delay(10);
    }
    for (uint16_t y = 0; y < SSD1351_HEIGHT - 1; y += 6) {
        oled.Draw_Line(SSD1351_WIDTH - 1, SSD1351_HEIGHT - 1, 0, y);
        delay(10);
    }
}

void lcdTestPattern(void)
{
    uint32_t i, j;
    oled.Set_Coordinate(0, 0);

    for (i = 0; i < 128; i++) {
        for (j = 0; j < 128; j++) {
            if (i < 16) {
                oled.Set_Color(RED);
                oled.Write_Data(color_byte[0]);
                oled.Write_Data(color_byte[1]);
            }
            else if (i < 32) {
                oled.Set_Color(YELLOW);
                oled.Write_Data(color_byte[0]);
                oled.Write_Data(color_byte[1]);
            }
            else if (i < 48) {
                oled.Set_Color(GREEN);
                oled.Write_Data(color_byte[0]);
                oled.Write_Data(color_byte[1]);
            }
            else if (i < 64) {
                oled.Set_Color(CYAN);
                oled.Write_Data(color_byte[0]);
                oled.Write_Data(color_byte[1]);
            }
            else if (i < 80) {
                oled.Set_Color(BLUE);
                oled.Write_Data(color_byte[0]);
                oled.Write_Data(color_byte[1]);
            }
            else if (i < 96) {
                oled.Set_Color(MAGENTA);
                oled.Write_Data(color_byte[0]);
                oled.Write_Data(color_byte[1]);
            }
            else if (i < 112) {
                oled.Set_Color(BLACK);
                oled.Write_Data(color_byte[0]);
                oled.Write_Data(color_byte[1]);
            }
            else {
                oled.Set_Color(WHITE);
                oled.Write_Data(color_byte[0]);
                oled.Write_Data(color_byte[1]);
            }
        }
    }
}

void testfastlines(void) {

    oled.Set_Color(WHITE);
    oled.Clear_Screen();

    for (uint16_t y = 0; y < SSD1351_WIDTH - 1; y += 5) {
        oled.Draw_FastHLine(0, y, SSD1351_WIDTH - 1);
        delay(10);
    }
    for (uint16_t x = 0; x < SSD1351_HEIGHT - 1; x += 5) {
        oled.Draw_FastVLine(x, 0, SSD1351_HEIGHT - 1);
        delay(10);
    }
}

void testdrawrects(void) {
    oled.Clear_Screen();
    for (uint16_t x = 0; x < SSD1351_HEIGHT - 1; x += 6) {
        oled.Draw_Rect((SSD1351_WIDTH - 1) / 2 - x / 2, (SSD1351_HEIGHT - 1) / 2 - x / 2, x, x);
        delay(10);
    }
}

void testfillrects(uint16_t color1, uint16_t color2) {

    uint16_t x = SSD1351_HEIGHT - 1;
    oled.Clear_Screen();
    oled.Set_Color(color1);
    oled.Set_FillColor(color2);
    for (; x > 6; x -= 6) {
        oled.Fill_Rect((SSD1351_WIDTH - 1) / 2 - x / 2, (SSD1351_HEIGHT - 1) / 2 - x / 2, x, x);
        oled.Draw_Rect((SSD1351_WIDTH - 1) / 2 - x / 2, (SSD1351_HEIGHT - 1) / 2 - x / 2, x, x);
    }
}

void testfillcircles(uint8_t radius, uint16_t color) {

    oled.Set_Color(color);

    oled.Fill_Circle(64, 64, radius);
}


void testdrawcircles(uint16_t color) {

    uint8_t r = 0;
    oled.Set_Color(color);

    for (; r < SSD1351_WIDTH / 2; r += 4) {
        oled.Draw_Circle(64, 64, r);
        delay(10);
    }
}

void testroundrects(void) {

    int color = 100;
    int x = 0, y = 0;
    int w = SSD1351_WIDTH - 1, h = SSD1351_HEIGHT - 1;

    oled.Clear_Screen();

    for (int i = 0; i <= 20; i++) {

        oled.Draw_RoundRect(x, y, w, h, 5);
        x += 2;
        y += 3;
        w -= 4;
        h -= 6;
        color += 1100;
        oled.Set_Color(color);
    }
}

void testtriangles(void) {
    oled.Clear_Screen();
    int color = 0xF800;
    int t;
    int w = SSD1351_WIDTH / 2;
    int x = SSD1351_HEIGHT - 1;
    int y = 0;
    int z = SSD1351_WIDTH;
    for (t = 0; t <= 15; t += 1) {
        oled.Draw_Triangle(w, y, y, x, z, x);
        x -= 4;
        y += 4;
        z -= 4;
        color += 100;
        oled.Set_Color(color);
    }
}