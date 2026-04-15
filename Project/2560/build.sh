#!/bin/bash

CURRENT_DIR=$(pwd)

echo "Current directory: $CURRENT_DIR"

echo "Building the project."

avr-gcc -mmcu=atmega2560 -Wall -Os -o $CURRENT_DIR/project-2560.elf $CURRENT_DIR/*.c

echo "Build completed. Output file: project-2560.elf"

echo "Converting the ELF file to HEX format."

avr-objcopy -j .text -j .data -O ihex $CURRENT_DIR/project-2560.elf $CURRENT_DIR/project-2560.hex

echo "HEX file generated: project-2560.hex"

echo "Build process completed successfully."
