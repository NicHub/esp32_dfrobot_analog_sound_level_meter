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
#include <dfrobot_analog_sound_level_meter.h>

const uint16_t movingAverageSize = 10;
float analogReadValues[movingAverageSize];
uint8_t cnt = 0;

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
    for (size_t _i = 0; _i < movingAverageSize; _i++)
    {
        analogReadValues[_i] = analogReadValue;
    }
}

/**
 *
 */
void Sound_Meter::readSoundLevelRaw()
{
    // Read sound level.
    Sound_Meter::analogReadValue = analogRead(Sound_Meter::sound_sensor_pin);
    Sound_Meter::voltageValue = Sound_Meter::analogReadValue * Sound_Meter::conv_analog_2_volt;
    Sound_Meter::dbValue = Sound_Meter::voltageValue * Sound_Meter::conv_volt_2_db;
}

/**
 *
 */
void Sound_Meter::calcMinMaxSoundLevel()
{
    // Get min and max analogReadValue.
    if (Sound_Meter::analogReadValueMax < Sound_Meter::analogReadValue)
        Sound_Meter::analogReadValueMax = Sound_Meter::analogReadValue;
    if (Sound_Meter::analogReadValueMin > Sound_Meter::analogReadValue)
        Sound_Meter::analogReadValueMin = Sound_Meter::analogReadValue;
}

/**
 *
 */
void Sound_Meter::calcMovingAverage()
{
    // Reset moving average in case of big difference.
    if (abs(Sound_Meter::lastdbValue - Sound_Meter::dbValue) > 10)
        initArrayForMovingAverage();

    // Calculate moving average;
    analogReadValues[cnt] = Sound_Meter::analogReadValue;
    float sum = 0;
    for (size_t _i = 0; _i < movingAverageSize; _i++)
    {
        sum += analogReadValues[_i];
    }
    Sound_Meter::analogReadValueAveraged = sum / movingAverageSize;
    Sound_Meter::voltageValueAveraged = Sound_Meter::analogReadValueAveraged * Sound_Meter::conv_analog_2_volt;
    Sound_Meter::dbValueAveraged = Sound_Meter::voltageValueAveraged * Sound_Meter::conv_volt_2_db;
    cnt++;
    if (cnt == movingAverageSize)
        cnt = 0;
}

/**
 *
 */
void Sound_Meter::calcLowPass1()
{
    // Low pass.
    if (abs(Sound_Meter::lastdbValue - Sound_Meter::dbValue) < 2)
    {
        Sound_Meter::dbValueLowPass1 = Sound_Meter::lastdbValueLowPass1;
    }
    else
    {
        Sound_Meter::lastdbValueLowPass1 = Sound_Meter::dbValue;
        Sound_Meter::dbValueLowPass1 = Sound_Meter::dbValue;
    }
}

/**
 *
 */
void Sound_Meter::calcLowPass2()
{
    // Low pass 2.
    const float LPF_Beta = 0.1;
    dbValueLowPass2 = dbValueLowPass2 - (LPF_Beta * (dbValueLowPass2 - dbValue));

    // Store last dbValue.
    lastdbValue = dbValue;
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
    uint16_t integrationTime_ms)
{
    // Update class variables.
    Sound_Meter::sound_sensor_pin = sound_sensor_pin;
    Sound_Meter::vref = vref;
    Sound_Meter::nb_bits = nb_bits;
    Sound_Meter::conv_volt_2_db = conv_volt_2_db;
    Sound_Meter::wait_ms = wait_ms;
    Sound_Meter::integrationTime_ms = integrationTime_ms;

    // Calculate
    // max_reading_val = 2 ^ nb_bits - 1.
    Sound_Meter::max_reading_val = 1;
    for (size_t _i = 0; _i < nb_bits; _i++)
        Sound_Meter::max_reading_val *= 2;
    Sound_Meter::max_reading_val -= 1;

    // conv_analog_2_volt
    Sound_Meter::conv_analog_2_volt = Sound_Meter::vref / Sound_Meter::max_reading_val;

    // analogReadValueMin
    Sound_Meter::analogReadValueMin = Sound_Meter::max_reading_val;

    // TODO
    // Calculate movingAverageSize
    // movingAverageSize = integrationTime_ms / wait_ms;

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
        R"rawText({"dbValue":%f,"dbValueAveraged":%f,"dbValueLowPass1":%f,"dbValueLowPass2":%f})rawText";

    sprintf(jsonMsg,
            formatString,
            dbValue,
            dbValueAveraged,
            dbValueLowPass1,
            dbValueLowPass2);
}
