CFLAGS		+= -I./sources/driver/led
CFLAGS		+= -I./sources/driver/rtc
CFLAGS		+= -I./sources/driver/button
CFLAGS		+= -I./sources/driver/ir
CFLAGS		+= -I./sources/driver/flash
CFLAGS		+= -I./sources/driver/fuzzy_logic
CFLAGS		+= -I./sources/driver/hs1101
CFLAGS		+= -I./sources/driver/kalman

CPPFLAGS	+= -I./sources/driver/SHT1x
CPPFLAGS	+= -I./sources/driver/thermistor
CPPFLAGS	+= -I./sources/driver/Adafruit_ssd1306syp
CPPFLAGS	+= -I./sources/driver/EmonLib
CPPFLAGS	+= -I./sources/driver/ds1302
CPPFLAGS	+= -I./sources/driver/epprom
CPPFLAGS	+= -I./sources/driver/exor
CPPFLAGS	+= -I./sources/driver/LTC68032
CPPFLAGS	+= -I./sources/driver/I2Cdev
CPPFLAGS	+= -I./sources/driver/MPU6050
CPPFLAGS	+= -I./sources/driver/L3G4200D

VPATH += sources/driver/led
VPATH += sources/driver/button
VPATH += sources/driver/rtc
VPATH += sources/driver/ir
VPATH += sources/driver/SHT1x
VPATH += sources/driver/thermistor
VPATH += sources/driver/Adafruit_ssd1306syp
VPATH += sources/driver/EmonLib
VPATH += sources/driver/ds1302
VPATH += sources/driver/eeprom
VPATH += sources/driver/flash
VPATH += sources/driver/fuzzy_logic
VPATH += sources/driver/kalman
VPATH += sources/driver/hs1101
VPATH += sources/driver/exor
VPATH += sources/driver/LTC68032
VPATH += sources/driver/I2Cdev
VPATH += sources/driver/MPU6050
VPATH += sources/driver/L3G4200D

SOURCES += sources/driver/led/led.c
SOURCES += sources/driver/rtc/rtc.c
SOURCES += sources/driver/button/button.c
SOURCES += sources/driver/ir/ir.c
SOURCES += sources/driver/flash/flash.c
SOURCES += sources/driver/fuzzy_logic/fuzzy_logic.c
SOURCES += sources/driver/hs1101/hs1101.c
SOURCES += sources/driver/kalman/kalman.c

SOURCES_CPP += sources/driver/SHT1x/SHT1x.cpp
SOURCES_CPP += sources/driver/thermistor/thermistor.cpp
SOURCES_CPP += sources/driver/Adafruit_ssd1306syp/Adafruit_GFX.cpp
SOURCES_CPP += sources/driver/Adafruit_ssd1306syp/Adafruit_ssd1306syp.cpp
SOURCES_CPP += sources/driver/Adafruit_ssd1306syp/glcdfont.cpp
SOURCES_CPP += sources/driver/EmonLib/EmonLib.cpp
SOURCES_CPP += sources/driver/ds1302/DS1302.cpp
SOURCES_CPP += sources/driver/eeprom/eeprom.cpp
SOURCES_CPP += sources/driver/exor/exor.cpp
SOURCES_CPP += sources/driver/LTC68032/LTC68032.cpp
SOURCES_CPP += sources/driver/I2Cdev/I2Cdev.cpp
SOURCES_CPP += sources/driver/MPU6050/MPU6050.cpp
SOURCES_CPP += sources/driver/L3G4200D/L3G4200D.cpp
