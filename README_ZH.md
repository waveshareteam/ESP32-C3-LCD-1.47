<div align="center">
  <h1>ESP32-C3-LCD-1.47</h1>
  <p><strong>ESP32-C3 1.47 英寸 172×320 IPS 液晶显示开发板。</strong></p>
  <p><a href="README.md">English</a> | <strong>简体中文</strong></p>
  <p>
    <a href="https://github.com/waveshareteam/ESP32-C3-LCD-1.47/actions/workflows/build.yml"><img alt="构建" src="https://github.com/waveshareteam/ESP32-C3-LCD-1.47/actions/workflows/build.yml/badge.svg"></a>
    <a href="https://github.com/waveshareteam/ESP32-C3-LCD-1.47/releases/latest"><img alt="最新版本" src="https://img.shields.io/github/v/release/waveshareteam/ESP32-C3-LCD-1.47"></a>
    <a href="LICENSE"><img alt="许可证" src="https://img.shields.io/github/license/waveshareteam/ESP32-C3-LCD-1.47"></a>
  </p>
  <p>
    <a href="https://www.waveshare.com/ESP32-C3-LCD-1.47.htm">🛒 产品</a> &middot;
    <a href="firmware/">📦 固件</a> &middot;
    <a href="examples/esp-idf/">🧩 ESP-IDF</a> &middot;
    <a href="examples/arduino/examples/">🔧 Arduino</a> &middot;
    <a href="https://docs.waveshare.net/ESP32-C3-LCD-1.47/">📖 文档</a>
  </p>
  <p><img src="docs/images/ESP32-C3-LCD-1.47.webp" alt="ESP32-C3-LCD-1.47 开发板" width="500"></p>
</div>

---

## ✨ 概述

本仓库提供 ESP32-C3-LCD-1.47 开发板的 ESP-IDF 和 Arduino 示例。开发板集成 1.47 英寸 172×320 IPS 液晶屏、Wi-Fi、蓝牙、USB Type-C、充电电路、SD 接口和 GPIO 扩展。

| 功能 | 器件或接口 |
| --- | --- |
| MCU | ESP32-C3 |
| 存储器 | 4 MB Flash，无 PSRAM |
| 显示 | 1.47 英寸 172×320 IPS 液晶屏，SPI，ST7789 |
| 存储 | MicroSD 接口 |

## 📦 快速开始

最新版本提供本仓库示例的可烧录固件包。

1. 下载所需示例和框架版本对应的 `*-combined.zip`。
2. 解压后通过 `python -m pip install esptool` 安装 esptool。
3. 通过 USB 连接开发板。
4. 运行压缩包中适用于操作系统的烧录脚本。
5. 如果开发板未自动重启，请手动复位。

出厂固件用于恢复开箱演示。在 [`firmware`](firmware/) 目录下载文件名带 `FactoryOnly` 的文件，并从 `0x0` 地址烧录。

## 🧪 示例

### ESP-IDF

| 示例 | 主要功能 |
| --- | --- |
| [ESP32-C3-LCD-1.47](examples/esp-idf/ESP32-C3-LCD-1.47/) | 液晶屏、LVGL、SD、亮度和无线示例 |

### Arduino

| 示例 | 主要功能 |
| --- | --- |
| [01_gfx_helloworld](examples/arduino/examples/01_gfx_helloworld/) | 液晶屏图形示例 |
| [02_SD](examples/arduino/examples/02_SD/) | SD 卡访问 |
| [03_Brightness](examples/arduino/examples/03_Brightness/) | 液晶屏亮度控制 |
| [04_LVGL](examples/arduino/examples/04_LVGL/) | LVGL 用户界面 |

草图使用的库随仓库提供，位于 [`examples/arduino/libraries`](examples/arduino/libraries/)。

## 🛠️ 开发环境

| 开发框架 | 版本 | 示例数 |
| --- | --- | ---: |
| ESP-IDF | v5.3.2 | 1 |
| Arduino-ESP32 | 3.1.1 | 4 |

## 📚 资料

- [原理图](hardware/schematic/)
- [机械资料（尺寸图和 3D 模型）](hardware/mechanical/)
- [产品页](https://www.waveshare.com/ESP32-C3-LCD-1.47.htm)
- [Wiki](https://docs.waveshare.net/ESP32-C3-LCD-1.47/)

## 🤝 支持

- 示例或仓库问题：[提交 issue](https://gitee.com/waveshare/ESP32-C3-LCD-1.47/issues)。
- 产品使用、硬件或订单问题：[微雪技术支持](https://docs.waveshare.net/ESP32-C3-LCD-1.47/Technical-Support/)。

## 📄 许可证

本仓库使用 Apache License 2.0，详见 [LICENSE](LICENSE)。

