# firmware footprint experiment

## Arduino Uno

### Snpes Node Lib + LoRaMESH Driver + User code

	Linking .pio/build/uno/firmware.elf
	Checking size .pio/build/uno/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [====      ]  38.6% (used 790 bytes from 2048 bytes)
	Flash: [==        ]  18.9% (used 6090 bytes from 32256 bytes)
	Building .pio/build/uno/firmware.hex

### Empty Project + LoRaMESH Driver

	Linking .pio/build/uno/firmware.elf
	Checking size .pio/build/uno/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [=         ]  12.3% (used 252 bytes from 2048 bytes)
	Flash: [          ]   1.6% (used 520 bytes from 32256 bytes)
	Building .pio/build/uno/firmware.hex

### Empty Project

	Linking .pio/build/uno/firmware.elf
	Checking size .pio/build/uno/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [          ]   0.4% (used 9 bytes from 2048 bytes)
	Flash: [          ]   1.4% (used 444 bytes from 32256 bytes)
	Building .pio/build/uno/firmware.hex

## STM32

### Snpes Node Lib + LoRaMESH Driver + User code

	Linking .pio/build/genericSTM32F103C8/firmware.elf
	Checking size .pio/build/genericSTM32F103C8/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [=         ]   7.5% (used 1540 bytes from 20480 bytes)
	Flash: [===       ]  28.1% (used 18396 bytes from 65536 bytes)
	Building .pio/build/genericSTM32F103C8/firmware.bin

### Empty Project + LoRaMESH Driver

	Linking .pio/build/genericSTM32F103C8/firmware.elf
	Checking size .pio/build/genericSTM32F103C8/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [          ]   5.0% (used 1020 bytes from 20480 bytes)
	Flash: [=         ]  11.6% (used 7604 bytes from 65536 bytes)
	Building .pio/build/genericSTM32F103C8/firmware.bin

### Empty Project

	Linking .pio/build/genericSTM32F103C8/firmware.elf
	Checking size .pio/build/genericSTM32F103C8/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [          ]   3.7% (used 764 bytes from 20480 bytes)
	Flash: [=         ]  10.6% (used 6916 bytes from 65536 bytes)
	Building .pio/build/genericSTM32F103C8/firmware.bin

## ESP32

### Snpes Gateway Lib + LoRaMESH Driver + User code

	Linking .pio/build/esp32doit-devkit-v1/firmware.elf
	Retrieving maximum program size .pio/build/esp32doit-devkit-v1/firmware.elf
	Checking size .pio/build/esp32doit-devkit-v1/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [=         ]   7.1% (used 23288 bytes from 327680 bytes)
	Flash: [==        ]  20.4% (used 268029 bytes from 1310720 bytes)
	Building .pio/build/esp32doit-devkit-v1/firmware.bin

### Empty Project + LoRaMESH Driver

	Linking .pio/build/esp32doit-devkit-v1/firmware.elf
	Retrieving maximum program size .pio/build/esp32doit-devkit-v1/firmware.elf
	Checking size .pio/build/esp32doit-devkit-v1/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [=         ]   6.7% (used 22108 bytes from 327680 bytes)
	Flash: [==        ]  17.6% (used 230345 bytes from 1310720 bytes)
	Building .pio/build/esp32doit-devkit-v1/firmware.bin

### Empty Project

	Linking .pio/build/esp32doit-devkit-v1/firmware.elf
	Retrieving maximum program size .pio/build/esp32doit-devkit-v1/firmware.elf
	Checking size .pio/build/esp32doit-devkit-v1/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [=         ]   6.7% (used 21860 bytes from 327680 bytes)
	Flash: [==        ]  17.5% (used 229209 bytes from 1310720 bytes)
	Building .pio/build/esp32doit-devkit-v1/firmware.bin
