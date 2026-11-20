# 🪄 Chamber of Circuits: Voice-Controlled ESP32 Magic Wand & 10 IoT Smart Props

Build your own **wizard-inspired smart home** using **ESP32-C6**, **ESP32-S3**, **offline voice recognition**, and ultra-low-latency **ESP-NOW** wireless communication. Wave your wand, speak a Latin incantation, and control 10 interactive magical room props without Wi-Fi lag or cloud subscriptions.

> 📖 **GET THE COMPLETE ILLUSTRATED BUILD MANUAL:**  
> This repository hosts the `.ino` firmware files. Want **step-by-step wiring schematics, pinout tables, component shopping lists, and full assembly blueprints**?  
> **[👉 Download the Official Illustrated E-Book on Gumroad!](https://paunikar2.gumroad.com/l/chamber-of-circuits)**

---

## ✨ Included Projects & Firmware Nodes

Click any folder below to open the Arduino sketch for that specific prop:

| # | Project Name | Firmware Folder | Microcontroller | Hardware & Interactive Effect |
| :--- | :--- | :--- | :--- | :--- |
| 1 | **Master Magic Wand** | [`/Wand_Transmitter`](./Wand_Transmitter) | **ESP32-C6** | INMP441 I2S Mic + RGB/UV LEDs (Voice TX) |
| 2 | **Festive Tree Controller** | [`/Christmas_Tree_Receiver`](./Christmas_Tree_Receiver) | **ESP32-S3** | AC Power Relay toggles holiday string lights |
| 3 | **Enchanted Hearth Fireplace** | [`/Fireplace_Receiver`](./Fireplace_Receiver) | **ESP32-S3** | WS2812B NeoPixel realistic flame flicker |
| 4 | **Alchemist's Smoldering Cauldron** | [`/Smoldering_Pot_Receiver`](./Smoldering_Pot_Receiver) | **ESP32-S3** | 24V Relay + Ultrasonic mist maker prop |
| 5 | **Arcane Thunder Cloud Ceiling** | [`/Thunder_Cloud_Receiver`](./Thunder_Cloud_Receiver) | **ESP32-S3** | WS2812B LED animated lightning storm clouds |
| 6 | **Secret UV Tracker Map** | [`/Invisible_Map_Receiver`](./Invisible_Map_Receiver) | **ESP32-C6** | Hidden UV ink reveal mode *(10s auto-fade)* |
| 7 | **Botanical Auto-Planter** | [`/Herbology_Receiver`](./Herbology_Receiver) | **ESP32-S3** | 5V Relay + Submersible automated water pump |
| 8 | **Infinity Mirror Secret Portal** | [`/Portal_Mirror_Receiver`](./Portal_Mirror_Receiver) | **ESP32-S3** | Two-way mirror tunnel + electromagnetic lock |
| 9 | **Grand Dispatch Chest** | [`/Dispatch_Box_Receiver`](./Dispatch_Box_Receiver) | **ESP32-S3** | Servo / Linear actuator self-opening box |
| 10 | **Great Hall Color Banner** | [`/Feast_Hall_Banner_Receiver`](./Feast_Hall_Banner_Receiver) | **ESP32-S3** | WS2812B color-shifting house wall banner |

*(Note: Ensure the folder names in your repository match the links above—and remember to update your `.ino` voice-command strings if you change any spoken trigger phrases below!)*

---

## ⚡ Voice Command & ESP-NOW Spell Matrix

The **ESP32-C6 Wand** processes voice commands locally at **16kHz** and broadcasts **2-character ESP-NOW tokens** (`A1`–`J4`) to trigger receiver nodes in milliseconds:

| Spoken Incantation | Token (ON) | Token (OFF) | Target Node & Hardware Behavior |
| :--- | :---: | :---: | :--- |
| **"solis illumination"** | `A1` | `A0` | Toggles the Festive Tree AC Relay node |
| **"ignis hearth"** | `B1` | `B0` | Runs the Fireplace asynchronous NeoPixel flicker effect |
| **"vapors mist"** | `C1` | `C0` | Toggles the Smoldering Cauldron ultrasonic mist relay |
| **"tonitrus red"** | `D1` | `D0` | Triggers **Red** lightning storm cloud animation |
| **"tonitrus blue"** | `D2` | `D0` | Triggers **Blue** lightning storm cloud animation |
| **"tonitrus green"** | `D3` | `D0` | Triggers **Green** lightning storm cloud animation |
| **"tonitrus white"** | `D4` | `D0` | Triggers **White** lightning storm cloud animation |
| **"arcana ink"** | `E1` | `E0` | Activates Wand UV LED for secret ink *(10s auto-fade)* |
| **"flora grow"** | `F1` | `F0` | Triggers Botanical Planter automated watering relay |
| **"portal mirror"** | `G1` | `G0` | Toggles Infinity Mirror portal lighting & latch |
| **"dispatch box"** | `H1` | `H0` | Drives non-blocking servo loop to open Dispatch Chest |
| **"banner red"** | `J1` | `J0` | Illuminates Great Hall Banner in solid **Red** |
| **"banner blue"** | `J2` | `J0` | Illuminates Great Hall Banner in solid **Blue** |
| **"banner green"** | `J3` | `J0` | Illuminates Great Hall Banner in solid **Green** |
| **"banner yellow"** | `J4` | `J0` | Illuminates Great Hall Banner in solid **Yellow** |
| **"lux red"** | `I1` | `I0` | Sets local Wand tip RGB LED to solid **Red** |
| **"lux blue"** | `I2` | `I0` | Sets local Wand tip RGB LED to solid **Blue** |
| **"lux green"** | `I3` | `I0` | Sets local Wand tip RGB LED to solid **Green** |
| **"umbra"** | `I0` | `I0` | Globally fades out and turns off all local Wand LEDs |

---

## 💻 Environment Setup & Arduino IDE Deployment

Follow these steps to configure your computer and flash the firmware to your ESP32-C6 and ESP32-S3 boards without compilation errors:

### 1. USB-to-UART Bridge Driver Installation
If your computer fails to recognize your board under `Tools -> Port`, install the driver matching your board's USB chip and verify you are using a **data-transfer USB-C cable** (not a power-only charging cable):
* **CP210x Chips:** Download the [Silicon Labs CP210x VCP Driver](https://silabs.com).
* **CH340 / CH341 Chips:** Download the [WCH CH340 Driver](https://wch-ic.com).

### 2. Espressif ESP32 Board Core Setup
1. Open **Arduino IDE** and navigate to `File` -> `Preferences`.
2. In **Additional Boards Manager URLs**, paste the official Espressif index link:
   ```text
   https://github.io
   ```
3. Go to `Tools` -> `Board` -> `Boards Manager...`, search for **`esp32`** by **Espressif Systems**, and install the latest version (`v3.0+` is required for native ESP32-C6 I2S drivers).

### 3. Required Library Matrix
Open `Sketch` -> `Include Library` -> `Manage Libraries...` and install the dependencies needed for your chosen nodes:

| Project Folder Node | Required External Library |
| :--- | :--- |
| **1. Wand_Transmitter** | *None* (Uses built-in `WiFi.h`, `esp_now.h`, and `driver/i2s_std.h`) |
| **2. Feast_Hall_Banner_Receiver** | **Adafruit NeoPixel** (by Adafruit) |
| **3. Christmas_Tree_Receiver** | *None* (Uses native Espressif GPIO control) |
| **4. Fireplace_Receiver** | **Adafruit NeoPixel** (by Adafruit) |
| **5. Smoldering_Pot_Receiver** | *None* (Uses native Espressif GPIO control) |
| **6. Thunder_Cloud_Receiver** | **Adafruit NeoPixel** (by Adafruit) |
| **7. Invisible_Map_Receiver** | *None* (Uses native Espressif GPIO control) |
| **8. Herbology_Receiver** | *None* (Uses native Espressif GPIO control) |
| **9. Portal_Mirror_Receiver** | *None* (Uses native Espressif GPIO control) |
| **10. Dispatch_Box_Receiver** | **ESP32Servo** (by Kevin Harrington) |

---

## 🎙️ INMP441 Microphone Hardware Notes (Wand Transmitter)

The **Wand Transmitter** uses an **INMP441 omnidirectional digital microphone** connected to the ESP32-C6 over the **I2S (Inter-IC Sound)** bus. If you modify or debug the audio capture code, keep these hardware rules in mind:

* **Modern Core Drivers Only:** The firmware uses the stable `driver/i2s_std.h` API. Do not use legacy `driver/i2s.h` libraries, as they fail to compile on ESP32-C6 boards.
* **Hardware L/R Pin Grounding:** Tie the physical **L/R (Left/Right)** pin on the INMP441 directly to **GND** so it broadcasts on the Left audio channel:
  ```cpp
  std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
  ```
* **Sample Rate Matching:** The I2S audio stream is locked to **16kHz with a 32-bit slot width** for reliable voice command recognition. Lowering the sample rate will break frequency scaling.

---

## ⭐ Support & Disclaimer
* **Enjoying these builds?** Click **Star ⭐** at the top right of this page so other makers can discover the project!
* **Need full schematics, pinout diagrams & troubleshooting guides?** Get the **[Official Illustrated Build E-Book on Gumroad](https://paunikar2.gumroad.com/l/chamber-of-circuits)**.
* *Disclaimer: This is an independent, fan-inspired DIY electronics project using original Latin-style voice triggers and hardware designs. It is not affiliated with, endorsed by, or associated with Warner Bros., J.K. Rowling, or any commercial franchise.*
