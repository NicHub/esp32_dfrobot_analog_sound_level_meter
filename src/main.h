/**
 * E S P 3 2    —    D F R O B O T    A N A L O G    S O U N D    L E V E L    M E T E R
 *
 * Copyright (C) 2019  Nicolas Jeanmonod, ouilogique.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <Arduino.h>

// Sound meter.
#include <DFRobotAnalogSoundLevelMeter.h>
Sound_Meter sm1;
const unsigned long WAIT_MS = 100UL;
#define SOUND_SENSOR_PIN 34 // This pin read the analog voltage from the sound level meter.

// OLED.
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET 4     // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Web server
#include <WebServerApp.h>
WebServerApp wsa;
AsyncWebSocket ws("/ws");
AsyncWebServer server(80);
AsyncEventSource events("/events");

/**
 *
 */
void setupSerial()
{
    Serial.begin(BAUD_RATE);
    Serial.flush();
}

/**
 *
 */
void serialPrintForPlotInArduinoPlotter()
{
    const float OFFSET_FOR_ARDUINO_PLOTTER = 0;
    Serial.print("yAxisLowLimit:");
    Serial.print(0);
    Serial.print(",yAxisHighLimit:");
    Serial.print(100);
    Serial.print(",sound_level_db:");
    Serial.print(sm1.sound_level_db, 3);
    Serial.print(",sound_level_db_moving_average:");
    Serial.print(sm1.sound_level_db_moving_average, 3);
    Serial.print(",sound_level_db_low_pass_1:");
    Serial.print(sm1.sound_level_db_low_pass_1 + OFFSET_FOR_ARDUINO_PLOTTER * 1, 3);
    Serial.print(",sound_level_db_low_pass_2:");
    Serial.print(sm1.sound_level_db_low_pass_2 + OFFSET_FOR_ARDUINO_PLOTTER * 2, 3);
    Serial.print(",sound_level_db_kalman:");
    Serial.print(sm1.sound_level_db_kalman + OFFSET_FOR_ARDUINO_PLOTTER * 3, 3);
    Serial.print("\n");
}

/**
 *
 */
void printOLEDNumericalValue()
{
    // Reduce the printing rate.
    static unsigned long targetT = 0UL;
    unsigned long currentT = millis();
    if (currentT < targetT)
        return;
    targetT = currentT + 500UL;

    // Print.
    display.setCursor(25, 8);
    display.clearDisplay();
    display.print(sm1.sound_level_db_kalman, 1);
    display.print(" dB");
    display.display();
}

/**
 *
 */
void printOLEDVuMeter()
{
    // Reduce the printing rate.
    static unsigned long targetT = 0UL;
    unsigned long currentT = millis();
    if (currentT < targetT)
        return;
    targetT = currentT + 500UL;

    // Print.
    display.setTextSize(2);
    display.setCursor(25, 8);
    display.clearDisplay();
    display.print(sm1.sound_level_db_kalman, 1);
    display.print(" dB");
    display.display();
    display.fillRect(0,
                     display.height() * (1 - sm1.sound_level_db_kalman / 100),
                     10,
                     display.height(),
                     SSD1306_INVERSE);
    display.display();
}

/**
 *
 */
void setupOLED()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        delay(1000);
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
}

/**
 *
 */
void wsPrint(char *json_msg)
{
    ws.textAll(json_msg);
}

/**
 *
 */
void printOLED_IPs()
{
    // Reduce the printing rate.
    static unsigned long targetT = 0UL;
    unsigned long currentT = millis();
    if (currentT < targetT)
        return;
    targetT = currentT + 500UL;

    // OLED setup.
    display.clearDisplay();
    display.setTextSize(1);

    // Print station IP.
    display.setCursor(5, 0);
    display.print("STATION IP");
    display.setCursor(15, 8);
    display.print(wsa.get_station_ip());

    // Print soft IP.
    display.setCursor(5, 16);
    display.print("SOFT AP IP");
    display.setCursor(15, 24);
    display.print(wsa.get_soft_ap_ip());

    // Print OLED.
    display.display();
    delay(5000);
}

/**
 *
 */
void setupWebServer()
{
    // Web
    wsa.scanNetwork();
    wsa.setupWebServer();
    Serial.print("STATION SSID:\n");
    Serial.println(wsa.get_ssid());
    Serial.print("SOFT AP SSID:\n");
    Serial.println(wsa.get_ap_ssid());
    printOLED_IPs();
}
