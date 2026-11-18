# 🪄 Chamber of Circuits: Distributed Interactive Wand & Magical Props Network

An enterprise-grade automation architecture built using the low-latency **ESP-NOW** protocol. This repository hosts the distributed firmware nodes connecting a voice-triggered **ESP32-C6 Wand** to an array of interactive physical props.

> 📖 **GET THE COMPLETE BUILD GUIDE:** Want the step-by-step wiring schematics, component list and full assembly manual? **[👉 Download the Official E-Book on Gumroad!](https://paunikar2.gumroad.com/l/chamber-of-circuits)**
---

## ⚡ The Magical Spell Matrix

The network maps real-time vocal incantations to localized **2-character alphanumeric tokens**. System elements utilize asynchronous power-saving routines and safety timeouts to optimize battery lifecycle performance.

| Spoken Spell Phrase | Network Token (ON) | Token (OFF) | Hardware Output Action & Node Behavior |
| :--- | :---: | :---: | :--- |
| **"solis illumination"** | `A1` | `A0` | Toggles the Christmas Tree Relay node |
| **"ignis hearth"** | `B1` | `B0` | Runs the Fireplace Asynchronous NeoPixel Flicker Machine |
| **"vapors mist"** | `C1` | `C0` | Toggles the Smoldering Pot / Mist Maker Relay node |
| **"tonitrus red"** | `D1` | `D0` | Triggers Red Asynchronous Lightning Storm Cloud matrix |
| **"tonitrus blue"** | `D2` | `D0` | Triggers Blue Asynchronous Lightning Storm Cloud matrix |
| **"tonitrus green"** | `D3` | `D0` | Triggers Green Asynchronous Lightning Storm Cloud matrix |
| **"tonitrus white"** | `D4` | `D0` | Triggers White Asynchronous Lightning Storm Cloud matrix |
| **"revelio ink"** | `E1` | `E0` | Glows local Wand UV LED & Maps Scanner *(10s auto-fade)* |
| **"herbivicus grow"** | `F1` | `F0` | Toggles Herbology Grow Environment Relays |
| **"portal mirror"** | `G1` | `G0` | Toggles Infinite Portal Mirror solid lighting |
| **"dispatch box"** | `H1` | `H0` | Drives non-blocking Servo loop to unlatch Ministry Box |
| **"feast red"** | `J1` | `J0` | Illuminates Feast Hall Banner project with solid **Red** |
| **"feast blue"** | `J2` | `J0` | Illuminates Feast Hall Banner project with solid **Blue** |
| **"feast green"** | `J3` | `J0` | Illuminates Feast Hall Banner project with solid **Green** |
| **"feast yellow"** | `J4` | `J0` | Illuminates Feast Hall Banner project with solid **Yellow** |
| **"lumos red"** | `I1` | `I0` | Pins local Wand Tip RGB to solid **Red** |
| **"lumos blue"** | `I2` | `I0` | Pins local Wand Tip RGB to solid **Blue** |
| **"lumos green"** | `I3` | `I0` | Pins local Wand Tip RGB to solid **Green** |
| **"nox"** | `I0` | `I0` | Globally terminates and fades out all active local Wand LEDs |

---

## 💻 Environment Setup & Toolchain Deployment

To interface your laptop with your ESP32 / ESP32-C6 hardware nodes and compile the software without errors, follow this structural deployment guide.

### 1. USB-to-UART Bridge VDT Driver Installation
If your computer fails to recognize your microcontrollers when plugged in via USB, you need to install the virtual COM port hardware drivers:
*   **For CH340 / CH341 chips:** Download and install the [WCH CH340 Driver](https://wch-ic.com).
*   **For CP210x chips:** Download and install the [Silicon Labs CP210x VCP Driver](https://silabs.com).
*   *Note: Ensure you use a high-quality USB-C cable capable of **data transmission**, not just a cheap power-only charging cord.*

### 2. Espressif ESP32 Core Setup
1. Inside the **Arduino IDE**, navigate to `File` -> `Preferences`.
2. Locate the **Additional Boards Manager URLs** field and append this official stable index link:
   `https://github.io`
3. Go to `Tools` -> `Board` -> `Boards Manager...`, search for `esp32` by **Espressif Systems**, and click **Install**.

### 3. Required Library Matrix
Open your Library Manager (`Tools` -> `Manage Libraries...`) and map these exact dependencies based on the specific nodes you wish to deploy:

| Project Folder Node | Required External Library Dependency |
| :--- | :--- |
| **1. Wand_Transmitter** | *None* (Uses native `WiFi.h`, `esp_now.h`, and `driver/i2s_std.h`) |
| **2. Feast_Hall_Banner_Receiver** | **Adafruit_NeoPixel** (by Adafruit) |
| **3. Christmas_Tree_Receiver** | *None* (Uses native Espressif GPIO control for relay arrays) |
| **4. Fireplace_Receiver** | **Adafruit_NeoPixel** (by Adafruit) |
| **5. Smoldering_Pot_Receiver** | *None* (Uses native Espressif GPIO control for relay arrays) |
| **6. Thunder_Cloud_Receiver** | **Adafruit_NeoPixel** (by Adafruit) |
| **7. Invisible_Map_Receiver** | *None* (Uses native Espressif GPIO control) |
| **8. Herbology_Receiver** | *None* (Uses native Espressif GPIO control) |
| **9. Portal_Mirror_Receiver** | *None* (Uses native Espressif GPIO control) |
| **10. Dispatch_Box_Receiver** | **ESP32Servo** (by Kevin Harrington) |

---

## 🎙️ INMP441 Microphone Hardware Architecture Notes

The **Wand Transmitter** utilizes an **INMP441 omnidirectional digital microphone** interfacing directly with the ESP32-C6 over the high-speed **I2S (Inter-IC Sound)** bus. If you modify or debug the core sample capturing logic, adhere to these explicit constraints:

*   **Native Core Drivers:** The code utilizes the modern, stable `driver/i2s_std.h` framework. Avoid old legacy I2S driver methods as they cause compilation errors on newer ESP32-C6 architectures.
*   **Hardware L/R Pin Trapping:** The physical **L/R (Left/Right) configuration pin** on your INMP441 module **must be tied securely to GND**. This configures the sensor to broadcast data on the Left audio slot channel. The firmware initialization structure isolates this profile via:
    ```cpp
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    ```
*   **Sample Rate Matching:** The audio pipe is tuned to a standard **16kHz sample rate with 32-bit slot width allocation** to interface seamlessly with modern embedded digital voice recognition and acoustic DSP pipelines. Do not drop below 16kHz or your frequency capturing scaling will break.
*   ### 🏷️ Tags & Related Topics
`esp32-projects` `esp32-c6-mini` `seeed-xiao-esp32-c6` `xiao-esp32-c6` `waveshare-esp32-c6` `esp32-s3-board` `esp32-s31` `esp8266` `esp-idf` `esp-now` `smart-home-props`
