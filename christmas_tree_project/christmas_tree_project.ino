#include <esp_now.h>
#include <WiFi.h>

// ==========================================
// STUDENT CONFIGURATION ZONE (CHANGE THESE)
// ==========================================
const int RELAY_PIN = 4;          // The GPIO pin connected to the relay module
const bool RELAY_ON_STATE = HIGH; // Set to HIGH if your relay turns on with a high signal, or LOW if it turns on with a low signal

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
  
  // Print the raw string to the Serial Monitor
  Serial.print("Magic Signal Received: ");
  Serial.println(incomingData.spellCode);

  // Analyze the 2-character code using string comparison (strcmp)
  if (strcmp(incomingData.spellCode, "A1") == 0) {
    // If the code is 'A1' (solis illumination), turn the light ON
    digitalWrite(RELAY_PIN, RELAY_ON_STATE);
    Serial.println("Action: Christmas Tree turned ON!");
  } 
  else if (strcmp(incomingData.spellCode, "A0") == 0) {
    // If the code is 'A0' (counter spell / nox), turn the light OFF
    digitalWrite(RELAY_PIN, !RELAY_ON_STATE);
    Serial.println("Action: Christmas Tree turned OFF!");
  } 
  else {
    // If the sender sends something else intended for other props, ignore it
    Serial.println("Notice: Signal not meant for the Christmas Tree. Doing nothing.");
  }
}
 
void setup() {
  Serial.begin(115200);
  
  // Configure the relay pin as an OUTPUT pin
  pinMode(RELAY_PIN, OUTPUT);
  
  // Set the default startup state to OFF
  digitalWrite(RELAY_PIN, !RELAY_ON_STATE);

  // Set the ESP32 Wi-Fi mode to Station (required for ESP-NOW)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Disconnect from home router to ensure maximum stability

  // Print the ESP32 MAC Address so you can configure your transmitter wand
  Serial.print("This ESP32 MAC Address is: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW protocol
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW!");
    return;
  }
  
  // Register the callback function to handle incoming data automatically
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("ESP-NOW Ready. Waiting for 'A1' or 'A0' spell commands...");
}
 
void loop() {
  // Loop remains empty as ESP-NOW operates asynchronously using interrupts
  delay(10);
}
