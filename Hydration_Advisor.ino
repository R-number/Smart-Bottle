//#include <Adafruit_ADXL343.h>
#include <gfxfont.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <Wire.h>
//#include "OLED_Driver.h"
//#include "OLED_GFX.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
#include "RTClib.h"  
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_Sensor.h>

int flag = 0;
int LED = 8;
uint32_t FSRpin = 54;

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();

RTC_DS3231 rtc;
DateTime currentTime(2021,3,2,13,40,0);

char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

// Screen dimensions
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 // Change this to 96 for 1.27" OLED.

// You can use any (4 or) 5 pins 
//#define SCLK_PIN 2
//#define MOSI_PIN 3
#define DC_PIN   26
#define CS_PIN   27
#define RST_PIN  29

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

Adafruit_SSD1351 oled = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

float p = 3.1415926;

void setup() {
    Serial.begin(115200);
    setupFSR();

    setupAccel();

    setupRTC();

    setupOLED();

    //setupBluetooth(); 
}

void loop() {
    /*float volume;
    float input;
    int weigh;
    DateTime time = rtc.now();

    if (Serial.available() > 0) {
        float input = Serial.parseFloat();
        /*
        weigh = loopAccel();
        if (weigh) {
            volume = readFSR();
        }
        else {
            Serial.println("Weight not measured");
        }

        //loopRTC();
        loopOLED();
    }*/

    float volume;
    float input;
    uint8_t accel;
    char text[] = "Hello World!";

    // Most likely need RTC to track display time.
    if (Serial.available() > 0) {
        input = Serial.parseFloat();
        accel = readAccel();

        if (accel == 0) {
            oled.fillScreen(BLACK);
        }
        else if (accel == 1) {
            oled.fillScreen(BLACK);
            char text[] = "Not level!";
            oled.setTextColor(BLUE);
            oled.setCursor(0, 0);
            oled.print(text);
        }
        else if (accel == 2) {
            volume = readFSR();
            oled.fillScreen(BLACK);
            char text[10];
            sprintf((char*)text, "%f", volume);
            oled.setTextColor(BLUE);
            oled.setCursor(0, 0);
            oled.print(text);
        }
    }
}

void setupFSR() {
    Serial.begin(9600);
    pinMode(54, INPUT);
    pinMode(52, OUTPUT);
    analogReadResolution(12);
}

