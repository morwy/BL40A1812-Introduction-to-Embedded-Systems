#!/bin/bash

echo "Building the project..."

avr-gcc -mmcu=atmega2560 -Wall -Os -o ./project-2560.elf ./*.c

echo "Build completed. Output file: project-2560.elf"

echo "Converting the ELF file to HEX format..."

avr-objcopy -j .text -j .data -O ihex ./project-2560.elf ./project-2560.hex

echo "HEX file generated: project-2560.hex"

echo "Build process completed successfully."
