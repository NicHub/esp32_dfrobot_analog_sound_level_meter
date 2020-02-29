/**
 * E S P 3 2    —    D F R O B O T    A N A L O G    S O U N D    L E V E L    M E T E R
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

// ~/.platformio/packages/framework-arduinoespressif32/cores/esp32/esp32-hal-adc.h
#define SOUND_SENSOR_PIN 34                         // This pin read the analog voltage from the sound level meter.
#define VREF 3.3f                                   // Voltage on AREF pin,default:operating voltage.
#define NB_BITS 12                                  // Analog resolution.
#define MAX_READING_VAL 4095.0f                     // = 2 ^ NB_BITS
#define CONV_ANALOG_2_VOLT (VREF / MAX_READING_VAL) // Conversion between analog read and voltage.
#define CONV_VOLT_2_DB 50.0f                        // Conversion between voltage and decibel.

void setup()
{
    Serial.begin(115200);
    Serial.flush();
    Serial.print("CONV_ANALOG_2_VOLT: ");
    Serial.println(CONV_ANALOG_2_VOLT, 48);

    analogReadResolution(NB_BITS);                       // Resolution of analogRead return values. Range: 9 - 12. Default: 12 bits (0-4095).
    analogSetWidth(NB_BITS);                             // Sets the sample bits and read resolution. Range: 9 - 12. Default: 12 bits (0-4095).
    analogSetCycles(8);                                  // Set number of cycles per sample. Range: 1 - 255. Default: 8.
    analogSetSamples(1);                                 // Set number of samples in the range. Range: 1 - 255. Default: 1.
    analogSetClockDiv(1);                                // Set the divider for the ADC clock. Range: 1 - 255. Default. 1.
    analogSetAttenuation(ADC_11db);                      // Set the attenuation for all channels. Can be ADC_0db, ADC_2_5db, ADC_6db, ADC_11db. Default: ADC_11db.
    analogSetPinAttenuation(SOUND_SENSOR_PIN, ADC_11db); // Set the attenuation for particular pin.
}

float analogReadValueMax = 0.0f;
float analogReadValueMin = MAX_READING_VAL;

void loop()
{
    // Read sound level.
    float analogReadValue = analogRead(SOUND_SENSOR_PIN);
    float voltageValue = analogReadValue * CONV_ANALOG_2_VOLT;
    float dbValue = voltageValue * CONV_VOLT_2_DB;

    // Get min and max analogReadValue.
    if (analogReadValueMax < analogReadValue)
        analogReadValueMax = analogReadValue;
    if (analogReadValueMin > analogReadValue)
        analogReadValueMin = analogReadValue;

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
    Serial.println();

    // Wait.
    delay(125);
}
