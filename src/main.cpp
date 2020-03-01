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
    sm1.setupSoundMeter(SOUND_SENSOR_PIN);
    setupOLED();
    setupWebServer();
}

/**
 *
 */
void loop()
{
    // Reduce the reading rate.
    static unsigned long targetT = 0UL;
    unsigned long currentT = millis();
    if (currentT < targetT)
        return;
    targetT = currentT + wait_ms;

    // Web.
    ArduinoOTA.handle();

    // Read sound level.
    sm1.readSoundLevel();
    static char jsonMsg[200];
    sm1.toJSON(jsonMsg);

    // Print to serial.
#define serial_out 1
#if serial_out == 0
    // Do not print to serial.
#elif serial_out == 1
    Serial.println(jsonMsg);
#elif serial_out == 2
    serialPrintForPlotInArduinoPlotter();
#endif

    // Print to OLED.
    printOLED();

    // Send to websocket.
    wsPrint(jsonMsg);
}
