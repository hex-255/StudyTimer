// main.cpp file

#include "Menu.h"
#include "StudyMode.h" 
#include "defines.h"
#include "BreakMode.h"
#include "Config_Menus.h"
#include "StartUp.h"
#include "BreakOverflow.h"
#include "FuelGauge.h"

#include "driver/rtc_io.h"

#include <Preferences.h>

Preferences preferences;



// Define globals here (only once)
CRGB leds[NUM_LEDS];
CRGB status_leds[NUM_LEDS_STATUS];
Adafruit_seesaw ss;
int32_t encoder_position;

bool magnet_detected = false;
bool audio_enabled = false;
bool right_click = false;
bool left_click = false;

bool Main_Menu_debug = true;
bool Study_Mode_debug = true;
bool Break_Mode_debug = true;
bool Config_Menus_debug = true;
bool Startup_debug = true;
bool Magnet_debug = false;
bool FuelGauge_debug = true;
bool I2C_debug = true;
bool NVM_debug = true;

bool strict_mode = false;

bool streak_enabled = false;

int streak_counter = 0; 

const int TONE_PIN = D10;
const int TONE_CHANNEL = 0;   // LEDC channel 0..15
const int TONE_FREQ = 500;    // default start frequency
const int TONE_RES = 8;       // 8-bit resolution



// ... existing ...

int time_per_block_break = 60;  // Adjust as needed
int break_interval = 0;
int current_led_index = 0;
unsigned long break_start_block_time = 0;
bool break_started = false;

bool breakStartupPlayed = false;

bool enable_status_LED = true;
bool battery_is_high = true;
bool audio_on = false;

bool check_FixedStudy = false;
int FixedStudy_Index = 9;
unsigned long config_blink_start = 0;
bool mode_config_entered = false;

// ... existing colorOptions, etc. ...

int32_t encoder_offset = 0;
int32_t last_position = 0;
unsigned long encoder_last_activity = 0;

bool FuelGauge_available = false;  // Will be set to true if initialization succeeds

// Button state tracking
bool buttonStates[5] = {true, true, true, true, true};  // Assume HIGH initially (INPUT_PULLUP)

// Color options (excluding white and green)
const CRGB colorOptions[] = {
    CRGB::Red,
    CRGB::Orange,
    CRGB::Aqua,
    CRGB::Gold,      // Amber
    CRGB::Purple,
    CRGB::Teal,
    CRGB::Ivory,
    CRGB::Yellow,
    CRGB::Cyan,
    CRGB::Maroon,
    CRGB::Blue,
    CRGB::Sienna,
    CRGB::Magenta
};

const char* colorOptions_strings[] = {
    "Red",
    "Orange",
    "Aqua",
    "Gold",
    "Purple",
    "Teal",
    "Ivory",
    "Yellow",
    "Cyan",
    "Maroon",
    "Blue",
    "Sienna",
    "Magenta"
};



const int numColors = sizeof(colorOptions) / sizeof(colorOptions[0]);
int currentColorIndex = 0;  // Track which color is active

AppState_t currentState = STATE_STARTUP;

int brightness_limit = 20;  // brightness -> will be loaded from NVM 

// ... existing ...

int time_per_block = 150;  // Adjust as needed
int study_counter = 0;
int num_leds_lit = 0;
unsigned long start_block_time = 0;
bool study_started = false;

// Save controller pointers for independent updates
CLEDController* ringController;
CLEDController* statusController;

void readPCA9536(void);

void handleStatusLEDs();

void handle_Sleep();

void handle_NVM();


void setup() {

    esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, 0);  //1 = High, 0 = Low

    delay(1000);  // Short delay before starting animation

    Serial.begin(115200);
    Wire.begin();  // Initialize I2C for Seesaw

    //pinMode(D6, INPUT_PULLUP);  // D6 on Xiao (GPIO7)
    pinMode(D6, OUTPUT); // Load Switch output pin 
    pinMode(D0, INPUT);  // ohne PULLUP kein Sleep 
    pinMode(D10, OUTPUT); // Audio Signal 
    pinMode(D9, INPUT_PULLUP);  // D9 on Xiao (GPIO8) - for magnet detection

    digitalWrite(D6, HIGH); // Ensure load switch is on
    //digitalWrite(D6, LOW); // Ensure load switch is off

    Serial.println("Hallo ich bin im Setup!!");

    if (!ss.begin(0x49)) {
        Serial.println("Seesaw not found");
    }

    ss.pinMode(CENTER_BUTTON, INPUT_PULLUP);   
    ss.pinMode(RIGHT_BUTTON, INPUT_PULLUP);   
    ss.pinMode(LEFT_BUTTON, INPUT_PULLUP);   
    ss.pinMode(UP_BUTTON, INPUT_PULLUP);   
    ss.pinMode(DOWN_BUTTON, INPUT_PULLUP);   

    encoder_position = ss.getEncoderPosition();
    last_position = 0;
    encoder_last_activity = millis();

    ringController = &FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    statusController = &FastLED.addLeds<WS2812, LED_PIN_STATUS, GRB>(status_leds, NUM_LEDS_STATUS);
    FastLED.clear();

    
    handle_NVM(); // Load settings from non-volatile storage

    ledcSetup(TONE_CHANNEL, TONE_FREQ, TONE_RES);
    ledcAttachPin(TONE_PIN, TONE_CHANNEL);
    ledcWrite(TONE_CHANNEL, 0);  // silent at start

    if(initFuelGauge()){
        setFuelGaugeCapacity(1100);  // Set your battery capacity in mAh
        setTerminateVoltage(3200);  // Set your battery's cutoff voltage in mV
        FuelGauge_available = true;
    }

}

