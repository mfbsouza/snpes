# firmware size experiment

## Arduino Uno: Snpes Node Lib + User code

	Linking .pio/build/uno/firmware.elf
	Checking size .pio/build/uno/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [====      ]  38.6% (used 790 bytes from 2048 bytes)
	Flash: [==        ]  18.9% (used 6090 bytes from 32256 bytes)
	Building .pio/build/uno/firmware.hex

### Empty Project

	Linking .pio/build/uno/firmware.elf
	Checking size .pio/build/uno/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [          ]   0.4% (used 9 bytes from 2048 bytes)
	Flash: [          ]   1.4% (used 444 bytes from 32256 bytes)
	Building .pio/build/uno/firmware.hex

### Average size:

* RAM: 790 - 9 = 781 bytes, 38.2%
* Flash: 6090 - 444 = 5646 bytes, 17.5%

## STM32: Snpes Node Lib + User code

	Linking .pio/build/genericSTM32F103C8/firmware.elf
	Checking size .pio/build/genericSTM32F103C8/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [=         ]   7.5% (used 1540 bytes from 20480 bytes)
	Flash: [===       ]  28.1% (used 18396 bytes from 65536 bytes)
	Building .pio/build/genericSTM32F103C8/firmware.bin

### Empty Project

	Linking .pio/build/genericSTM32F103C8/firmware.elf
	Checking size .pio/build/genericSTM32F103C8/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [          ]   3.7% (used 764 bytes from 20480 bytes)
	Flash: [=         ]  10.6% (used 6916 bytes from 65536 bytes)
	Building .pio/build/genericSTM32F103C8/firmware.bin

### Average size:

* RAM: 1540 - 764 = 776 bytes, 3.8%
* Flash: 18396 - 6916 = 11480 bytes, 17.5%

## ESP32: Snpes Gateway Lib + User code

	Linking .pio/build/esp32doit-devkit-v1/firmware.elf
	Retrieving maximum program size .pio/build/esp32doit-devkit-v1/firmware.elf
	Checking size .pio/build/esp32doit-devkit-v1/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [=         ]   5.4% (used 17576 bytes from 327680 bytes)
	Flash: [==        ]  19.1% (used 250721 bytes from 1310720 bytes)
	Building .pio/build/esp32doit-devkit-v1/firmware.bin

### Empty Project

	Linking .pio/build/esp32doit-devkit-v1/firmware.elf
	Retrieving maximum program size .pio/build/esp32doit-devkit-v1/firmware.elf
	Checking size .pio/build/esp32doit-devkit-v1/firmware.elf
	Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
	RAM:   [          ]   4.9% (used 16144 bytes from 327680 bytes)
	Flash: [==        ]  16.2% (used 212961 bytes from 1310720 bytes)
	Building .pio/build/esp32doit-devkit-v1/firmware.bin


### Average size:

* RAM: 17576 - 16144 = 1432 bytes, 0.5%
* Flash: 250721 - 212961 = 37760 bytes, 2.9%

