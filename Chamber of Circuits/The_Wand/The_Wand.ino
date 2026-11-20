#include <esp_now.h>
#include <WiFi.h>
#include <driver/i2s.h> 

// ====================================================================
// STUDENT CONFIGURATION ZONE (VARIABLES ON TOP FOR EASY UPDATES)
// ====================================================================
const int BUTTON_PIN = 23;      // Push-button input pin
const int UV_LED_PIN = 21;      // Pin connected to your UV LED (purple dot)

// RGB LED output pins (Common Cathode style configuration)
const int RGB_RED_PIN = 02;     
const int RGB_GREEN_PIN = 03;   
const int RGB_BLUE_PIN = 01;    

// INMP441 Microphone Pin layout
const int I2S_WS_PIN = 02;      // Word Select
const int I2S_SCK_PIN = 03;     // Serial Clock
const int I2S_SD_PIN = 22;      // Serial Data (Blue wire)

// --- MICROPHONE SIGNAL PROCESSING CONFIGURATION ---
const int32_t AUDIO_THRESHOLD = 45000; // Speech volume limit (Raise if background chatter triggers it)
const int INTEGRATION_WINDOW = 30;     // Number of processing loops a sound must sustain to count as a word
const int SILENCE_TIMEOUT = 180;       // Milliseconds of continuous quiet needed to mark the gap between syllables

// --- BROADCAST NETWORK CHANNEL ---
uint8_t receiverMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// --- MAGICAL DICTIONARY: SPELL BEAT SIGNATURES ---
struct SpellPattern {
  String spellName;
  int expectedPulses; 
  char dataPayload;   
};

SpellPattern spellBook[] = {
  { "Nox",           1,  'n' },  // 1 Vocal Burst -> Local UV LED OFF
  { "Lumos",         2,  'l' },  // 2 Vocal Bursts -> Local UV LED ON
  { "Lumos Red",     3,  'a' },  // 3 Vocal Bursts -> Local RGB RED
  { "Lumos Blue",    3,  'b' },  // 3 Vocal Bursts -> Local RGB BLUE
  { "Lumos Green",   3,  'c' },  // 3 Vocal Bursts -> Local RGB GREEN
  { "Nox Light",     2,  'x' },  // 2 Vocal Bursts -> Local RGB OFF
  
  // Custom Wireless Project Prop Target Mappings:
  { "Incendio",      3,  'p' },  // 3 Vocal Bursts -> Activates Water Pump module
  { "Aguamenti",     4,  's' },  // 4 Vocal Bursts -> Deactivates Water Pump module
  { "Alohomora",     4,  'o' },  // 4 Vocal Bursts -> Opens Box Servo Lid
  { "Colloportus",   4,  'c' },  // 4 Vocal Bursts -> Closes Box Servo Lid
  { "Verdimillious", 4,  'e' }   // 4 Vocal Bursts -> Triggers Flag to Purple
};

// ====================================================================
// SYSTEM SETUP INTERNALS
// ====================================================================
const int TOTAL_SPELLS = sizeof(spellBook) / sizeof(spellBook);
#define DMA_BUFFER_SIZE 256
int32_t audioBuffer[DMA_BUFFER_SIZE];

typedef struct struct_message {
    char dataChar;
} struct_message;

struct_message wandPacket;
esp_now_peer_info_t peerInfo;

void initMicrophone() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = DMA_BUFFER_SIZE,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK_PIN,
    .ws_io_num = I2S_WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD_PIN
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

void setup() {
  Serial.begin(115200);

  pinMode(UV_LED_PIN, OUTPUT);
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  
  digitalWrite(UV_LED_PIN, LOW);
  analogWrite(RGB_RED_PIN, 0); analogWrite(RGB_GREEN_PIN, 0); analogWrite(RGB_BLUE_PIN, 0);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  initMicrophone();

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW!");
    return;
  }

  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  Serial.println("=================================================");
  Serial.println("🧙‍♂️ PRODUCTION-GRADE WAND FIRMWARE READY");
  Serial.println("=================================================");
}

