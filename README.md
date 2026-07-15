# ESP32 Minimalist Hardware Monitor

A sleek, high-density PC hardware telemetry dashboard that runs on an ESP32 with a 128x64 SSD1306 OLED display. 

This project strips away cluttered split-screens and bubbly UI elements in favor of a hardcore, unified developer aesthetic. It uses a dynamic Python backend to fetch system data and beams it over Serial to the ESP32.

## 🚀 Features

* **Dynamic Spacing Engine:** The layout automatically detects missing sensors (like laptop GPUs or unreadable fans) and recalculates the vertical padding to perfectly distribute the remaining metrics across the screen.
* **Custom Clock Equalizer:** CPU frequency is visualized as an ascending signal-bar block graphic that scales with your live clock speed.
* **Precision Progress Tracks:** Thick 4px horizontal tracks with a 2px solid fill for CPU Temp, GPU Temp, and RAM utilization.
* **Smart CPU Identification:** Automatically strips marketing fluff (like "Intel", "Core", "TM") to display just the raw model number (e.g., `i7-12700K` or `R7 5800X`).
* **Clean Status Bar:** Top-level 12-hour AM/PM time and a phone-style battery monitor complete with a custom vector lightning bolt ⚡ when charging.
* **Cross-Platform:** Runs natively on Linux and Windows.

## 🛠️ Hardware Requirements
* ESP32 Development Board (e.g., NodeMCU ESP32)
* 0.96" SSD1306 OLED Display (128x64)
* 4 Jumper Wires (I2C: SDA to Pin 33, SCL to Pin 32, VCC, GND)

## 💻 Software Installation

### 1. The ESP32 Firmware (PlatformIO)
This project is built using PlatformIO.
1. Open the project folder in VS Code with the PlatformIO extension installed.
2. The `platformio.ini` file is already configured for the `esp32dev` board and includes the necessary Adafruit SSD1306 and GFX libraries.
3. Build and upload the project to your ESP32.

### 2. The Python Backend
The Python script requires a few dependencies to read your system sensors.

**For Arch Linux / CachyOS:**
Install the packages natively to avoid virtual environment conflicts:
```bash
sudo pacman -S python-py-cpuinfo python-pyserial python-psutil
## ⚙️ Configuration

Before running the dashboard, you need to tell the Python script which USB port your ESP32 is plugged into. 

1. **Find your Port:**
   * **On Linux (CachyOS/Arch):** Open your terminal and run `ls /dev/tty*`. Plug in your ESP32 and run it again. The new device that appears is your ESP32 (usually `/dev/ttyUSB0` or `/dev/ttyACM0`).
   * **On Windows:** Right-click the Start button, open **Device Manager**, and look under "Ports (COM & LPT)". Note the COM number (e.g., `COM3`).

2. **Update `monitor.py`:**
   Open the Python script in your text editor and locate this block near the top:
   ```python
   if OS_TYPE == 'Windows':
       SERIAL_PORT = 'COM3'  # <-- Change this to your Windows COM port
   else:
       SERIAL_PORT = '/dev/ttyUSB0' # <-- Change this to your Linux port
