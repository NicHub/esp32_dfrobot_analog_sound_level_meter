/**
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



class Sound_Meter
{
private:
    void initArrayForMovingAverage();

public:
    float max_reading_val;
    float conv_analog_2_volt;

    uint8_t sound_sensor_pin ;
    float vref ;
    float nb_bits;
    float conv_volt_2_db;
    uint16_t wait_ms ;
    uint16_t integrationTime_ms;



    float analogReadValueMax = 0.0f;
    float analogReadValueMin = 1E6f;
    float analogReadValue = 0.0f;
    float voltageValue = 0.0f;
    float dbValue = 0.0f;
    float analogReadValueAveraged = 0.0f;
    float voltageValueAveraged = 0.0f;
    float dbValueAveraged = 0.0f;
    float dbValueLowPass1 = 0.0f;
    float lastdbValue = 0.0f;
    float lastdbValueLowPass1 = 0.0f;
    float dbValueLowPass2 = 0.0f;

    Sound_Meter();
    void setupSoundMeter(
        uint8_t sound_sensor_pin = 34,
        float vref = 3.3f,
        float nb_bits = 12.0f,
        float conv_volt_2_db = 50.0f,
        uint16_t wait_ms = 100,
        uint16_t integrationTime_ms = 500);
    void readSoundLevel();
    void readSoundLevelRaw();
    void calcMinMaxSoundLevel();
    void calcMovingAverage();
    void calcLowPass1();
    void calcLowPass2();
    void toJSON(char *jsonMsg);
};
