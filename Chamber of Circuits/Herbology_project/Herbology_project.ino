#include <esp_now.h>
#include <WiFi.h>

// =========================================================================
// HARDWARE PIN CONFIGURATION
// =========================================================================
const int HERBOLOGY_PIN = 4;        // Pin connected to the grow system relay or driver
const bool GROW_ON_STATE = HIGH;    // HIGH if active-high relay/driver, LOW if active-low

// ESP-NOW Data structure matching the wand's payload format
typedef struct struct_message {
    char spellCode[3]; // Capacity for 2 characters + null terminator
} struct_message;

struct_message incomingData;

// =========================================================================
// ESP-NOW PACKET RECEPTION CALLBACK
// =========================================================================
void OnDataRecv(const esp_now_recv_info *recvInfo, const uint8_t *data, int len) {
  // Clear buffer and copy incoming packet data safely
  memset(&incomingData, 0, sizeof(incomingData));
  memcpy(&incomingData, data, min(len, (int)sizeof(incomingData) - 1));
  
  Serial.print("Magic Signal Received: ");
  Serial.println(incomingData.spellCode);

  // Analyze the 2-character code using string comparison (strcmp)
  if (strcmp(incomingData.spellCode, "F1") == 0) {
    Serial.println("Spell Acknowledged: Herbivicus Grow! Stimulating life elements...");
    digitalWrite(HERBOLOGY_PIN, GROW_ON_STATE);
  } 
  else if (strcmp(incomingData.spellCode, "F0") == 0) {
    Serial.println("Counter-Spell Acknowledged: Halting growth acceleration...");
    digitalWrite(HERBOLOGY_PIN, !GROW_ON_STATE);
  } 
  else {
    // Drop signals meant for other active magical items in the area
    Serial.println("Notice: Signal not meant for Herbology. Doing nothing.");
  }
}
 
void setup() {
  Serial.begin(115200);
  
  pinMode(HERBOLOGY_PIN, OUTPUT);
  digitalWrite(HERBOLOGY_PIN, !GROW_ON_STATE); // Keep device off on initial boot

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); 

  Serial.print("This ESP32 MAC Address is: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW!");
    return;
  }
  
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Herbology System Receiver Online. Waiting for 'F1' or 'F0'...");
}
 
void loop() {
  // Free execution loop for background interrupt handlers
  delay(10);
}
