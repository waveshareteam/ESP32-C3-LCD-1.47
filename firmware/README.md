# Factory Firmware

[English](README.md) | [简体中文](README_ZH.md)

This directory stores the tested factory image for the ESP32-C3-LCD-1.47 development board.

| File | Purpose |
| --- | --- |
| [ESP32-C3-LCD-1.47_FactoryOnly_20260514.bin](ESP32-C3-LCD-1.47_FactoryOnly_20260514.bin) | Factory recovery firmware for the ESP32-C3-LCD-1.47 board, dated 2026-05-14. Flash from `0x0`. |

Before flashing:

- Back up any device-specific data that must be preserved.
- Confirm the board model and hardware revision.
- Erase the target device before writing the merged image.
- Follow the flashing procedure in the product documentation.

Flash with esptool:

```bash
python -m pip install esptool
esptool.py --chip esp32c3 --port <PORT> erase_flash
esptool.py --chip esp32c3 --port <PORT> write_flash 0x0 ESP32-C3-LCD-1.47_FactoryOnly_20260514.bin
```

SHA-256: `2FF4EAE0F1E75E16F4DA0D9E71FE09D674D4A6ACAB37FDCD9DEC691F50CD411B`

This file is a single merged BIN for factory recovery.
