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
float waterTarget = 2000;
float waterVolume = 0;
float waterDrank = 0;
uint8_t waterStreak = 0;
uint8_t waterRank = 0;

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
    char streak[3];
    char rank[3];
    uint8_t position[2];

    oled.fillScreen(BLACK);

    // Avatar
    oled.setCursor(0, 0);
    position[0] = 24;
    position[1] = 8;
    if (waterDrank / waterTarget >= 0.75) {// 75% of target met. Should it be 100%?
        oled.fillCircle(position[0], position[1] + 40, 25, YELLOW);
        oled.fillCircle(position[0], position[1] + 40, 20, BLACK);
        oled.fillRect(0, 0, SCREEN_WIDTH, position[1] - 25, BLACK);
    }
    else {
        oled.fillCircle(position[0], position[1] + 40, 25, YELLOW);
        oled.fillCircle(position[0], position[1] + 40, 20, BLACK);
        oled.fillRect(0, position[1] + 25, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
    }
    oled.fillCircle(position[0] - 12, position[1], 7, YELLOW);
    oled.fillCircle(position[0] + 12, position[1], 7, YELLOW);
    //oled.fillCircle(mood_position[0], mood_position[1] + 20, 55, YELLOW);
    //oled.fillCircle(mood_position[0], mood_position[1] + 20, 50, BLACK);

    // Water Target
    oled.setCursor(5, SCREEN_HEIGHT-10);
    oled.setTextColor(CYAN);
    sprintf(drank, "%04.0f", waterDrank);
    oled.print("Target: ");
    oled.print(drank);
    oled.print("/");
    sprintf(target, "%04.0f", waterTarget);
    oled.print(target);
    oled.print(" mL");
    oled.drawLine(0, 112, 128, 112, CYAN);

    // Streak
    oled.setCursor(55, 0);
    oled.setTextColor(MAGENTA);
    sprintf(streak, "%03u", waterStreak);
    oled.print("Streak - ");
    oled.print(streak);

    // Rank
    oled.setCursor(55, 15);
    sprintf(rank, "%03u", waterRank);
    oled.print("Rank   - ");
    oled.print(rank);
}

void loop() {
    float input, volume; 
    static uint8_t accel, previous_accel = 0;
    static uint32_t accelInterval;
    static uint32_t displayInterval;
    static bool displayFlag = false;
    static bool tiltFlag = false;

    if (millis() - accelInterval >= 2000) {// Read accelerometer with two second intervals.
        accelInterval = millis();
        accel = readAccel();
        Serial.print("accel = ");
        Serial.println(accel);
    }

    if (displayFlag) {
        if (millis() - displayInterval >= 20000) {// Display message for 20 seconds.
            displayFlag = false;
            oled.fillScreen(BLACK);
        }
    }
    else if (tiltFlag) {
        if (millis() - displayInterval >= 5000) {// Display message for 5 seconds.
            tiltFlag = false;
            oled.fillScreen(BLACK);
        }
    }
    else if (accel == 0) {
        if (previous_accel != 0)
            oled.fillScreen(BLACK);
        previous_accel = 0;
    }
    else if (accel == 1) {
        if (previous_accel != 1) {
            tiltFlag = true;
            displayInterval = millis();
            oled.fillScreen(BLACK);
            char text[] = "Surface not level!";
            oled.setTextColor(YELLOW);
            oled.setCursor(10, SCREEN_HEIGHT-10);
            oled.print(text);
        }
        previous_accel = 1;
    }
    else if (accel == 2) {
        if (previous_accel != 2) {
            displayFlag = true;
            displayInterval = millis();
            volume = readFSR();
            if (volume < waterVolume)
                waterDrank += waterVolume - volume;
            waterVolume = volume;
            updateOLED();
        }
        previous_accel = 2;
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
        delay(200);
    }

    if (stationary) {
        //Serial.print("Stationary, ");
        if ((X[4] == 0) && (Y[4] == 0) && (Z[4] == 9)) {
            //Serial.println("Upright");
            weigh = 2;
        }
        else {
            //Serial.println("Tilted");
            weigh = 1;
        }
    }
    else {
        //Serial.println("Moving");
        weigh = 0;
    }
    return weigh;
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
