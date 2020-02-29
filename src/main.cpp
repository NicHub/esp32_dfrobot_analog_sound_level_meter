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

#include <Arduino.h>

// SOUND METER.
// ~/.platformio/packages/framework-arduinoespressif32/cores/esp32/esp32-hal-adc.h
#define SOUND_SENSOR_PIN 34                         // This pin read the analog voltage from the sound level meter.
#define VREF 3.3f                                   // Voltage on AREF pin,default:operating voltage.
#define NB_BITS 12                                  // Analog resolution.
#define MAX_READING_VAL 4095.0f                     // = 2 ^ NB_BITS
#define CONV_ANALOG_2_VOLT (VREF / MAX_READING_VAL) // Conversion between analog read and voltage.
#define CONV_VOLT_2_DB 50.0f                        // Conversion between voltage and decibel.
const uint16_t wait_ms = 100;
const uint16_t integrationTime_ms = 500;
const uint16_t movingAverageSize = integrationTime_ms / wait_ms;
float analogReadValues[movingAverageSize];
float analogReadValueMax = 0.0f;
float analogReadValueMin = MAX_READING_VAL;
float analogReadValue = 0.0f;
float voltageValue = 0.0f;
float dbValue = 0.0f;
float analogReadValueAveraged = 0.0f;
float voltageValueAveraged = 0.0f;
float dbValueAveraged = 0.0f;
float dbValueLowPass = 0.0f;
float lastdbValue = 0.0f;
float lastdbValueLowPass = 0.0f;
float dbValueLowPass2 = 0.0f;
uint8_t cnt = 0;

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
void printCompilationDateAndTime()
{
    Serial.print("###\ncompilation date and time:\n");
    Serial.print(COMPILATION_DATE);
    Serial.print("\n");
    Serial.print(COMPILATION_TIME);
    Serial.print("\n###\n\n");
}

/**
 *
 */
void initArrayForMovingAverage()
{
    for (size_t _i = 0; _i < movingAverageSize; _i++)
    {
        analogReadValues[_i] = analogReadValue;
    }
}

/**
 *
 */
void readSoundLevel()
{

    // Read sound level.
    analogReadValue = analogRead(SOUND_SENSOR_PIN);
    voltageValue = analogReadValue * CONV_ANALOG_2_VOLT;
    dbValue = voltageValue * CONV_VOLT_2_DB;

    // Get min and max analogReadValue.
    if (analogReadValueMax < analogReadValue)
        analogReadValueMax = analogReadValue;
    if (analogReadValueMin > analogReadValue)
        analogReadValueMin = analogReadValue;

    // Reset moving average in case of big difference.
    if (abs(lastdbValue - dbValue) > 10)
        initArrayForMovingAverage();

    // Calculate moving average;
    analogReadValues[cnt] = analogReadValue;
    float sum = 0;
    for (size_t _i = 0; _i < movingAverageSize; _i++)
    {
        sum += analogReadValues[_i];
    }
    analogReadValueAveraged = sum / movingAverageSize;
    voltageValueAveraged = analogReadValueAveraged * CONV_ANALOG_2_VOLT;
    dbValueAveraged = voltageValueAveraged * CONV_VOLT_2_DB;
    cnt++;
    if (cnt == movingAverageSize)
        cnt = 0;

    // Low pass.
    if (abs(lastdbValue - dbValue) < 2)
    {
        dbValueLowPass = lastdbValueLowPass;
    }
    else
    {
        lastdbValueLowPass = dbValue;
        dbValueLowPass = dbValue;
    }

    // Low pass 2.
    const float LPF_Beta = 0.1;
    dbValueLowPass2 = dbValueLowPass2 - (LPF_Beta * (dbValueLowPass2 - dbValue));

    // Store last dbValue.
    lastdbValue = dbValue;
}

/**
 *
 */
