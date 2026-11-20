# 出厂固件

[English](README.md) | [简体中文](README_ZH.md)

本目录保存 ESP32-C3-LCD-1.47 开发板的实测出厂镜像。

| 文件 | 用途 |
| --- | --- |
| [ESP32-C3-LCD-1.47_FactoryOnly_20260514.bin](ESP32-C3-LCD-1.47_FactoryOnly_20260514.bin) | ESP32-C3-LCD-1.47 出厂恢复固件，日期为 2026-05-14，从 `0x0` 地址开始烧录。 |

烧录前：

- 备份需要保留的设备专用数据。
- 确认开发板型号和硬件版本。
- 烧录合并镜像前先整片擦除目标设备。
- 按产品文档中的烧录流程操作。

用 esptool 烧录：

```bash
python -m pip install esptool
esptool.py --chip esp32c3 --port <PORT> erase_flash
esptool.py --chip esp32c3 --port <PORT> write_flash 0x0 ESP32-C3-LCD-1.47_FactoryOnly_20260514.bin
```

SHA-256：`2FF4EAE0F1E75E16F4DA0D9E71FE09D674D4A6ACAB37FDCD9DEC691F50CD411B`

该文件是用于恢复出厂功能的单个合并 BIN。
