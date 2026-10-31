#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>

// ====================================================================
// STUDENT CONFIGURATION ZONE (VARIABLES ON TOP)
// ====================================================================
const int SERVO_PIN = 16;        // The GPIO pin connected to the Servo Signal (Yellow wire)
const int SERVO_DELAY = 15;      // Speed control: Lower number = Faster opening, Higher = Slower smooth sweep

const int ANGLE_CLOSED = 0;      // Servo angle when the lid is flat/closed (0-180)
const int ANGLE_OPEN   = 90;     // Servo angle when the lid is fully open (Adjust if your lid needs 120 or 180 degrees)

// ESP-NOW Data structure matching the wand's payload format
typedef struct struct_message {
    char spellCode[3]; // Capacity for 2 characters + null terminator
} struct_message;

struct_message incomingData;

// ====================================================================
// SYSTEM SETUP & OPERATIONAL TRACKING VARIABLES
// ====================================================================
Servo myServo;                 // Create the servo object
int currentAngle = 0;          // Tracks the current active physical position of the arm
int targetAngle = 0;           // Tracks the target position set by the incoming wireless commands

// ====================================================================
// ESP-NOW CALLBACK FUNCTION (TRIGGERS LID STATE INSTANTLY)
// ====================================================================
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *data, int len) {
  // Clear buffer and copy incoming packet data safely
  memset(&incomingData, 0, sizeof(incomingData));
  memcpy(&incomingData, data, min(len, (int)sizeof(incomingData) - 1));

  Serial.print("Box Lid Command Received: '");
  Serial.print(incomingData.spellCode);
  Serial.println("'");

  // Analyze the 2-character code using string comparison (strcmp) to match wand commands
  if (strcmp(incomingData.spellCode, "H1") == 0) {
    targetAngle = ANGLE_OPEN;
    Serial.println("Action: Unlatching and OPENING the box lid...");
  } 
  else if (strcmp(incomingData.spellCode, "H0") == 0) {
    targetAngle = ANGLE_CLOSED;
    Serial.println("Action: Latching and CLOSING the box lid...");
  } 
  else {
    Serial.println("Notice: Unassigned data packet received. Lid maintaining current state.");
  }
}

// ====================================================================
// SETUP FUNCTION
// ====================================================================
void setup() {
  Serial.begin(115200);

  // Allocate a hardware timer channel and configure the servo library frequency
  ESP32PWM::allocateTimer(0);
  myServo.setPeriodHertz(50);             // Standard 50Hz frequency for SG90/MG90S servos
  myServo.attach(SERVO_PIN, 500, 2400);   // Attaches the pin with standard minimum and maximum pulse widths
  
  // Safety Boot Measure: Force the box lid to remain securely CLOSED on startup
  currentAngle = ANGLE_CLOSED;
  targetAngle = ANGLE_CLOSED;
  myServo.write(currentAngle);

  // Initialize internal Wi-Fi radio for background ESP-NOW execution
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Clear association hooks for cleaner wireless response times
  
  Serial.print("Box Lid Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Critical Error: ESP-NOW protocol failed to start!");
    return;
  }
  
  // Link the background data tracking function to the radio antenna
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Lid Controller Active! Awaiting wireless H1 or H0 spell commands...");
}

// ====================================================================
// MAIN LOOP: SMOOTH POSITION TRANSIT SWEEP ENGINE
// ====================================================================
void loop() {
  
  // Instead of snapping hard (which strips plastic gears and yanks loose glue joints),
  // this sweeps the arm incrementally toward the target position step-by-step
  if (currentAngle < targetAngle) {
    currentAngle++;
    myServo.write(currentAngle);
    delay(SERVO_DELAY); // Wait a few milliseconds before the next degree step
  } 
  else if (currentAngle > targetAngle) {
    currentAngle--;
    myServo.write(currentAngle);
    delay(SERVO_DELAY);
  }
  
  delay(1); // Baseline loop clock synchronization step
}
