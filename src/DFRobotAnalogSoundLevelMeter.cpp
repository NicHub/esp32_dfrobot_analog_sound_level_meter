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

#include <Arduino.h>
#include <DFRobotAnalogSoundLevelMeter.h>

const uint16_t MOVING_AVERAGE_SIZE = 10;
float SOUND_LEVEL_RAW[MOVING_AVERAGE_SIZE];
uint8_t CNT = 0;
SimpleKalmanFilter SIMPLE_KALMAN_FILTER(2, 2, 0.1);

/**
 *
 */
Sound_Meter::Sound_Meter()
{
}

/**
 *
 */
void Sound_Meter::initArrayForMovingAverage()
{
    for (size_t _i = 0; _i < MOVING_AVERAGE_SIZE; _i++)
    {
        SOUND_LEVEL_RAW[_i] = sound_level_raw;
    }
}

/**
 *
 */
void Sound_Meter::readSoundLevelRaw()
{
    // Read sound level.
    Sound_Meter::sound_level_raw = analogRead(Sound_Meter::sound_sensor_pin);
    Sound_Meter::sound_level_volt = Sound_Meter::sound_level_raw * Sound_Meter::conv_analog_2_volt;
    Sound_Meter::sound_level_db = Sound_Meter::sound_level_volt * Sound_Meter::conv_volt_2_db;
}

/**
 *
 */
void Sound_Meter::calcMinMaxSoundLevel()
{
    // Get min and max sound_level_raw.
    if (Sound_Meter::sound_level_raw_max < Sound_Meter::sound_level_raw)
        Sound_Meter::sound_level_raw_max = Sound_Meter::sound_level_raw;
    if (Sound_Meter::sound_level_raw_min > Sound_Meter::sound_level_raw)
        Sound_Meter::sound_level_raw_min = Sound_Meter::sound_level_raw;
}

/**
 *
 */
void Sound_Meter::calcMovingAverage()
{
    // Reset moving average in case of big difference.
    if (abs(Sound_Meter::last_sound_level_db - Sound_Meter::sound_level_db) > 10)
        initArrayForMovingAverage();

    // Calculate moving average;
    SOUND_LEVEL_RAW[CNT] = Sound_Meter::sound_level_raw;
    float sum = 0;
    for (size_t _i = 0; _i < MOVING_AVERAGE_SIZE; _i++)
    {
        sum += SOUND_LEVEL_RAW[_i];
    }
    Sound_Meter::moving_average_raw = sum / MOVING_AVERAGE_SIZE;
    Sound_Meter::moving_average_voltage = Sound_Meter::moving_average_raw * Sound_Meter::conv_analog_2_volt;
    Sound_Meter::moving_average_sound_level = Sound_Meter::moving_average_voltage * Sound_Meter::conv_volt_2_db;
    CNT++;
    if (CNT == MOVING_AVERAGE_SIZE)
        CNT = 0;
}

/**
 *
 */
void Sound_Meter::calcLowPass1()
{
    // Low pass.
    if (abs(Sound_Meter::last_sound_level_db - Sound_Meter::sound_level_db) < 2)
    {
        Sound_Meter::sound_level_db_low_pass_1 = Sound_Meter::last_sound_level_db_low_pass_1;
    }
    else
    {
        Sound_Meter::last_sound_level_db_low_pass_1 = Sound_Meter::sound_level_db;
        Sound_Meter::sound_level_db_low_pass_1 = Sound_Meter::sound_level_db;
    }
}

/**
 *
 */
void Sound_Meter::calcLowPass2()
{
    // Low pass 2.
    const float LPF_BETA = 0.1;
    Sound_Meter::sound_level_db_low_pass_2 = Sound_Meter::sound_level_db_low_pass_2 - (LPF_BETA * (Sound_Meter::sound_level_db_low_pass_2 - Sound_Meter::sound_level_db));

    // Store last sound_level_db.
    Sound_Meter::last_sound_level_db = Sound_Meter::sound_level_db;
}