// ====================================================================
// MAIN OPERATIONAL EXECUTION ENGINE
// ====================================================================
void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("\n🎤 Wand actively recording... Speak spell now!");
    
    digitalWrite(UV_LED_PIN, HIGH); delay(60); digitalWrite(UV_LED_PIN, LOW);

    int detectedSyllables = 0;
    bool speechOngoing = false;
    int voiceSustainCounter = 0;
    unsigned long lastVoiceTime = 0;
    unsigned long listeningWindow = millis();

    // Loop continuously for a clean 2-second capturing window
    while (millis() - listeningWindow < 2000) {
      size_t bytesRead = 0;
      
      // Pull an entire chunk stream block out of I2S DMA memory
      i2s_read(I2S_NUM_0, &audioBuffer, sizeof(audioBuffer), &bytesRead, portMAX_DELAY);
      int samplesCount = bytesRead / 4;

      if (samplesCount > 0) {
        // Calculate the average absolute magnitude of this brief sound window
        long long sumAmplitude = 0;
        for (int i = 0; i < samplesCount; i++) {
          sumAmplitude += abs(audioBuffer[i]);
        }
        int32_t averageVolume = sumAmplitude / samplesCount;

        // ENERGY WINDOW CRITICAL VALIDATION
        if (averageVolume > AUDIO_THRESHOLD) {
          lastVoiceTime = millis(); // Reset structural silence countdown clock
          
          if (!speechOngoing) {
            voiceSustainCounter++;
            // Only confirm it as a real syllable if it lasts long enough to be human speech (filters out pops/clicks)
            if (voiceSustainCounter > INTEGRATION_WINDOW) {
              detectedSyllables++;
              speechOngoing = true;
              Serial.print("   🔊 Validated Syllable Tracked! Current Count: ");
              Serial.println(detectedSyllables);
            }
          }
        } else {
          // If the signal drops below threshold, reset sustain counters
          if (!speechOngoing) {
            voiceSustainCounter = 0;
          }
          
          // SILENCE TRACKER
          // Clear speech ongoing variable only if total silence persists longer than the gap window
          if (speechOngoing && (millis() - lastVoiceTime > SILENCE_TIMEOUT)) {
            speechOngoing = false;
            voiceSustainCounter = 0;
          }
        }
      }
    }

    Serial.println("⏹️ Processing final footprint profile...");
    Serial.print("-> Recognized Spoken Syllable Metric: ");
    Serial.println(detectedSyllables);

    bool matchFound = false;
    for (int i = 0; i < TOTAL_SPELLS; i++) {
      if (detectedSyllables == spellBook[i].expectedPulses) {
        char cmd = spellBook[i].dataPayload;
        matchFound = true;

        Serial.print("✨ SPELL TRIGGERED CAUSE SUCCESS: ");
        Serial.println(spellBook[i].spellName);

        // 1. PHYSICAL ACTION MODIFIERS
        if (cmd == 'l') { digitalWrite(UV_LED_PIN, HIGH); }
        else if (cmd == 'n') { digitalWrite(UV_LED_PIN, LOW); }
        else if (cmd == 'a') { analogWrite(RGB_RED_PIN, 255); analogWrite(RGB_GREEN_PIN, 0); analogWrite(RGB_BLUE_PIN, 0); }
        else if (cmd == 'b') { analogWrite(RGB_RED_PIN, 0); analogWrite(RGB_GREEN_PIN, 0); analogWrite(RGB_BLUE_PIN, 255); }
        else if (cmd == 'c') { analogWrite(RGB_RED_PIN, 0); analogWrite(RGB_GREEN_PIN, 255); analogWrite(RGB_BLUE_PIN, 0); }
        else if (cmd == 'x') { analogWrite(RGB_RED_PIN, 0); analogWrite(RGB_GREEN_PIN, 0); analogWrite(RGB_BLUE_PIN, 0); }

        // 2. DISPATCH OVER WIRELESS PACKET ROUTER
        wandPacket.dataChar = cmd;
        esp_now_send(receiverMAC, (uint8_t *) &wandPacket, sizeof(wandPacket));
        
        Serial.print("🚀 Broadcasting packet command '");
        Serial.print(cmd);
        Serial.println("' to room modules.");
        break;
      }
    }

    if (!matchFound && detectedSyllables > 0) {
      Serial.println("❌ Match Failed: Audio signature did not align with spellbook metrics.");
    }

    Serial.println("Release button to clear...");
    while(digitalRead(BUTTON_PIN) == LOW) { delay(10); }
    Serial.println("Wand clear. Ready for next casting cycle.\n");
  }
  delay(10);
}
