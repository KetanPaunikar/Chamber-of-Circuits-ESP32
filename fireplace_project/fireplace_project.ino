#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 4
#define NUM_LEDS 30
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

bool fireActive = false;

// ESP-NOW Data structure matching the wand's payload format
typedef struct struct_message {
    char spellCode[3]; // Capacity for 2 characters + null terminator
} struct_message;

struct_message incomingData;

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *data, int len) {
  // Clear buffer and copy incoming packet data safely
  memset(&incomingData, 0, sizeof(incomingData));
  memcpy(&incomingData, data, min(len, (int)sizeof(incomingData) - 1));
  
  Serial.print("Magic Signal Received: ");
  Serial.println(incomingData.spellCode);

  // Analyze the 2-character code using string comparison (strcmp)
  if (strcmp(incomingData.spellCode, "B1") == 0) {
    Serial.println("Spell Acknowledged: Ignis Hearth activated!");
    fireActive = true;
  }
  else if (strcmp(incomingData.spellCode, "B0") == 0) {
    Serial.println("Counter-Spell Acknowledged: Quenching fireplace...");
    fireActive = false; 
    strip.clear(); 
    strip.show(); 
  }
}

void setup() {
  Serial.begin(115200); 
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Avoid home router latency hooks
  
  strip.begin(); 
  strip.show(); // Ensure NeoPixel strip starts completely turned off
  
  Serial.print("This ESP32 MAC Address is: "); 
  Serial.println(WiFi.macAddress());
  
  if (esp_now_init() == ESP_OK) {
    esp_now_register_recv_cb(OnDataRecv);
    Serial.println("ESP-NOW Ready. Waiting for 'B1' or 'B0' spell commands...");
  } else {
    Serial.println("Error initializing ESP-NOW!");
  }
}

void loop() {
  if (fireActive) {
    for (int i = 0; i < NUM_LEDS; i++) {
      int r = random(200, 255);     // High Red
      int g = random(50, 120);      // Pulsing Orange/Yellow
      int b = random(0, 10);        // Tiny Blue/Ember
      strip.setPixelColor(i, r, g, b);
    }
    strip.show();
    delay(random(50, 150));        // Organic fire flicker rate
  }
}
