#!/bin/bash

# Uploading the compiled project to the Arduino UNO microcontroller

echo "Finding to which port the Arduino UNO is connected."
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
    signature=$(sudo avrdude -c arduino -p atmega328p -P $port -U signature:r:-:h 2>&1 | grep "device signature" | awk '{print $5}')
    if [ "$signature" = "0x1e950f" ]; then
        echo "Found Arduino UNO on $port!"
        echo ""
        break
    else
        echo "$port is not Arduino UNO (signature: $signature)."
        echo ""
    fi
done

if [ "$signature" != "0x1e950f" ]; then
    echo "Arduino UNO not found on any port!"
    exit 1
fi

echo "Uploading the project to the Arduino UNO on $port."

sudo avrdude -v -c arduino -p atmega328p -P $port -b 115200 -U flash:w:./project-UNO.hex
if [ $? -ne 0 ]; then
    echo "Upload failed!"
    exit 1
fi

echo "Upload completed."