void loop(void) {

    // if D0 is pulled high through pull up -> go to sleep 
    if(digitalRead(D0) == HIGH){
      handle_Sleep();  
    }

    switch (currentState) {
        case STATE_STARTUP:
            handleStartup();
            break;

        case STATE_MAIN_MENU:
            handleMainMenu();
            break;

        case STATE_BRIGHTNESS_CONFIG:
            handleBrightnessConfig();
            break;

        case STATE_MODE_CONFIG:
            handleModeConfig();
            break;

        case STATE_STUDY_MODE:
            handleStudyMode();
            break;

        case STATE_BREAK_MODE:
            handleBreakMode();
            break;

        case STATE_BREAK_OVERFLOW:
            handleBreak_Overflow();
            break;
    }

    readPCA9536(); // check PCA9536 for magnet status and button press

    handleStatusLEDs(); // Update status LEDs based on current conditions

    delay(10);
}

void handle_Sleep() {
    // Turn off the LED ring before sleeping
    FastLED.clear();
    FastLED.show();

    // Turn off the load switch to cut power to peripherals
    digitalWrite(D6, LOW); // Ensure load switch is off

    Serial.println("In sleep condition - preparing to sleep");

    // Also turn off the status strip if you want
    fill_solid(status_leds, NUM_LEDS_STATUS, CRGB::Black);
    FastLED.show();
    Serial.println("delaying before sleep...");
    Serial.println("Going to sleep now");
    delay(5000);
    Serial.flush();  // Ensure all serial output is sent before sleeping
    esp_deep_sleep_start();
}

void handle_NVM(){

    preferences.begin("StudyTimer", false);
    
    brightness_bar_count = preferences.getInt("br_bar", brightness_bar_count);
    if(NVM_debug){
        Serial.print("Loaded brightness_bar_count from NVM: "); 
        Serial.println(brightness_bar_count);
    }
    if (brightness_bar_count < BRIGHTNESS_BAR_MIN || brightness_bar_count > BRIGHTNESS_BAR_MAX) {
        brightness_bar_count = 1;
    }
    brightness_limit = brightnessLevels[brightness_bar_count - 1];
    FastLED.setBrightness(brightness_limit);

    check_FixedStudy = preferences.getInt("mode_fix", check_FixedStudy);
    FixedStudy_Index = preferences.getInt("fix_index", FixedStudy_Index);
    if(NVM_debug){
        Serial.print("Loaded mode_fix (check_FixedStudy) from NVM: "); 
        Serial.println(check_FixedStudy);
        Serial.print("Loaded fix_index (FixedStudy_Index) from NVM: "); 
        Serial.println(FixedStudy_Index);
    }

}

void handleStatusLEDs() {

    // Status LED at index 2: audio status (orange if enabled, blue if disabled)
    if(enable_status_LED){
        CRGB color = audio_enabled ? CRGB::Orange : CRGB::Blue;
        //color.nscale8_video(40); // Scale down brightness for status LED
        status_leds[2] = color;
    } else {
        status_leds[2] = CRGB::Black;  // Keep it off if status LED is disabled
    }

    if(enable_status_LED){
        if(FuelGauge_available){
            int SoC = getFuelGaugeSOC();
            if(SoC > 60){
                CRGB battery_color = CRGB::Green;
                status_leds[0] = battery_color;
            }
            else if(SoC <= 60 && SoC > 20){
                CRGB battery_color = CRGB::Orange;
                status_leds[0] = battery_color;
            }
            else{
                CRGB battery_color = CRGB::Red;
                status_leds[0] = battery_color;
            }
        }
    } 
    else {
        status_leds[0] = CRGB::Black;  // Keep it off if status LED is disabled
    }


    FastLED.show();
}

void readPCA9536(void) {
    // Set IO0 as input
    Wire.beginTransmission(0x41);
    Wire.write(0x03); // Configuration register
    Wire.write(0x0F); // Set all bits to 1 (all inputs)
    Wire.endTransmission();

    // Read input port
    Wire.beginTransmission(0x41);
    Wire.write(0x00); // Input port register
    Wire.endTransmission(false);
    Wire.requestFrom(0x41, 1);

    if (Wire.available()) {
        byte input = Wire.read();
        magnet_detected = not(input & 0x01);  // Bit 0 = 0 means magnet detected (active low)
        audio_enabled = not(input & 0x02);  // Bit 1 = 0 means audio enabled (active low)
        right_click = not(input & 0x04);  // Bit 2 = 0 means right click detected (active low)
        left_click = not(input & 0x08);  // Bit 3 = 0 means left click detected (active low)
        if (Magnet_debug) {
            Serial.print("PCA9536 IO0: ");
            Serial.println(magnet_detected ? "magnet detected" : "no magnet");
            Serial.print("PCA9536 IO1: ");
            Serial.println(audio_enabled ? "audio enabled" : "audio disabled");
            Serial.print("PCA9536 IO2: ");
            Serial.println(right_click ? "right click detected" : "no right click");
            Serial.print("PCA9536 IO3: ");
            Serial.println(left_click ? "left click detected" : "no left click");
        }
    } else {
        if (Magnet_debug) {
            Serial.println("Failed to read from PCA9536");
        }
    }
}