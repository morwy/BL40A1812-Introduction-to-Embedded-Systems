# Compiling on Linux

1. Install VSCode and the C++ extension.
2. Install following packages for building for Arduino Uno and 2560:

    ```bash
    sudo apt-get update && sudo apt-get install gcc-avr avr-libc avrdude
    ```

3. Use the corresponding `build.sh` and `upload.sh` scripts to compile and upload your code.
