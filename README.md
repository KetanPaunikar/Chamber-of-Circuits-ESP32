# 🪄 Harry Potter Interactive Wand & Magical Props Network

An automated wizarding network built using the **ESP-NOW** protocol. The system features an interactive **ESP32-C6 (CodeCell C6)** microphone-equipped Wand that processes spoken spells locally and broadcasts action tokens to target magical props scattered around the room.

---

## ⚡ The Magical Spell Matrix

The system maps spoken vocal incantations to short **2-character alphanumeric network tokens**. To save power, local LEDs and specific remote props utilize automated activity timers.

| Spoken Spell Phrase | Network Token (ON) | Counter Spell / Token (OFF) | Hardware Output Action & Behavior |
| :--- | :---: | :---: | :--- |
| **"solis illumination"** | `A1` | `A0` | Toggles the Christmas Tree Relay |
| **"ignis hearth"** | `B1` | `B0` | Toggles the Fireplace NeoPixel Flickering Machine |
| **"vapors mist"** | `C1` | `C0` | Toggles the Smoldering Pot / Mist Maker Relay |
| **"tonitrus red"** | `D1` | `D0` | Triggers Red Asynchronous Lightning Storm |
| **"tonitrus blue"** | `D2` | `D0` | Triggers Blue Asynchronous Lightning Storm |
| **"tonitrus green"** | `D3` | `D0` | Triggers Green Asynchronous Lightning Storm |
| **"tonitrus white"** | `D4` | `D0` | Triggers White Asynchronous Lightning Storm |
| **"revelio ink"** | `E1` | `E0` | Smoothly glows local Wand UV LED & Maps Scanner *(10s auto-fade)* |
| **"herbivicus grow"** | `F1` | `F0` | Toggles Herbology Grow Environment Relays |
| **"portal mirror"** | `G1` | `G0` | Toggles Infinite Portal Mirror light strips |
| **"dispatch box"** | `H1` | `H0` | Sweeps Servo latch engine to unpack Ministry Box |
| **"whirlpool stir"** | `J1` | `J0` | Drives internal Mug H-Bridge fan driver *(30s auto-stop)* |
| **"lumos red"** | `I1` | `I0` | Keeps local Wand Tip RGB glowing **Red** solid |
| **"lumos blue"** | `I2` | `I0` | Keeps local Wand Tip RGB glowing **Blue** solid |
| **"lumos green"** | `I3` | `I0` | Keeps local Wand Tip RGB glowing **Green** solid |
| **"nox"** | `I0` | `I0` | Globally kills and fades out all active local Wand LEDs |

---

## 🛠️ Hardware & Pin Layout Maps

### 1. Wand Transmitter (CodeCell C6)
*   **Push Button:** GPIO 23 *(Active LOW, triggers voice stream capture)*
*   **INMP441 Microphone:** WS = Pin 2, SCK = Pin 3, SD = Pin 1
*   **UV LED Cathode:** GPIO 10 *(Common Anode, LEDC PWM mapped)*
*   **RGB Red Cathode:** GPIO 22 *(Common Anode, LEDC PWM mapped)*
*   **RGB Blue Cathode:** GPIO 20 *(Common Anode, LEDC PWM mapped)*
*   **RGB Green Cathode:** GPIO 21 *(Common Anode, LEDC PWM mapped)*
*   *Note: Due to Common Anode configuration, writing `0` yields full brightness and `255` cuts power off.*

### 2. Self-Stirring Mug Receiver (CodeCell C6)
*   **Motor Driver M1+ Line:** GPIO 5
*   **Motor Driver M1- Line:** GPIO 6

### 3. Ministry Dispatch Box Receiver (ESP32)
*   **Servo Control Line:** GPIO 16 *(Utilizes `ESP32Servo` incremental safety sweep)*

---

## 📂 Repository File Architecture

To compile these codes successfully in the Arduino IDE without path errors, keep your directory structured exactly as shown below:

```text
├── README.md
├── Wand_Transmitter/
│   └── Wand_Transmitter.ino
├── Self_Stirring_Mug_Receiver/
│   └── Self_Stirring_Mug_Receiver.ino
├── Christmas_Tree_Receiver/
│   └── Christmas_Tree_Receiver.ino
├── Fireplace_Receiver/
│   └── Fireplace_Receiver.ino
├── Smoldering_Pot_Receiver/
│   └── Smoldering_Pot_Receiver.ino
├── Thunder_Cloud_Receiver/
│   └── Thunder_Cloud_Receiver.ino
├── Invisible_Map_Receiver/
│   └── Invisible_Map_Receiver.ino
├── Herbology_Receiver/
│   └── Herbology_Receiver.ino
├── Portal_Mirror_Receiver/
│   └── Portal_Mirror_Receiver.ino
└── Dispatch_Box_Receiver/
    └── Dispatch_Box_Receiver.ino
```

## 🔌 Requirements & Libraries
Ensure you have installed the following dependencies in your Arduino IDE Library Manager before uploading:
*   `Adafruit_NeoPixel` (Used for Fireplace and Thunder Cloud strings)
*   `ESP32Servo` (Used for the smooth non-blocking Ministry Box lid sweep)
