Quick Start

1.. install stlink

https://github.com/texane/stlink.git


2.. install gcc-arm-none-eabi

https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads


3.. install openocd (if you want to debug using openocd)

https://github.com/gnu-mcu-eclipse/openocd.git


4.. install ddd for dbg UI debug (if you want to debug using openocd and ddd)

sudo apt-get install ddd


5.. install ak_flash (flash firmware using bootloader via UART)

https://github.com/thannt92/ak_flash.git

In other to using ak_flash you MUST be burn bootloader first.

https://github.com/thannt92/arm_cortex_m3_base_source/tree/master/boot

build and burn firmware command:

cd boot

make

make flash

NOTE: make flash using stflash (st-link debugger)


6.. redirect tools path.

https://github.com/thannt92/arm_cortex_m3_base_source/blob/master/application/Makefile


GCC_PATH	= /home/thannt/workspace/tools/gcc-arm-none-eabi-5_4-2016q3

OPENOCD_PATH	= /home/thannt/workspace/tools/openocd


change 2 paths corresponding to your isntalling.

build application:

cd appplication

make help

NOTE: make help suppy more command build information. 
