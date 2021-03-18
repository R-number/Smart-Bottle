//#include <Adafruit_ADXL343.h>
#include <SPI.h>
#include <Wire.h>
#include "OLED_Driver.h"
#include "OLED_GFX.h"
#include "RTClib.h"  
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_Sensor.h>

uint32_t FSRpin = 54;

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();


void setup() {
    Serial.begin(115200);
    setupFSR();
    setupAccel();

}

void loop() {
    float volume;
    float input;
    int weigh;

    if (Serial.available() > 0) {
        float input = Serial.parseFloat();

        weigh = loopAccel();
        if (weigh) {
            volume = readFSR();
        }
        else {
            Serial.println("Weight not measured");
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

    uint32_t readFSR;
    float voltageFSR;
    float volume;
    int R = 2000;//
    float Vcc = 3.23;//~3.3
    float fsrResistance;

    digitalWrite(52, HIGH);
    delay(500);
    readFSR = analogRead(54);
    voltageFSR = ((float)readFSR / 4095.0) * Vcc;   // Calculate voltage at A0 pin
    Serial.print("Voltage: ");
    Serial.print(voltageFSR);


    fsrResistance = ((Vcc * R) / voltageFSR) - R;   // Calculate FSR Resistance
    Serial.print("     Resistance: ");
    Serial.print(fsrResistance);

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

int loopAccel() {
    int weigh = 0;
    sensors_event_t event;

    /*          ^ Z             direction of axes of accerlerometer
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

    Serial.println(X[0]);
    Serial.println(Y[0]);
    Serial.println(Z[0]);

    for (int i = 1; i < 5; i++) {
        accel.getEvent(&event);
        X[i] = event.acceleration.x;
        Y[i] = event.acceleration.y;
        Z[i] = event.acceleration.z;

        Xdiff = X[i] - X[i - 1];            //Check if values are constant
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
        if ((X[4] == 0) && (Y[4] == 0) && (Z[4] == 9)) {    //Check if upright
            Serial.println("Upright");
            weigh = 1;
        }
        else {
            Serial.println("Tilted");
        }
    }
    else {
        Serial.println("Moving");
    }
    return weigh;
}