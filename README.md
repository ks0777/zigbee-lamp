| Supported Targets | ESP32-C6 |
| ----------------- | -------- |

# Zigbee Light 

This is a firmware for a zigbee endpoint I wrote for my ceiling light. It runs on an esp32-c6 and provides a color control cluster for an RGB led strip and another color control cluster for two white led strips with differing color temperatures.

## Hardware Required

* One development board with ESP32-C6 SoC acting as Zigbee end-device
* A USB cable for power supply and programming

## Configure the project

Before project configuration and build, make sure to set the correct chip target using `idf.py --preview set-target esp32c6` command.

## Erase the NVRAM 

Before flash it to the board, it is recommended to erase NVRAM if user doesn't want to keep the previous examples or other projects stored info 
using `idf.py -p PORT erase-flash`

## Build and Flash

Build the project, flash it to the board, and start the monitor tool to view the serial output by running `idf.py -p PORT flash monitor`.

(To exit the serial monitor, type ``Ctrl-]``.)