void serialPrint()
{
    // Display analog read values.
    if (analogReadValue < 9.5f)
        Serial.print("   ");
    else if (analogReadValue < 99.5f)
        Serial.print("  ");
    else if (analogReadValue < 999.5f)
        Serial.print(" ");
    Serial.print(analogReadValue, 0);

    // Display voltage values.
    Serial.print("  |  ");
    Serial.print(voltageValue);
    Serial.print(" V");

    // Display sound level values.
    Serial.print("  |  ");
    if (dbValue < 9.5f)
        Serial.print("  ");
    else if (dbValue < 99.5f)
        Serial.print(" ");
    Serial.print(dbValue, 1);
    Serial.print(" dBA");

    // Display analogReadValueMin.
    Serial.print("  |  ");
    if (analogReadValueMin < 9.5f)
        Serial.print("   ");
    else if (analogReadValueMin < 99.5f)
        Serial.print("  ");
    else if (analogReadValueMin < 999.5f)
        Serial.print(" ");
    Serial.print(analogReadValueMin, 0);

    // Display analogReadValueMax.
    Serial.print("  |  ");
    if (analogReadValueMax < 9.5f)
        Serial.print("   ");
    else if (analogReadValueMax < 99.5f)
        Serial.print("  ");
    else if (analogReadValueMax < 999.5f)
        Serial.print(" ");
    Serial.print(analogReadValueMax, 0);

    // Display hallRead()
    float hallValue = hallRead();
    Serial.print("  |  hall ");
    Serial.print(hallValue);

    // New line.
    Serial.print("\n");
}

/**
 *
 */
void serialPrintForPlot()
{
    const float offsetForArduinoPlotter = 0;
    Serial.print("yAxisLowLimit:");
    Serial.print(0);
    Serial.print(",yAxisHighLimit:");
    Serial.print(100);
    Serial.print(",dbValue:");
    Serial.print(dbValue, 3);
    Serial.print(",dbValueAveraged:");
    Serial.print(dbValueAveraged, 3);
    Serial.print(",dbValueLowPass:");
    Serial.print(dbValueLowPass + offsetForArduinoPlotter, 3);
    Serial.print(",dbValueLowPass2:");
    Serial.print(dbValueLowPass2 + offsetForArduinoPlotter * 2, 3);
    Serial.print("\n");
}

/**
 *
 */
void oledPrint()
{
    // Reduce the display rate;
    static int lastT = millis();
    int currentT = millis();
    if (currentT - lastT < 250)
        return;
    lastT = currentT;

    // OLED.
    display.setCursor(25, 8);
    display.clearDisplay();
    display.print(dbValueAveraged, 1);
    display.print(" dB");
    display.display();
}

/**
 *
 */
void wsPrint()
{
    static char jsonMsg[100];
    sprintf(jsonMsg, R"rawText({"dbValue":%f,"dbValueAveraged":%f})rawText",
            dbValue, dbValueAveraged);
    // Serial.print(jsonMsg);
    // Serial.print("\n");
    ws.textAll(jsonMsg);
}

/**
 *
 */
void setup()
{
    Serial.begin(115200);
    Serial.flush();
    printCompilationDateAndTime();

    // SOUND METER.
    analogReadResolution(NB_BITS);                       // Resolution of analogRead return values. Range: 9 - 12. Default: 12 bits (0-4095).
    analogSetWidth(NB_BITS);                             // Sets the sample bits and read resolution. Range: 9 - 12. Default: 12 bits (0-4095).
    analogSetCycles(8);                                  // Set number of cycles per sample. Range: 1 - 255. Default: 8.
    analogSetSamples(1);                                 // Set number of samples in the range. Range: 1 - 255. Default: 1.
    analogSetClockDiv(1);                                // Set the divider for the ADC clock. Range: 1 - 255. Default. 1.
    analogSetAttenuation(ADC_11db);                      // Set the attenuation for all channels. Can be ADC_0db, ADC_2_5db, ADC_6db, ADC_11db. Default: ADC_11db.
    analogSetPinAttenuation(SOUND_SENSOR_PIN, ADC_11db); // Set the attenuation for particular pin.

    // OLED.
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        delay(1000);
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    // Initial reading to fill the array for the moving average.
    readSoundLevel();
    initArrayForMovingAverage();

    // Web
    wsa.scanNetwork();
    wsa.setupWebServer();
}

/**
 *
 */
void loop()
{
    ArduinoOTA.handle();
    if (!ws.enabled())
    {
        Serial.println("No WebSocket!");
        return;
    }

    readSoundLevel();
    // serialPrint();
    // serialPrintForPlot();
    oledPrint();
    wsPrint();

    delay(wait_ms);
}
