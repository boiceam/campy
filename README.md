# Campy LED Controller
Campy is an Arduino NeoPixel LED controller designed for Seeeduino XIAO module. The current version controls 20 NeoPixel LEDs on a stake (10 per side). These light-up stakes are intended for campground path illumination and ambiance creation.  
## First Time Setup
Download and install the Arduino IDE for you system. The Arduino IDE can be download from: https://www.arduino.cc/en/software  

Click on `File > Preference`, and fill Additional Boards Manager URLs with the url below: 
https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json  

Click `Tools-> Board... -> Boards Manager`, type keyword "Seeeduino XIAO" into the search field.  
Install the `Seeed SAMD Boards` package.  

Click `Tools-> Manage Libriaries`, type keyword `SleepyDog` into the search field.  
Install the `Adafruit SleepyDog Library` library.

Click `Tools-> Manage Libriaries`, type keyword "`NeoPixel` into the search field.  
Install the `Adafruit NeoPixel` library.
## Session Setup
### Compiling
When fist opening the project in the Arduino IDE check the board setting before compiling.   
Click `Tools-> Board... -> Seeed SAMD (32-bits ARM Cortex-M0+ and ARM Cortex-M4 boards) -> Seeeduino XIAO`  
### Programing
Click `Tools-> Port -> /dev/cu.usbmodemXXXXX (Seeeduino XIAO)`  
*NOTE: This is the MacOS port naming syntax. Exact port name may vary, unplug module and the plug it in and see what new port appears if unsure*
## Low Power Mode
When low power mode is enabled the device sleeps between LED updates to conserve energy. This disables the USB connection and requires manual entry in bootloader mode before programming. When using low power mode, programming mode must be entered using the bootloader pads on the module. For details instructions see: https://wiki.seeedstudio.com/Seeeduino-XIAO/#enter-bootloader-mode

For ease of development it is recommended to only enable low power mode once development is complete or for power testing. 