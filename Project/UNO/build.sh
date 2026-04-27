#!/bin/bash

CURRENT_DIR=$(pwd)

echo "Current directory: $CURRENT_DIR"

echo "Building the project."

avr-gcc -mmcu=atmega328p -Wall -Os -o $CURRENT_DIR/project-UNO.elf $CURRENT_DIR/current/GccApplication1/*.c

echo "Build completed. Output file: project-UNO.elf"

echo "Converting the ELF file to HEX format."

avr-objcopy -j .text -j .data -O ihex $CURRENT_DIR/project-UNO.elf $CURRENT_DIR/project-UNO.hex

echo "HEX file generated: project-UNO.hex"

echo "Build process completed successfully."
