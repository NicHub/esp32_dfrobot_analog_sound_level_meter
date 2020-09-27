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

#include <main.h>

/**
 *
 */
void setup()
{
    setupSerial();
    setupOLED();
    sm1.setupSoundMeter(SOUND_SENSOR_PIN);
    setupWebServer();
}

/**
 *
 */
void loop()
{
    // Reduce the reading rate.
    static unsigned long _target_t = 0UL;
    unsigned long _current_t = millis();
    if (_current_t < _target_t)
        return;
    _target_t = _current_t + WAIT_MS;

    // Web.
    ArduinoOTA.handle();

    // Read sound level.
    sm1.readSoundLevel(0);
    static char json_msg[129];
    sm1.toJSON(json_msg);

    // Print to serial.
#define SERIAL_OUT 0
#if SERIAL_OUT == 0
    // Do not print to serial.
#elif SERIAL_OUT == 1
    Serial.println(json_msg);
#elif SERIAL_OUT == 2
    serialPrintForPlotInArduinoPlotter();
#endif

    // Print to OLED.
    // printOLEDNumericalValue();
    printOLEDVuMeter();

    // Send to websocket.
    wsPrint(json_msg);
}
