#!/bin/bash

echo "Building the project..."

avr-gcc -mmcu=atmega328p -Wall -Os -o ./project-UNO.elf ./*.c

echo "Build completed. Output file: project-UNO.elf"

echo "Converting the ELF file to HEX format..."

avr-objcopy -j .text -j .data -O ihex ./project-UNO.elf ./project-UNO.hex

echo "HEX file generated: project-UNO.hex"

echo "Build process completed successfully."
