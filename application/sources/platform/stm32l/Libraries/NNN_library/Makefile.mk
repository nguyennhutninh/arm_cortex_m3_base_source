
CFLAGS += -I./sources/platform/stm32l/Libraries/NNN_library/spi
CFLAGS += -I./sources/platform/stm32l/Libraries/NNN_library/gpio


CPPFLAGS += -I./sources/platform/stm32l/Libraries/NNN_library/spi
CPPFLAGS += -I./sources/platform/stm32l/Libraries/NNN_library/gpio

VPATH += sources/platform/stm32l/Libraries/NNN_library/spi
VPATH += sources/platform/stm32l/Libraries/NNN_library/gpio

# C source files
SOURCES += sources/platform/stm32l/Libraries/NNN_library/spi/tm_stm32f4_spi.c
SOURCES += sources/platform/stm32l/Libraries/NNN_library/gpio/tm_stm32f4_gpio.c
