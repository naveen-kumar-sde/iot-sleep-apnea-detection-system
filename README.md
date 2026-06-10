# IoT Sleep Apnea Detection System

<div align="center">

![Platform](https://img.shields.io/badge/Platform-ESP32%20%7C%20Arduino%20Nano-blue?style=for-the-badge&logo=arduino)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20%2F%20Arduino-orange?style=for-the-badge&logo=cplusplus)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Completed-brightgreen?style=for-the-badge)

**An intelligent, real-time IoT-based sleep apnea monitoring system with CPAP automation, fall detection, and a live web dashboard.**

*Mini Project — B.E. Electronics and Communication Engineering*  
*V.S.B. Engineering College, Karur — April 2026*

</div>

---

## 📑 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [System Architecture](#-system-architecture)
- [Hardware Components](#-hardware-components)
- [Circuit Connections](#-circuit-connections)
- [Software & Libraries](#-software--libraries)
- [Project Structure](#-project-structure)
- [How It Works](#-how-it-works)
- [Web Dashboard](#-web-dashboard)
- [Setup & Installation](#-setup--installation)
- [Team](#-team)

---

## 📌 Overview

Sleep apnea is a potentially serious sleep disorder in which breathing repeatedly stops and starts during sleep. This project presents an **intelligent IoT-based detection and response system** that:

- Continuously monitors breathing sounds, blood oxygen (SpO₂), heart rate, temperature, and humidity.
- Automatically triggers a **CPAP (Continuous Positive Airway Pressure)** device via a relay when an apnea event is detected.
- Detects patient **fall events** using an accelerometer.
- Provides a **real-time web dashboard** accessible from any device on the same Wi-Fi network.
- Displays live readings on a **16×2 LCD**.

---

## ✨ Features

| Feature | Description |
|---|---|
| 🫁 **Apnea Detection** | Detects breathing cessation using an analog sound sensor |
| 💓 **Heart Rate & SpO₂** | MAX30100 pulse oximeter for non-invasive monitoring |
| 🌡️ **Ambient Monitoring** | DHT11 for temperature and humidity readings |
| 🛡️ **Fall Detection** | MPU6050 accelerometer on Arduino Nano sub-node |
| ⚙️ **CPAP Automation** | Relay automatically activates CPAP airflow on apnea |
| 🔔 **Buzzer Alert** | Immediate audible alarm on apnea/fall events |
| 📟 **LCD Display** | 16×2 display shows live vitals at bedside |
| 🌐 **Web Dashboard** | Mobile-friendly real-time dashboard via Wi-Fi AP |
| 📡 **No Internet Needed** | ESP32 runs as its own Access Point |

---

## 🏗️ System Architecture

```
                     ┌─────────────────────────────────────────┐
                     │              ESP32 (Main Hub)           │
                     │                                         │
  DHT11 ────────────►│  Temp / Humidity                        │
  Sound Sensor ─────►│  Breathing Sound (ADC)                  │
  MAX30100 ─────────►│  Heart Rate + SpO₂ (I²C)               │
  Nano (UART) ──────►│  Fall Status                            │
                     │                                         │
                     │  ┌──────────┐  ┌───────────┐           │
                     │  │  LCD     │  │ Web Server │           │
                     │  │ (16×2)   │  │ (Port 80)  │           │
                     │  └──────────┘  └─────┬─────┘           │
                     │                      │                  │
                     │  RELAY ──► CPAP      │                  │
                     │  BUZZER ──► Alert    │                  │
                     └──────────────────────┼──────────────────┘
                                            │  Wi-Fi AP
                                     ┌──────▼──────┐
                                     │  Phone /    │
                                     │  Laptop     │
                                     │  (Browser)  │
                                     └─────────────┘

         ┌─────────────────────────────┐
         │   Arduino Nano (Sub-node)   │
         │   MPU6050 → Fall Detection  │
         │   UART TX ──► ESP32 RX      │
         └─────────────────────────────┘
```

---

## 🔧 Hardware Components

| # | Component | Quantity | Purpose |
|---|---|---|---|
| 1 | **ESP32 DevKit** | 1 | Main controller, Wi-Fi AP, web server |
| 2 | **Arduino Nano** | 1 | Fall detection sub-node |
| 3 | **MAX30100** Pulse Oximeter | 1 | Heart rate & SpO₂ measurement |
| 4 | **DHT11** Sensor | 1 | Temperature & humidity measurement |
| 5 | **Sound Sensor Module** | 1 | Breathing sound detection (apnea trigger) |
| 6 | **MPU6050** Accelerometer | 1 | Fall detection (on Nano) |
| 7 | **16×2 LCD** (HD44780) | 1 | Local real-time display |
| 8 | **5V Relay Module** | 1 | CPAP device control |
| 9 | **Buzzer** | 1 | Audible alert |
| 10 | **CPAP Machine** | 1 | Therapeutic airflow device |
| 11 | Jumper Wires, Breadboard | — | Connections |
| 12 | Power Supply (5V/3.3V) | — | Powering the circuit |

---

## 🔌 Circuit Connections

### ESP32 Pin Mapping

| Component | ESP32 GPIO |
|---|---|
| DHT11 (Data) | GPIO 4 |
| Sound Sensor (ADC) | GPIO 34 |
| Relay (CPAP control) | GPIO 23 |
| Buzzer | GPIO 5 |
| LCD RS | GPIO 14 |
| LCD EN | GPIO 27 |
| LCD D4 | GPIO 26 |
| LCD D5 | GPIO 25 |
| LCD D6 | GPIO 33 |
| LCD D7 | GPIO 32 |
| UART2 RX (from Nano TX) | GPIO 16 |
| UART2 TX (to Nano RX) | GPIO 17 |
| MAX30100 SDA | GPIO 21 |
| MAX30100 SCL | GPIO 22 |

### Arduino Nano Pin Mapping

| Component | Nano Pin |
|---|---|
| MPU6050 SDA | A4 |
| MPU6050 SCL | A5 |
| TX to ESP32 RX | D1 (TX) |

> ⚠️ **Important:** Ensure a **common GND** connection between the ESP32 and Arduino Nano.

---

## 📚 Software & Libraries

### ESP32 Libraries
| Library | Purpose | Install via |
|---|---|---|
| `WiFi.h` | Wi-Fi Access Point | Built-in ESP32 |
| `WebServer.h` | HTTP web server | Built-in ESP32 |
| `Wire.h` | I²C communication | Built-in |
| `DHT.h` | DHT11 sensor | Arduino Library Manager (`DHT sensor library` by Adafruit) |
| `LiquidCrystal.h` | LCD display | Built-in |
| `MAX30100_PulseOximeter.h` | Pulse oximeter | Arduino Library Manager (`MAX30100lib` by OXullo Intersecans) |

### Arduino Nano Libraries
| Library | Purpose | Install via |
|---|---|---|
| `Wire.h` | I²C communication | Built-in |
| `MPU6050.h` | Accelerometer | Arduino Library Manager (`MPU6050` by Electronic Cats) |

---

## 📁 Project Structure

```
iot-sleep-apnea-detection-system/
│
├── src/
│   ├── esp32_main/
│   │   └── esp32_main.ino          # ESP32 main controller sketch
│   │
│   └── nano_fall_detection/
│       └── nano_fall_detection.ino # Arduino Nano fall detection sketch
│
├── docs/
│   └── project_report.pdf          # Full mini project report
│
├── README.md                        # This file
└── LICENSE
```

---

## ⚙️ How It Works

### 1. Apnea Detection
The **sound sensor** continuously monitors breathing sounds. When the analog ADC value falls **below the threshold** (default: `2800`) for more than **6 seconds**, the system classifies it as an apnea event:
- 🔔 **Buzzer** sounds an immediate alert.
- ⚙️ **CPAP relay** is activated to restore airflow.
- 🌐 Dashboard shows CPAP status as **"ON"**.

### 2. Vital Signs Monitoring
The **MAX30100** pulse oximeter reads:
- **Heart Rate (BPM)** — shown on LCD and dashboard.
- **SpO₂ (%)** — critical for detecting oxygen desaturation.

### 3. Environment Monitoring
The **DHT11** reads ambient **temperature** and **humidity** — useful for assessing sleep environment quality.

### 4. Fall Detection
The **Arduino Nano** runs independently, reading the **MPU6050 accelerometer** every 100 ms. If a sudden impact (>2.5g) followed by near-zero acceleration (<0.3g) is detected, it sends `"FALL DETECTED"` to the ESP32 via UART.

### 5. Web Dashboard
The ESP32 runs as a **Wi-Fi Access Point**:
- **SSID:** `SleepApneaMonitor`
- **Password:** `sleep1234`
- **Dashboard URL:** `http://192.168.4.1`

Connect any device to the Wi-Fi and open the URL in a browser. The dashboard **auto-refreshes every 5 seconds**.

---

## 🌐 Web Dashboard

The dashboard displays all vitals in a clean, dark-themed card layout:

```
┌─────────────────────────────────────────────────┐
│         😴 IoT Sleep Apnea Monitor              │
│  ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐   │
│  │ Temp   │ │Humidity│ │ Sound  │ │  BPM   │   │
│  │ 27.3°C │ │  65%   │ │  3100  │ │ 72.0   │   │
│  └────────┘ └────────┘ └────────┘ └────────┘   │
│  ┌────────┐ ┌────────┐ ┌────────┐               │
│  │  SpO₂  │ │  Fall  │ │  CPAP  │               │
│  │  98%   │ │ Normal │ │  OFF   │               │
│  └────────┘ └────────┘ └────────┘               │
└─────────────────────────────────────────────────┘
```

---

## 🚀 Setup & Installation

### Step 1 — Install Arduino IDE
Download from [arduino.cc](https://www.arduino.cc/en/software) (v1.8+ or v2.x).

### Step 2 — Add ESP32 Board Support
1. Go to **File → Preferences**.
2. Add to "Additional Boards Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Boards Manager**, search **ESP32**, and install.

### Step 3 — Install Required Libraries
Open **Tools → Manage Libraries** and install:
- `DHT sensor library` by Adafruit
- `MAX30100lib` by OXullo Intersecans
- `MPU6050` by Electronic Cats

### Step 4 — Flash ESP32
1. Open `src/esp32_main/esp32_main.ino` in Arduino IDE.
2. Select **Board: "ESP32 Dev Module"** and the correct **COM port**.
3. Click **Upload**.

### Step 5 — Flash Arduino Nano
1. Open `src/nano_fall_detection/nano_fall_detection.ino`.
2. Select **Board: "Arduino Nano"** and the correct **COM port**.
3. Click **Upload**.

### Step 6 — Connect & Test
1. Power up both boards.
2. Connect your phone/laptop to Wi-Fi: **`SleepApneaMonitor`** (password: `sleep1234`).
3. Open browser → `http://192.168.4.1`
4. You should see the live dashboard! 🎉

---

## 👥 Team

| Name | Roll Number | Role |
|---|---|---|
| **Naveen Kumar N** | 922523106133 | Project Lead & ESP32 Firmware |
| **Saktibalan M** | 922523106178 | Hardware & Circuit Design |
| **Baranidharan S** | 922523106301 | Sensor Integration & Testing |
| **Manoj P** | 922523106302 | Arduino Nano & Fall Detection |

**Supervisor:** Dr. P. Surendar, M.E., Ph.D.  
**Department:** Electronics and Communication Engineering  
**Institution:** V.S.B. Engineering College, Karur — 639111  
**Affiliated to:** Anna University, Chennai | Approved by AICTE, New Delhi  

---

## 📄 License

This project is licensed under the [MIT License](LICENSE) — feel free to use and adapt for academic purposes.

---

<div align="center">
Made with ❤️ by Team Sleep Apnea IoT | V.S.B. Engineering College, 2026
</div>
