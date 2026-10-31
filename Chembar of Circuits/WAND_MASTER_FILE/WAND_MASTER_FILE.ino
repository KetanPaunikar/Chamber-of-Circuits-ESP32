#include <esp_now.h>
#include <WiFi.h>
#include <driver/i2s_std.h>

// REPLACE WITH YOUR RECEIVER'S MAC ADDRESS
uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// =========================================================================
// EASY CONFIGURATION ZONE: Adjust your timeouts and settings here!
// =========================================================================
const unsigned long UV_TIMEOUT_MS = 10000;  // 10 seconds (Change this value to adjust timer)

// HARDWARE PIN ASSIGNMENTS & PWM CHANNELS (From schematic)
const int BUTTON_PIN   = 23;  // Physical push button (Active LOW)
const int UV_LED_PIN   = 10;  // UV LED Cathode pin
const int RGB_RED_PIN  = 22;  // RGB Red Cathode pin
const int RGB_BLUE_PIN = 20;  // RGB Blue Cathode pin
const int RGB_GRN_PIN  = 21;  // RGB Green Cathode pin

// INMP441 Microphone Pins
const int I2S_WS       = 2;   
const int I2S_SCK      = 3;   
const int I2S_SD       = 1;   

// =========================================================================
// GLOBAL STATE TRACKING VARIABLES
// =========================================================================
unsigned long uvStartTime = 0;
bool isUvActive = false;

// ESP-NOW Data structure
typedef struct struct_message {
    char spellCode; 
} struct_message;

struct_message myData;
i2s_chan_handle_t rx_handle = NULL;

struct SpellMapping {
    const char* spellName;
    const char* onCode;
    const char* offCode;
};

SpellMapping spellBook[] = {
  {"solis illumination",   "A1",     "A0"},
  {"ignis hearth",         "B1",     "B0"},
  {"vapors mist",          "C1",     "C0"},
  {"tonitrus red",         "D1",     "D0"}, 
  {"tonitrus blue",        "D2",     "D0"}, 
  {"tonitrus green",       "D3",     "D0"}, 
  {"tonitrus white",       "D4",     "D0"}, 
  {"revelio ink",          "E1",     "E0"}, // Local UV Glow
  {"herbivicus grow",      "F1",     "F0"},
  {"portal mirror",        "G1",     "G0"},
  {"dispatch box",         "H1",     "H0"},
  {"whirlpool stir",       "J1",     "J0"}, // Magic Stirring Mug
  {"lumos red",            "I1",     "I0"}, // Local Red Glow
  {"lumos blue",           "I2",     "I0"}, // Local Blue Glow
  {"lumos green",          "I3",     "I0"}, // Local Green Glow
  {"nox",                  "I0",     "I0"}  // Turns off ALL local LEDs
};

const int totalSpells = sizeof(spellBook) / sizeof(spellBook[0]);

// Helper function to handle inverted Common Anode PWM (0 = Full Bright, 255 = Off)
void setRGB_PWM(int r_val, int b_val, int g_val) {
  ledcWrite(RGB_RED_PIN, 255 - r_val);
  ledcWrite(RGB_BLUE_PIN, 255 - b_val);
  ledcWrite(RGB_GRN_PIN, 255 - g_val);
}

// Helper function to handle inverted Common Anode UV PWM (0 = Full Bright, 255 = Off)
void setUV_PWM(int uv_val) {
  ledcWrite(UV_LED_PIN, 255 - uv_val);
}

// Smooth magic blue breathing pulse confirmation for general network spells
void pulseBlueIndicator() {
  for (int duty = 0; duty <= 150; duty += 5) { 
    setRGB_PWM(0, duty, 0);
    delay(15);
  }
  for (int duty = 150; duty >= 0; duty -= 5) {
    setRGB_PWM(0, duty, 0);
    delay(15);
  }
  setRGB_PWM(0, 0, 0); 
}

// Smooth fading transitions for turning the UV LED ON or OFF
void fadeUV(bool turnOn) {
  if (turnOn) {
    for (int duty = 0; duty <= 255; duty += 5) {
      setUV_PWM(duty);
      delay(10);
    }
  } else {
    for (int duty = 255; duty >= 0; duty -= 5) {
      setUV_PWM(duty);
      delay(10);
    }
    setUV_PWM(0); // Ensure complete shutoff
  }
}

// Fade out everything smoothly (Called when 'nox' is executed)
void fadeOutAllLEDs() {
  // Read current active levels to ramp down uniformly
  for (int level = 255; level >= 0; level -= 5) {
    // If UV was active, pull it down smoothly alongside any potential active RGB legs
    ledcWrite(RGB_RED_PIN, 255 - (level > 0 ? level : 0));
    ledcWrite(RGB_BLUE_PIN, 255 - (level > 0 ? level : 0));
    ledcWrite(RGB_GRN_PIN, 255 - (level > 0 ? level : 0));
    ledcWrite(UV_LED_PIN, 255 - (level > 0 ? level : 0));
    delay(8);
  }
  setRGB_PWM(0, 0, 0);
  setUV_PWM(0);
}

