#include <esp_now.h>
#include <WiFi.h>

// =========================================================================
// EASY CONFIGURATION ZONE: Adjust your automatic safety cutoff here!
// =========================================================================
const unsigned long STIR_TIMEOUT_MS = 30000;  // 30 seconds auto-shutoff timer

// HARDWARE PIN ASSIGNMENTS (CodeCell C6 Motor Driver Pins)
const int MOTOR_PIN_PLUS  = 5;   // Controls the M1+ line (Red wire)
const int MOTOR_PIN_MINUS = 6;   // Controls the M1- line (Black wire)

// =========================================================================
// GLOBAL STATE TRACKING VARIABLES
// =========================================================================
unsigned long fanStartTime = 0;
bool isFanActive = false;

// ESP-NOW Data structure matching the wand's payload format
typedef struct struct_message {
    char spellCode[3]; // Capacity for 2 characters + null terminator
} struct_message;

struct_message incomingData;

// Callback function executed whenever a spell packet arrives over the air
void OnDataRecv(const esp_now_recv_info *recvInfo, const uint8_t *data, int len) {
  memset(&incomingData, 0, sizeof(incomingData));
  memcpy(&incomingData, data, min(len, (int)sizeof(incomingData) - 1));
  
  Serial.print("Magic Signal Received: ");
  Serial.println(incomingData.spellCode);

  // -------------------------------------------------------------------------
  // SELF-STIRRING MUG FAN LOGIC MATRIX
  // -------------------------------------------------------------------------
  if (strcmp(incomingData.spellCode, "J1") == 0) {
    Serial.println("Spell Acknowledged! Spinning the whirlpool stirring fan...");
    
    // Drive the fan forward: set M1+ High and M1- Low
    digitalWrite(MOTOR_PIN_PLUS, HIGH);
    digitalWrite(MOTOR_PIN_MINUS, LOW);
    
    // Benchmark timing variables for safety window tracking
    fanStartTime = millis();
    isFanActive = true;
  } 
  else if (strcmp(incomingData.spellCode, "J0") == 0) {
    Serial.println("Counter-Spell Acknowledged! Braking the stirring fan...");
    
    // Kill power to the motor entirely
    digitalWrite(MOTOR_PIN_PLUS, LOW);
    digitalWrite(MOTOR_PIN_MINUS, LOW);
    isFanActive = false;
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize the motor driver pins as outputs
  pinMode(MOTOR_PIN_PLUS, OUTPUT);
  pinMode(MOTOR_PIN_MINUS, OUTPUT);
  
  // Ensure the mug starts completely resting/idle
  digitalWrite(MOTOR_PIN_PLUS, LOW);
  digitalWrite(MOTOR_PIN_MINUS, LOW);

  // Initialize Wi-Fi in Station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); 

  // Initialize ESP-NOW protocol
  if (esp_now_init() != ESP_OK) {
    Serial.println("Critical Error: Failed to initialize ESP-NOW inside the mug!");
    return;
  }

  // Register the data reception callback function
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("Sorcerer's Coffee Mug Online! Waiting for wand signals...");
}

void loop() {
  // AUTOMATIC MOTOR SAFETY TIMEOUT WATCHDOG RUNNING CONTINUOUSLY
  if (isFanActive && (millis() - fanStartTime >= STIR_TIMEOUT_MS)) {
    Serial.println("[Safety Timeout] Braking fan automatically to preserve battery...");
    
    digitalWrite(MOTOR_PIN_PLUS, LOW);
    digitalWrite(MOTOR_PIN_MINUS, LOW);
    
    isFanActive = false;
  }
  
  delay(10); // Smooth background loop breathing window
}
