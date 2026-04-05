#!/bin/bash

# Uploading the compiled project to the ATmega2560 microcontroller

echo "Finding to which port the ATmega2560 is connected."
echo ""

# Get all ACM ports
ports=$(ls /dev/ttyACM* 2>/dev/null)

if [ -z "$ports" ]; then
    echo "No Arduino devices found on /dev/ttyACM*!"
    exit 1
fi

# Loop through each port and check signature
for port in $ports; do
    echo "Checking $port."
    signature=$(sudo avrdude -c wiring -p atmega2560 -P $port -U signature:r:-:h 2>&1 | grep "device signature" | awk '{print $5}')
    if [ "$signature" = "0x1e9801" ]; then
        echo "Found ATmega2560 on $port!"
        echo ""
        break
    else
        echo "$port is not ATmega2560 (signature: $signature)."
        echo ""
    fi
done

if [ "$signature" != "0x1e9801" ]; then
    echo "ATmega2560 not found on any port!"
    exit 1
fi

echo "Uploading the project to the ATmega2560 on $port."

sudo avrdude -v -c wiring -p atmega2560 -P $port -b 115200 -U flash:w:./project-2560.hex
if [ $? -ne 0 ]; then
    echo "Upload failed!"
    exit 1
fi

echo "Upload completed."
