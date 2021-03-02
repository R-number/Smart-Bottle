//#include <Adafruit_ADXL343.h>
#include <SPI.h>
#include <Wire.h>
#include "OLED_Driver.h"
#include "OLED_GFX.h"
#include "RTClib.h"  
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_Sensor.h>

int flag = 0;
int LED = 8;
uint32_t FSRpin = 54;

OLED_GFX oled = OLED_GFX();

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();

RTC_DS3231 rtc;
DateTime currentTime(2021,3,2,13,40,0);

char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

void setup() {
    Serial.begin(115200);
    setupFSR();

    setupAccel();

    setupRTC();

    setupOLED();

    //setupBluetooth(); 
}

void loop() {
    float volume;
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
        }*/

        //loopRTC();
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

int loopAccel() {
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
    uint8_t text[] = "Hello World !";
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