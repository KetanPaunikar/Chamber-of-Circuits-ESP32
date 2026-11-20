#include <esp_now.h>
#include <WiFi.h>

// ==========================================
// STUDENT CONFIGURATION ZONE (CHANGE THESE)
// ==========================================
const int RELAY_PIN = 4;            // The GPIO pin connected to the relay (Green wire)

// Set to HIGH if your red relay module jumper is set to 'H' (High Level Trigger)
// Set to LOW if your red relay module jumper is set to 'L' (Low Level Trigger)
const bool RELAY_ON_STATE = HIGH; 

// ESP-NOW Data structure matching the wand's payload format
typedef struct struct_message {
    char spellCode[3]; // Capacity for 2 characters + null terminator
} struct_message;

struct_message incomingData;

// ==========================================
// CALLBACK FUNCTION (TRIGGERED WHEN DATA RECEIVED)
// ==========================================
void OnDataRecv(const esp_now_recv_info *recvInfo, const uint8_t *data, int len) {
  // Clear buffer and copy incoming packet data safely
  memset(&incomingData, 0, sizeof(incomingData));
  memcpy(&incomingData, data, min(len, (int)sizeof(incomingData) - 1));
  
  // Print the raw string data to the Serial Monitor
  Serial.print("Magic Signal Received: ");
  Serial.println(incomingData.spellCode);

  // Analyze the 2-character code using string comparison (strcmp)
  if (strcmp(incomingData.spellCode, "C1") == 0) {
    // If it matches 'C1' (vapors mist), activate the mist maker
    digitalWrite(RELAY_PIN, RELAY_ON_STATE);
    Serial.println("Action: Mist Maker / Atomizer turned ON!");
  } 
  else if (strcmp(incomingData.spellCode, "C0") == 0) {
    // If it matches 'C0' (counter spell / nox), deactivate the mist maker
    digitalWrite(RELAY_PIN, !RELAY_ON_STATE);
    Serial.println("Action: Mist Maker / Atomizer turned OFF!");
  } 
  else {
    // Ignore signals intended for other props around the room
    Serial.println("Notice: Signal not meant for the Smoldering Pot. Doing nothing.");
  }
}
 
void setup() {
  // Initialize Serial Monitor at 115200 baud rate for debugging output
  Serial.begin(115200);
  
  // Set the relay signal pin as an OUTPUT
  pinMode(RELAY_PIN, OUTPUT);
  
  // Keep the device turned OFF when it first boots up safely
  digitalWrite(RELAY_PIN, !RELAY_ON_STATE);

  // Set the ESP32 Wi-Fi mode to Station mode (required for ESP-NOW radio)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Prevent auto-connecting to home routers for minimal latency

  // Print this device's MAC Address so you can copy it into your wand transmitter code
  Serial.print("This ESP32 MAC Address is: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW wireless communication protocol
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW!");
    return;
  }
  
  // Register the callback function to handle incoming data automatically
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("ESP-NOW Ready. Waiting for 'C1' or 'C0' spell commands...");
}
 
void loop() {
  // Loop remains completely empty because ESP-NOW works instantly using background listener events
  delay(10);
}