void initI2SMic() {
  i2s_chan_config_t chan_cfg = I2S_CHAN_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
  i2s_new_channel(&chan_cfg, NULL, &rx_handle);

  i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
    .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
    .gpio_cfg = { .mclk = I2S_GPIO_UNUSED, .bclk = (gpio_num_t)I2S_SCK, .ws = (gpio_num_t)I2S_WS, .dout = I2S_GPIO_UNUSED, .din = (gpio_num_t)I2S_SD }
  };
  std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
  i2s_channel_init_std_rx(rx_handle, &std_cfg);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Set up ESP32 PWM channels for ALL emitters to allow fading animations
  ledcAttach(RGB_RED_PIN, 5000, 8);  // 5kHz frequency, 8-bit resolution
  ledcAttach(RGB_BLUE_PIN, 5000, 8);
  ledcAttach(RGB_GRN_PIN, 5000, 8);
  ledcAttach(UV_LED_PIN, 5000, 8);
  
  // Start completely dark (Common Anode 255-0 = 255 out to pin, turning them OFF)
  setRGB_PWM(0, 0, 0); 
  setUV_PWM(0);

  initI2SMic();

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) return;

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void loop() {
  // 1. AUTOMATIC UV SAFETY TIMEOUT WATCHDOG RUNNING CONTINUOUSLY
  if (isUvActive && (millis() - uvStartTime >= UV_TIMEOUT_MS)) {
    Serial.println("[Safety Timeout] Fading out UV LED to preserve LiPo battery...");
    fadeUV(false); // Smoothly fade the UV LED to off
    isUvActive = false;
  }

  // 2. CHECK FOR PUSH BUTTON TO COMMENCE VOICE SAMPLING
  if (digitalRead(BUTTON_PIN) == LOW) {
    i2s_channel_enable(rx_handle);
    setRGB_PWM(0, 30, 0); // Dim blue listener indicator

    static int32_t i2s_read_buff;
    size_t bytes_read = 0;
    while(digitalRead(BUTTON_PIN) == LOW) {
      i2s_channel_read(rx_handle, &i2s_read_buff, sizeof(i2s_read_buff), &bytes_read, 100);
    }
    
    i2s_channel_disable(rx_handle);
    setRGB_PWM(0, 0, 0);

    // Simulation placeholder text voice injection:
    String recognizedVoicePhrase = "revelio ink"; 
    
    executeVoiceSpell(recognizedVoicePhrase);
  }
}

void executeVoiceSpell(String voiceCommand) {
  bool spellFound = false;

  for (int i = 0; i < totalSpells; i++) {
    if (voiceCommand.equalsIgnoreCase(spellBook[i].spellName)) {
      spellFound = true;
      
      strcpy(myData.spellCode, spellBook[i].onCode);
      esp_now_send(receiverAddress, (uint8_t *) &myData, sizeof(myData));

      // Cut off any existing UV state immediately if switching to a new spell
      if (isUvActive && !voiceCommand.equalsIgnoreCase("revelio ink")) {
        setUV_PWM(0);
        isUvActive = false;
      }

      // -------------------------------------------------------------------------
      // LOCAL PERSISTENT LIGHT SPELLS
      // -------------------------------------------------------------------------
      if (voiceCommand.equalsIgnoreCase("lumos red") || voiceCommand.equalsIgnoreCase("tonitrus red")) {
        setRGB_PWM(255, 0, 0); 
      } 
      else if (voiceCommand.equalsIgnoreCase("lumos blue") || voiceCommand.equalsIgnoreCase("tonitrus blue")) {
        setRGB_PWM(0, 255, 0); 
      } 
      else if (voiceCommand.equalsIgnoreCase("lumos green") || voiceCommand.equalsIgnoreCase("tonitrus green")) {
        setRGB_PWM(0, 0, 255); 
      } 
      else if (voiceCommand.equalsIgnoreCase("revelio ink")) {
        setRGB_PWM(0, 0, 0); // Turn off RGB elements
        fadeUV(true);        // Smoothly glow the UV LED on
        uvStartTime = millis(); // Benchmark system time for automatic fading cut-off
        isUvActive = true;
      }
      // -------------------------------------------------------------------------
      // SYSTEM SHUTDOWN SPELL (NOX)
      // -------------------------------------------------------------------------
      else if (voiceCommand.equalsIgnoreCase("nox")) {
        fadeOutAllLEDs(); // Smoothly dims down both UV and RGB legs at once
        isUvActive = false;
      }
      // -------------------------------------------------------------------------
      // GENERIC SPELL BRANCH (Smooth blue pulse confirmation for things like the coffee mug)
      // -------------------------------------------------------------------------
      else {
        pulseBlueIndicator();
      }
      break;
    }
  }
}
