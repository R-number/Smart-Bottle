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
DateTime devTime(2021, 3, 2, 21, 0, 0);

int secondInterval;
int flag = 0;

void setup() {
    Serial.begin(115200);
    setupFSR();
    setupRTC(devTime);
    setupAccel();
    setupOLED();
    //setupBluetooth(); 
}

void updateOLED() {
    //
}

void loop() {
    float volume;
    float input;
    uint8_t accel;
    char text[] = "Hello World!";

    // Most likely need RTC to track display time.
    //DateTime now = rtc.now();
    //Serial.print(now.day(), DEC);
    //Serial.print(now.second(), DEC);
    if (Serial.available() > 0) {
        input = Serial.parseFloat();

        if (secondInterval - rtc.now().second() >= 2) {
            secondInterval = rtc.now().second();
            accel = readAccel();
        }
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
        delay(400);
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
    secondInterval = rtc.now().second();
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
