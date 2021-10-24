# E S P 3 2    —    D F R O B O T    A N A L O G    S O U N D    L E V E L    M E T E R

<https://www.dfrobot.com/product-1663.html>

<https://wiki.dfrobot.com/Gravity__Analog_Sound_Level_Meter_SKU_SEN0232>

<https://fr.aliexpress.com/item/32846247596.html?spm=a2g0s.9042311.0.0.61346c375GJr6S>

<p align="center">
<img width=600px src="https://raw.githubusercontent.com/NicHub/esp32_dfrobot_analog_sound_level_meter/master/images/dfrobot_analog_sound_level_meter_001.jpg" alt="dfrobot analog sound level meter board" />
</p>
<p align="center">
<img width=600px src="https://raw.githubusercontent.com/NicHub/esp32_dfrobot_analog_sound_level_meter/master/images/dfrobot_analog_sound_level_meter_002.jpg" alt="dfrobot analog sound level meter board" />
</p>
<p align="center">
<img width=600px src="https://raw.githubusercontent.com/NicHub/esp32_dfrobot_analog_sound_level_meter/master/images/esp32_dfrobot_analog_sound_level_meter_plots.jpg" alt="dfrobot analog sound level meter plots" />
</p>

## PINOUT

| wire  |       signal       |
| ----- | :----------------: |
| blue  | Analog output IO34 |
| red   |  VCC (3.3V - 5V)   |
| black |        GND         |

## Upload data directory to SPIFS

```bash
platformio run --target uploadfs
```

## Plotly

- <https://plot.ly/javascript/streaming/>

## Kalman Filter

https://github.com/denyssene/SimpleKalmanFilter/blob/master/src/SimpleKalmanFilter.cpp

## Vu meter

- <https://fr.wikipedia.org/wiki/VU-mètre>
- <https://fr.wikipedia.org/wiki/Peak_Programme_Meter>