/**
 *
 */
void Sound_Meter::calcKalman()
{
    Sound_Meter::sound_level_db_kalman = SIMPLE_KALMAN_FILTER.updateEstimate(Sound_Meter::sound_level_db);
}

/**
 *
 */
void Sound_Meter::readSoundLevel()
{
    Sound_Meter::readSoundLevelRaw();
    Sound_Meter::calcMinMaxSoundLevel();
    Sound_Meter::calcMovingAverage();
    Sound_Meter::calcLowPass1();
    Sound_Meter::calcLowPass2();
    Sound_Meter::calcKalman();
}

/**
 *
 */
void Sound_Meter::setupSoundMeter(
    uint8_t sound_sensor_pin,
    float vref,
    float nb_bits,
    float conv_volt_2_db,
    uint16_t wait_ms,
    uint16_t integration_time_ms)
{
    // Update class variables.
    Sound_Meter::sound_sensor_pin = sound_sensor_pin;
    Sound_Meter::vref = vref;
    Sound_Meter::nb_bits = nb_bits;
    Sound_Meter::conv_volt_2_db = conv_volt_2_db;
    Sound_Meter::wait_ms = wait_ms;
    Sound_Meter::integration_time_ms = integration_time_ms;

    // Calculate
    // sound_level_raw_max_possible = 2 ^ nb_bits - 1.
    Sound_Meter::sound_level_raw_max_possible = 1;
    for (size_t _i = 0; _i < nb_bits; _i++)
        Sound_Meter::sound_level_raw_max_possible *= 2;
    Sound_Meter::sound_level_raw_max_possible -= 1;

    // conv_analog_2_volt
    Sound_Meter::conv_analog_2_volt = Sound_Meter::vref / Sound_Meter::sound_level_raw_max_possible;

    // sound_level_raw_min
    Sound_Meter::sound_level_raw_min = Sound_Meter::sound_level_raw_max_possible;

    // TODO
    // Calculate MOVING_AVERAGE_SIZE. Currently it is fixed.
    // MOVING_AVERAGE_SIZE = integration_time_ms / wait_ms;

    // Analog setup.
    analogReadResolution(nb_bits);                       // Resolution of analogRead return values. Range: 9 - 12. Default: 12 bits (0-4095).
    analogSetWidth(nb_bits);                             // Sets the sample bits and read resolution. Range: 9 - 12. Default: 12 bits (0-4095).
    analogSetCycles(8);                                  // Set number of cycles per sample. Range: 1 - 255. Default: 8.
    analogSetSamples(1);                                 // Set number of samples in the range. Range: 1 - 255. Default: 1.
    analogSetClockDiv(1);                                // Set the divider for the ADC clock. Range: 1 - 255. Default. 1.
    analogSetAttenuation(ADC_11db);                      // Set the attenuation for all channels. Can be ADC_0db, ADC_2_5db, ADC_6db, ADC_11db. Default: ADC_11db.
    analogSetPinAttenuation(sound_sensor_pin, ADC_11db); // Set the attenuation for particular pin.

    // Initial reading to fill the array for the moving average.
    Sound_Meter::readSoundLevel();
    Sound_Meter::initArrayForMovingAverage();
}

/**
 *
 */
void Sound_Meter::toJSON(char *jsonMsg)
{
    const char *formatString =
        R"rawText({"sound_level_dB":{"raw":%.1f,"moving_average":%.1f,"low_pass_1":%.1f,"low_pass_2":%.1f,"kalman":%.1f}})rawText";

    sprintf(jsonMsg,
            formatString,
            Sound_Meter::sound_level_db,
            Sound_Meter::moving_average_sound_level,
            Sound_Meter::sound_level_db_low_pass_1,
            Sound_Meter::sound_level_db_low_pass_2,
            Sound_Meter::sound_level_db_kalman);
}
