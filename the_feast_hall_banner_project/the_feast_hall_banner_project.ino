#include <esp_now.h>
#include <WiFi.h>
#include <FastLED.h>

// --- Configuration ---
#define LED_PIN     14          // Data pin connected to GPIO 14
#define NUM_LEDS    60          // Standard 1-meter strip count (Adjust if your strip differs)
#define BRIGHTNESS  200         // Maximum brightness (0-255)
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// --- Magical Transition Variables ---
CRGB targetColor = CRGB::Black;
CRGB currentColor = CRGB::Black;
const uint8_t fadeSpeed = 8;    // Higher = faster transition, Lower = slower & smoother

// --- ESP-NOW Message Structure ---
struct Message {
    char command[4]; // To safely store strings like "J0", "J1", etc.
};
Message incomingMessage;

// --- ESP-NOW Callback Function ---
void onDataRecv(const esp_now_recv_info_t *recvInfo, const uint8_t *data, int len) {
    if (len >= sizeof(Message)) {
        memcpy(&incomingMessage, data, sizeof(Message));
        
        // Strip trailing spaces or null-terminate safely
        String cmd = String(incomingMessage.command);
        cmd.trim();

        Serial.print("Received Magic Command: ");
        Serial.println(cmd);

        // Map commands to House Colors
        if (cmd == "J1") {
            targetColor = CRGB::Red;         // Gryffindor
        } else if (cmd == "J2") {
            targetColor = CRGB::Blue;        // Ravenclaw
        } else if (cmd == "J3") {
            targetColor = CRGB::Green;       // Slytherin
        } else if (cmd == "J4") {
            targetColor = CRGB::Gold;        // Hufflepuff (Yellow/Gold)
        } else if (cmd == "J0") {
            targetColor = CRGB::Black;       // Off (Nox)
        }
    }
}

void setup() {
    Serial.begin(115200);

    // Initialize LED strip
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    // Initialize WiFi in Station Mode for ESP-NOW
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    Serial.print("ESP32-S3 MAC Address: ");
    Serial.println(WiFi.macAddress());

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register receive callback
    esp_now_register_recv_cb(onDataRecv);
    Serial.println("Ready to receive house commands...");
}

void loop() {
    // Non-blocking magical fade effect loop
    EVERY_N_MILLISECONDS(20) {
        bool transitionDone = true;
        
        for (int i = 0; i < NUM_LEDS; i++) {
            // Blend current color toward target color smoothly
            leds[i] = blend(leds[i], targetColor, fadeSpeed);
            
            // Add a subtle magical sparkle/flicker effect if transitioning or active
            if (targetColor != CRGB::Black && random8() < 2) {
                leds[i] += CRGB(20, 20, 20); // Soft white overlay shimmer
            }
        }
        
        FastLED.show();
    }
}
