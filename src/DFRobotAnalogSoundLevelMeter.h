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
#include "SimpleKalmanFilter.h"

class Sound_Meter
{
private:
    void initArrayForMovingAverage();

public:
    float sound_level_raw_max_possible;
    float conv_analog_2_volt;

    uint8_t sound_sensor_pin;
    float vref;
    float nb_bits;
    float conv_volt_2_db;
    uint16_t wait_ms;
    uint16_t integration_time_ms;

    float sound_level_raw_max = 0.0f;
    float sound_level_raw_min = 1E6f;
    float sound_level_raw = 0.0f;
    float sound_level_volt = 0.0f;
    float sound_level_db = 0.0f;
    float moving_average_raw = 0.0f;
    float moving_average_voltage = 0.0f;
    float moving_average_sound_level = 0.0f;
    float sound_level_db_low_pass_1 = 0.0f;
    float last_sound_level_db = 0.0f;
    float last_sound_level_db_low_pass_1 = 0.0f;
    float sound_level_db_low_pass_2 = 0.0f;
    float sound_level_db_kalman = 0.0f;

    Sound_Meter();
    void setupSoundMeter(
        uint8_t sound_sensor_pin = 34,
        float vref = 3.3f,
        float nb_bits = 12.0f,
        float conv_volt_2_db = 50.0f,
        uint16_t wait_ms = 100,
        uint16_t integration_time_ms = 500);
    void readSoundLevel();
    void readSoundLevelRaw();
    void calcMinMaxSoundLevel();
    void calcMovingAverage();
    void calcLowPass1();
    void calcLowPass2();
    void calcKalman();
    void toJSON(char *json_msg);
};
