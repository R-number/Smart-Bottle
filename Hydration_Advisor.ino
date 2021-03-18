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

char input[11];
nibbles_t unixIn;

DateTime devTime(2021, 3, 5, 16, 15, 0);
float waterTarget = 2000;
float waterVolume = 0;
float waterDrank = 0;
uint8_t waterStreak = 8;
uint8_t waterRank = 0;
bool exerciseFlag = false;
bool reminderFlag = false;
bool smile = false;
uint8_t message = 0;
int exercise = 0;

void setup() {
    setupFSR();
    setupRTC(devTime);
    setupAccel();
    setupOLED();
    setupBluetooth(); 
}

void checkbottleReset() {
    if ((rtc.now().hour() == 0) && (rtc.now().minute() == 0) && (rtc.now().second() <= 10)) {
        if (waterDrank >= waterTarget)
            waterStreak += 1;
        waterDrank = 0;
        exerciseFlag = false;
        waterTarget = 2000;
        smile = false;
        message = 0;
        exercise = 0;
    }
}

void updateOLED() {
    char drank[6];
    char target[7];
    char streak[3];
    char rank[3];
    uint8_t position[2];

    if ((rtc.now().hour() >= 9) && (rtc.now().hour() <= 21)) {
        if (waterDrank < ((float)(12 - (21 - rtc.now().hour())) * (waterTarget / 12.0))) { // Checks if on target.
            smile = false;
            message = 0;
        }
        else {
            message = 2;
            smile = true;
        }
    }

    if (exerciseFlag) { // Exercise notification and water target increase.
        exerciseFlag = false;
        waterTarget += 250;
        smile = true;
        message = 3;
        exercise = 1;
    }
    else if (waterDrank >= 8000) {
        smile = true;
        message = 1;
    }
    else if (waterDrank >= waterTarget) {
        smile = true;
        message = 4;
    }
    
    oled.fillScreen(BLACK);

    // Avatar
    oled.setCursor(0, 0);
    position[0] = 24;
    position[1] = 8;
    if (smile) {
        oled.fillCircle(position[0], position[1] + 15, 25, YELLOW);
        oled.fillCircle(position[0], position[1] + 15, 20, BLACK);
        oled.fillRect(0, 0, SCREEN_WIDTH, position[1] + 15, BLACK);
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

    oled.setTextColor(WHITE); // Message positioning.
    oled.setCursor(0, 56);

    if (message == 0) {
        oled.print("You're a little");
        oled.setCursor(0, 64);
        oled.print("behind target. Try");
        oled.setCursor(0, 72);
        oled.print("drinking a bit more.");
    }
    else if (message == 1) {
        oled.print("You're drinking a");
        oled.setCursor(0, 64);
        oled.print("lot of water. Be");
        oled.setCursor(0, 72);
        oled.print("careful not to");
        oled.setCursor(0, 80);
        oled.print("drink too much!");
    }
    else if (message == 2) {
        oled.print("You're on target,");
        oled.setCursor(0, 64);
        oled.print("well done.");
    }
    else if(message == 3){
        oled.print("Hope you enjoyed your");
        oled.setCursor(0, 64);
        oled.print("exercise! Remember");
        oled.setCursor(0, 72);
        oled.print("to drink more water.");
    }
    else {//message == 4
        oled.print("You've hit your goal");
        oled.setCursor(0, 72);
        oled.print("     Well Done!");
    }

    // Water Target
    oled.setCursor(5, SCREEN_HEIGHT - 10);
    oled.setTextColor(CYAN);
    sprintf(drank, "%04.0f", waterDrank);
    oled.print("Target: ");
    oled.print(drank);
    oled.print("/");
    sprintf(target, "%04.0f", waterTarget);
    oled.print(target);
    oled.print("mL");
    oled.drawLine(0, 112, 128, 112, CYAN);
}

void loop() {
    float input, volume; 
    static uint8_t accel, previous_accel = 0;
    static uint32_t accelInterval;
    static uint32_t displayInterval;
    static bool displayFlag = false;
    static bool tiltFlag = false;
    static bool bluetoothFlag = false;
    static uint32_t reminderInterval;

    if (millis() - accelInterval >= 2000) {// Read accelerometer and poll Bluetooth with two second intervals.
        accelInterval = millis();
        accel = readAccel();

        if (accel == 2) {
           if (previous_accel != 2) {
                volume = readFSR();
                if (volume >= 0) {
                    displayFlag = true;
                    displayInterval = millis();
                    if (volume < waterVolume)
                        waterDrank += waterVolume - volume;
                    waterVolume = volume;
                    // streak counting
                    updateOLED();
                }
                previous_accel = 2;
            }
        }
        else if (accel == 1) {
            if (previous_accel != 1) {
                tiltFlag = true;
                displayInterval = millis();
                oled.fillScreen(BLACK);
                char text[] = "Surface not level!";
                oled.setTextColor(YELLOW);
                oled.setCursor(10, SCREEN_HEIGHT - 10);
                oled.print(text);
                previous_accel = 1;
            }
        }
        else if (accel == 0) {
            if (previous_accel != 0) {
                //oled.fillScreen(BLACK);
                previous_accel = 0;
            }
        }

        bluetoothFlag = loopBluetooth();
        if (bluetoothFlag) {
            displayFlag = true;
            displayInterval = millis();
        }

        checkbottleReset();
    }
    if (reminderFlag) {
        if (millis() - reminderInterval >= 20000) {// 20 second reminder.
            reminderFlag = false;
            oled.fillScreen(BLACK);
        }
    }
    // Reminder every two hours from 9am to 9pm.
    else if (((rtc.now().hour() == 9) || (rtc.now().hour() == 11) || (rtc.now().hour() == 13) || (rtc.now().hour() == 15) || (rtc.now().hour() == 17) || (rtc.now().hour() == 19) || (rtc.now().hour() == 21)) && (rtc.now().minute() == 0)) {
        reminderFlag = true;
        reminderInterval = millis();
        updateOLED();
    }
    else if (displayFlag) {
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
    if (voltageFSR < 0.5) {
        volume = -1;
    }
    else if (voltageFSR < 1.05) {
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
        if ((X[4] == -9) && (Y[4] == 0) && (Z[4] == 0)) {
            weigh = 2;
        }
        else {
            weigh = 1;
        }
    }
    else {
        weigh = 0;
    }
    return weigh;
}

void setupFSR() {
    pinMode(pin_FSR, INPUT);
    pinMode(pin_Vin, OUTPUT);
    analogReadResolution(ADC_Word);
}

void setupRTC(DateTime currentTime) {
    rtc.begin();
    rtc.adjust(currentTime);
}

void setupAccel() {
    Wire.begin();
    if (!accel.begin())
    {
        while (1);
    }
}

void setupOLED() {
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
}

bool loopBluetooth() {
    int i = 0;
    int process = 0;
    int digit1;
    int digit2;

    while (Serial1.available() > 0) {    //Get String
        input[i] = Serial1.read();

        i = i + 1;
        if (input[i - 1] == 'Z') {
            delay(10);
            input[i] = Serial1.read();
            delay(10);
            input[i] = Serial1.read();
            process = 1;
            break;
        }
        delay(10);
    }

    if (process) { //Process String
        process = 0;
        if (input[0] == 'T') {
            unixIn.NIBBLE0 = asciiToHex(input[8]);
            unixIn.NIBBLE1 = asciiToHex(input[7]);
            unixIn.NIBBLE2 = asciiToHex(input[6]);
            unixIn.NIBBLE3 = asciiToHex(input[5]);
            unixIn.NIBBLE4 = asciiToHex(input[4]);
            unixIn.NIBBLE5 = asciiToHex(input[3]);
            unixIn.NIBBLE6 = asciiToHex(input[2]);
            unixIn.NIBBLE7 = asciiToHex(input[1]);

            rtc.adjust(unixIn.VAL);
            Serial1.write("TZ\n");
        }
        else if (input[0] == 'E') {
            if (!exercise) {
                exerciseFlag = true;
                updateOLED();
            }
            Serial1.write("EZ\n");
            return true;
        }
        else if (input[0] == 'R') {
            digit1 = asciiToHex(input[1]);
            digit2 = asciiToHex(input[2]);
            waterRank = (16 * digit1) + digit2;
            Serial1.write("RZ\n");
            updateOLED();
            return true;
        }
        else if (input[0] == 'W') {
            Serial1.write("W");
            Serial1.write(hexToAscii(waterStreak / 16));
            Serial1.write(hexToAscii(waterStreak % 16));
            Serial1.write("Z\n");
        }
    }
    return false;

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