float readFSR() {

    uint32_t readFSR, readVin;
    float voltageFSR, voltageVin;
    float volume;
    float weight;
    float mass;
    float bottleMass = 0.290;// bottle + puck
    int R = 2000;//
    float Vcc = 3.23;//~3.3
    float fsrResistance;
    double fsrConductance;
    float constant = 11004.9;
    float fsrForce;

    digitalWrite(52, HIGH);
    delay(500);
    // readVin = analogRead(pin_Vin);
    //voltageVin = (float)(readVin / ADC_Scale);// Connect Vin to A1/D55
    readFSR = analogRead(54);
    voltageFSR = ((float)readFSR / 4095.0) * Vcc;
    Serial.print("Voltage: ");
    Serial.print(voltageFSR);
    // Outputs: actual weight, Vcc, voltage, resistance, conductance, force felt, calculated volume

    //300g 2.71V
    //350g 2.75V
    //400g 2.79V
    //450g 2.83V
    //500g 2.86V

    //2k
    //1.2 to 1.8

    //Serial.print("Vcc: ");
    //Serial.print(Vcc);
    //Serial.println(" V");
    //Serial.print("Voltage: ");
    //Serial.print(voltage);
    //Serial.println(" V");
    fsrResistance = ((Vcc * R) / voltageFSR) - R;
    //Serial.print("FSR Resistance: ");
    //Serial.print(fsrResistance);
    //Serial.println(" Ohms");
    fsrConductance = 1.0 / fsrResistance;
    Serial.print("     Resistance: ");
    Serial.print(fsrResistance);
    //Serial.print("FSR Conductance: ");
    //Serial.print(fsrConductance);
    //Serial.println(" Ohms");
    //fsrForce = fsrConductance * constant;
    //Serial.print("FSR Force: ");
    //Serial.print(fsrForce);
    //Serial.println(" N");
    //mass = (fsrForce / 9.81) - bottleMass;
    //volume = mass * 1000;
    //Serial.print("Calculated Volume: ");
    //Serial.print(volume);
    //Serial.println(" mL");
    /*
    Serial.print(Vcc);
    Serial.print(", ");
    Serial.print(voltage);
    Serial.print(", ");
    Serial.print(fsrResistance);
    Serial.print(", ");
    Serial.print(fsrConductance);
    Serial.print(", ");
    Serial.print(fsrForce);
    Serial.print(", ");
    //Serial.print(weight);
    //Serial.print(", ");
    Serial.println(volume); */

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

void setupAccel() {
    Serial.println();

    //ACCEL
    Serial.println("ACCEL - wire.begin ====");
    Wire.begin();
    if (!accel.begin())
    {
        Serial.println("No valid sensor found");
        while (1);
    }
}

int readAccel() {
    int weigh = 0;
    sensors_event_t event;

    /*          ^ Z
                |
                + - - > Y
               /
              /
            /_ X
    */
    int X[5];
    int Y[5];
    int Z[5];
    int Xdiff;
    int Ydiff;
    int Zdiff;
    int stationary;

    accel.getEvent(&event);
    X[0] = event.acceleration.x;
    Y[0] = event.acceleration.y;
    Z[0] = event.acceleration.z;

    for (int i = 1; i < 5; i++) {
        accel.getEvent(&event);
        X[i] = event.acceleration.x;
        Y[i] = event.acceleration.y;
        Z[i] = event.acceleration.z;

        Xdiff = X[i] - X[i - 1];
        Ydiff = Y[i] - Y[i - 1];
        Zdiff = Z[i] - Z[i - 1];

        if (!Xdiff && !Ydiff && !Zdiff) {
            stationary = 1;
        }
        else {
            stationary = 0;
            break;
        }
        delay(400);
    }

    if (stationary) {
        Serial.print("Stationary, ");
        if ((X[4] == 0) && (Y[4] == 0) && (Z[4] == 9)) {
            Serial.println("Upright");
            weigh = 2;
        }
        else {
            Serial.println("Tilted");
            weigh = 1;
        }
    }
    else {
        Serial.println("Moving");
    }
    return weigh;
}

void setupRTC() {
    rtc.begin();
    rtc.adjust(currentTime);
    return;
}

void loopRTC() {
    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    //Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    //Serial.print(") ");
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

void setupOLED() {
    Serial.print("hello!");
    oled.begin();

    Serial.println("init");

    // You can optionally rotate the display by running the line below.
    // Note that a value of 0 means no rotation, 1 means 90 clockwise,
    // 2 means 180 degrees clockwise, and 3 means 270 degrees clockwise.
    //tft.setRotation(1);
    // NOTE: The test pattern at the start will NOT be rotated!  The code
    // for rendering the test pattern talks directly to the display and
    // ignores any rotation.

    return;
}

/*void loopOLED() {

    uint16_t time = millis();
    oled.fillRect(0, 0, 128, 128, BLUE);
    time = millis() - time;

    Serial.println(time, DEC);
    delay(1000);

    lcdTestPattern();
    delay(1000);

    oled.invert(true);
    delay(1000);
    oled.invert(false);
    delay(1000);

    oled.fillScreen(BLACK);
    testdrawtext("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ", WHITE);
    delay(1000);

    // tft print function!
    tftPrintTest();
    delay(500);

    //a single pixel
    oled.drawPixel(tft.width() / 2, tft.height() / 2, GREEN);
    delay(500);

    // line draw test
    testlines(YELLOW);
    delay(500);

    // optimized lines
    testfastlines(RED, BLUE);
    delay(500);


    testdrawrects(GREEN);
    delay(1000);

    testfillrects(YELLOW, MAGENTA);
    delay(1000);

    oled.fillScreen(BLACK);
    testfillcircles(10, BLUE);
    testdrawcircles(10, WHITE);
    delay(1000);

    testroundrects();
    delay(500);

    testtriangles();
    delay(500);

    Serial.println("done");
    delay(1000);

    return;
}*/

void setupBluetooth() {
    Serial1.begin(9600);
    pinMode(LED, OUTPUT);
    Serial.println("Ready to connect\nDefualt password is 1234 or 000");
}



void loopBluetooth() {
    if (Serial1.available())
        flag = Serial1.read();
    if (flag == 1) {
        digitalWrite(LED, HIGH);
        Serial.println("LED On");
    }
    else if (flag == 0) {
        digitalWrite(LED, HIGH);
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

/**************************************************************************/
/*!
    @brief  Renders a simple test pattern on the screen
*/
/**************************************************************************/
/*void lcdTestPattern(void)
{
    static const uint16_t PROGMEM colors[] =
    { RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA, BLACK, WHITE };

    for (uint8_t c = 0; c < 8; c++) {
        tft.fillRect(0, tft.height() * c / 8, tft.width(), tft.height() / 8,
            pgm_read_word(&colors[c]));
    }
}*/