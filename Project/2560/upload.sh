#!/bin/bash

# Uploading the compiled project to the ATmega2560 microcontroller

echo "Finding to which port the ATmega2560 is connected..."

port=$(ls /dev/ttyACM* 2>/dev/null | head -n 1)

echo "Found port: $port"

echo "Uploading the project to the ATmega2560..."

avrdude -v -c arduino -p atmega2560 -P $port -b 115200 -U flash:w:./project-2560.hex

echo "Upload completed."
