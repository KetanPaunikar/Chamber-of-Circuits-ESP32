#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 4
#define NUM_LEDS 30
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Global storm configuration state engines
bool cloudActive = false;
int stormR = 0, stormG = 0, stormB = 0; 

// Track background timing variables to flicker without using delay()
unsigned long lastFlickerTime = 0;
int flickerInterval = 100;

// ESP-NOW Data structure matching the wand's payload format
typedef struct struct_message {
    char spellCode[3]; // Capacity for 2 characters + null terminator
} struct_message;

struct_message incomingData;

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *data, int len) {
  memset(&incomingData, 0, sizeof(incomingData));
  memcpy(&incomingData, data, min(len, (int)sizeof(incomingData) - 1));
  
  Serial.print("Magic Signal Received: ");
  Serial.println(incomingData.spellCode);

  // -------------------------------------------------------------------------
  // THUNDER CLOUD COLOR MATRIX PROCESSING INTERRUPT
  // -------------------------------------------------------------------------
  if (strcmp(incomingData.spellCode, "D1") == 0) {
    Serial.println("Spell Acknowledged: Tonitrus RED activated!");
    cloudActive = true; stormR = 255; stormG = 0; stormB = 0;
  }
  else if (strcmp(incomingData.spellCode, "D2") == 0) {
    Serial.println("Spell Acknowledged: Tonitrus BLUE activated!");
    cloudActive = true; stormR = 0; stormG = 0; stormB = 255;
  }
  else if (strcmp(incomingData.spellCode, "D3") == 0) {
    Serial.println("Spell Acknowledged: Tonitrus GREEN activated!");
    cloudActive = true; stormR = 0; stormG = 255; stormB = 0;
  }
  else if (strcmp(incomingData.spellCode, "D4") == 0) {
    Serial.println("Spell Acknowledged: Tonitrus WHITE activated!");
    cloudActive = true; stormR = 255; stormG = 255; stormB = 255;
  }
  else if (strcmp(incomingData.spellCode, "D0") == 0) {
    Serial.println("Counter-Spell Acknowledged: Calming storm cloud...");
    cloudActive = false; 
    strip.clear(); 
    strip.show(); 
  }
}

void setup() {
  Serial.begin(115200); 
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); 
  
  strip.begin(); 
  strip.show(); // Ensure strip boots up clean and off
  
  Serial.print("This ESP32 MAC Address is: "); 
  Serial.println(WiFi.macAddress());
  
  if (esp_now_init() == ESP_OK) {
    esp_now_register_recv_cb(OnDataRecv);
    Serial.println("ESP-NOW Ready. Waiting for storm cloud spell codes...");
  } else {
    Serial.println("Error initializing ESP-NOW!");
  }
}

void loop() {
  // Asynchronous lightning machine: triggers only when active and interval passes
  if (cloudActive && (millis() - lastFlickerTime >= flickerInterval)) {
    lastFlickerTime = millis();
    flickerInterval = random(30, 200); // Randomize step gaps for real crackling rhythm

    // Randomize brightness intensity to mock authentic electrical lightning flashes
    float brightnessFactor = random(20, 100) / 100.0;
    int currentR = stormR * brightnessFactor;
    int currentG = stormG * brightnessFactor;
    int currentB = stormB * brightnessFactor;

    for (int i = 0; i < NUM_LEDS; i++) {
      // Randomly leave some pixels dim or fully bright to avoid a flat static look
      if (random(0, 10) > 3) {
        strip.setPixelColor(i, currentR, currentG, currentB);
      } else {
        strip.setPixelColor(i, 0, 0, 0); // Temporary black pocket/crackle shadow
      }
    }
    strip.show();
  }
}
