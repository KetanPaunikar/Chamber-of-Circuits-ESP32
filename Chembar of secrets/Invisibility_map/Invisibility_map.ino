#include <driver/i2s.h> // Native ESP32 framework to pull digital data from the I2S microphone

// ====================================================================
// STUDENT CONFIGURATION ZONE (VARIABLES ON TOP FOR EASY UPDATES)
// ====================================================================
// --- HARDWARE WIRING PINS (Matches your precise wand layout) ---
const int BUTTON_PIN = 23;      // Tactile casting button pin
const int UV_LED_PIN = 21;      // Ultraviolet (UV) LED pin (Purple dot wire)

// INMP441 Digital Microphone I2S Pin Hardware Configuration
const int I2S_WS_PIN = 02;      // Word Select line
const int I2S_SCK_PIN = 03;     // Serial Clock line
const int I2S_SD_PIN = 22;      // Serial Data line (Blue wire)

// --- MICROPHONE SIGNAL PROCESSING FILTERS ---
const int32_t AUDIO_THRESHOLD = 45000; // Volume filter intensity (Increase if background noise interferes)
const int INTEGRATION_WINDOW = 30;     // Minimum sound length filter to block random ambient clicks
const int SILENCE_TIMEOUT = 180;       // Quiet duration (ms) required to separate distinct syllables

// --- EBOOK LOCAL UV SPELLBOOK REGISTRY ---
// EXPLANATION FOR STUDENTS: 
// The last variable in each row dictates the physical behavior of the UV LED.
// - true  = Turn the local UV LED ON
// - false = Turn the local UV LED OFF
struct SpellPattern {
  String spellName;
  int expectedPulses; 
  bool targetState;   
};

SpellPattern spellBook[] = {
  { "Nox",   1, false },  // 1 Spoken Beat (Nox!) -> Turns the UV LED completely OFF
  { "Lumos", 2, true  }   // 2 Spoken Beats (Lu-mos!) -> Turns the UV LED completely ON
};

// ====================================================================
// CORE PERIPHERAL & OPERATIONAL PIPELINES
// ====================================================================
const int TOTAL_SPELLS = sizeof(spellBook) / sizeof(spellBook);
#define DMA_BUFFER_SIZE 256
int32_t audioBuffer[DMA_BUFFER_SIZE];

// Configures the low-level I2S hardware registers to talk directly to the INMP441 chip
void initMicrophone() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,                         // 16kHz standard vocal recording rate
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,    // INMP441 outputs 32-bit high-resolution audio words
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,     // Mono microphone channel profile
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = DMA_BUFFER_SIZE,                   // Shallow buffer for instant real-time envelope tracking
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK_PIN,
    .ws_io_num = I2S_WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD_PIN
  };

  // Bind the hardware driver configurations into the microcontroller architecture
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

// ====================================================================
// SETUP CONFIGURATION
// ====================================================================
void setup() {
  Serial.begin(115200);

  // Initialize the UV LED pin as a standard power output rail
  pinMode(UV_LED_PIN, OUTPUT);
  digitalWrite(UV_LED_PIN, LOW); // Start with the UV light safely turned OFF

  // Configure casting trigger switch using internal pullup resistor stabilization
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Wake up the direct memory access microphone communication line
  initMicrophone();

  Serial.println("\n=================================================");
  Serial.println("🧙‍♂️ EBOOK CHAPTER: LOCAL UV SPELL CASTING ONLINE");
  Serial.println("=================================================");
}

// ====================================================================
// MAIN OPERATIONAL ANALYSIS LOOP
// ====================================================================
void loop() {
  // Check if the student is pressing down the tactile wand switch
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("\n🎤 Wand actively listening... Speak your spell clearly!");
    
    // Quick hardware flicker on the UV line to confirm recording has begun
    digitalWrite(UV_LED_PIN, HIGH); delay(60); digitalWrite(UV_LED_PIN, LOW);

    int detectedSyllables = 0;
    bool speechOngoing = false;
    int voiceSustainCounter = 0;
    unsigned long lastVoiceTime = 0;
    unsigned long recordingWindow = millis();

    // Loop continuously for a tight 2-second capturing window while button remains pressed
    while (millis() - recordingWindow < 2000) {
      size_t bytesRead = 0;
      
      // Pull raw audio blocks straight out of the microphone hardware memory buffers
      i2s_read(I2S_NUM_0, &audioBuffer, sizeof(audioBuffer), &bytesRead, portMAX_DELAY);
      int samplesCount = bytesRead / 4;

      if (samplesCount > 0) {
        // Average out the absolute electrical heights of this audio window snippet
        long long sumAmplitude = 0;
        for (int i = 0; i < samplesCount; i++) {
          sumAmplitude += abs(audioBuffer[i]);
        }
        int32_t averageVolume = sumAmplitude / samplesCount;

        // --- ENERGY WINDOW INTEGRATION FILTER ---
        if (averageVolume > AUDIO_THRESHOLD) {
          lastVoiceTime = millis(); // User is making noise, push back the silence timeout clock
          
          if (!speechOngoing) {
            voiceSustainCounter++;
            // Confirm as a true syllable beat only if it sustains long enough (avoids pops or background static)
            if (voiceSustainCounter > INTEGRATION_WINDOW) {
              detectedSyllables++;
              speechOngoing = true;
              Serial.print("   🔊 Spoken Beat Tracked! Syllable Count = ");
              Serial.println(detectedSyllables);
            }
          }
        } else {
          // If the audio signal drops below threshold, dump current sustain progress trackers
          if (!speechOngoing) {
            voiceSustainCounter = 0;
          }
          
          // --- TIME-LATCH COOLDOWN GATE ---
          // Clear the speech tracking gate only if total silence stays continuous longer than the timeout gap
          if (speechOngoing && (millis() - lastVoiceTime > SILENCE_TIMEOUT)) {
            speechOngoing = false;
            voiceSustainCounter = 0;
          }
        }
      }
    }

    Serial.println("⏹️ Audio sampling timeframe finalized.");
    Serial.print("-> Evaluated Total Spoken Footprint: ");
    Serial.println(detectedSyllables);

    // --- LOCAL REGISTRY MATCHING SEARCH ---
    bool spellFound = false;
    for (int i = 0; i < TOTAL_SPELLS; i++) {
      if (detectedSyllables == spellBook[i].expectedPulses) {
        bool targetState = spellBook[i].targetState;
        spellFound = true;

        Serial.print("✨ SPELLBOOK MATCH FOUND: ");
        Serial.println(spellBook[i].spellName);

        // Physically switch the local UV LED state based on the rule configuration row
        if (targetState == true) {
          digitalWrite(UV_LED_PIN, HIGH);
          Serial.println("Action: Wand tip illuminated (UV ON)!");
        } else {
          digitalWrite(UV_LED_PIN, LOW);
          Serial.println("Action: Wand tip extinguished (UV OFF)!");
        }
        break; // Stop running the search mapping loop
      }
    }

    if (!spellFound && detectedSyllables > 0) {
      Serial.println("❌ Match Failed: Audio signature did not align with a registered UV spell.");
    }

    // Safety lockout loop: Halt logic step updates until student physically lets go of the button
    Serial.println("Release casting switch to clear wand system...");
    while(digitalRead(BUTTON_PIN) == LOW) { 
      delay(10); 
    }
    Serial.println("Wand clear. Ready for next incantation.\n");
  }
  delay(10); // System tick scheduling pause
}
