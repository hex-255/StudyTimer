// defines.h file

#ifndef DEFINES_H
#define DEFINES_H

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_seesaw.h"
#include <FastLED.h>
#include "esp_sleep.h"
#include "SparkFunBQ27441.h"

// for saving to non-volatile storage
#include <Preferences.h>

#define LED_PIN 4
#define NUM_LEDS 24

#define PCF8574_I2C_ADDRESS 0x38

#define LED_PIN_STATUS 3
#define NUM_LEDS_STATUS 4

#define WAKEUP_GPIO GPIO_NUM_1    // Only RTC IO are allowed - ESP32 Pin example -> D0 Pin 

// for saving to non-volatile storage
extern Preferences preferences;

extern CRGB status_leds[];

// Declare globals (definitions will be in main.cpp)
extern CRGB leds[];
extern Adafruit_seesaw ss;
extern int32_t encoder_position;

extern bool magnet_detected;
extern bool audio_enabled;
extern bool right_click;
extern bool left_click;
extern int streak_counter; 

extern bool streak_enabled;

extern bool Main_Menu_debug;
extern bool Study_Mode_debug;
extern bool Break_Mode_debug;
extern bool Config_Menus_debug;
extern bool Startup_debug;
extern bool Magnet_debug;
extern bool FuelGauge_debug;
extern bool I2C_debug;
extern bool NVM_debug;
extern bool FuelGauge_available;

extern bool strict_mode;

// Global BQ27441 instance with default address 0x55
extern BQ27441 lipo;

// ... existing code ...

extern int brightness_limit;  // Max brightness (0-255)
const int fade_steps = 40;         // Number of fade steps
const int fade_delay_ms = 10;      // Delay between steps (20ms ≈ 0.02s)

extern bool breakStartupPlayed;

extern int brightness_bar_count;

extern const int brightnessLevels[];
extern const int brightnessLevelsCount;

extern bool enable_status_LED;
extern bool battery_is_high;
extern bool audio_on;

extern const int TONE_PIN;
extern const int TONE_CHANNEL;
extern const int TONE_FREQ;
extern const int TONE_RES;

#define BRIGHTNESS_BAR_MIN 1
#define BRIGHTNESS_BAR_MAX 5

extern bool check_FixedStudy;
extern int FixedStudy_Index;
extern unsigned long config_blink_start;
const float config_blink_period = 1.0;  // seconds
extern bool mode_config_entered;

// ... existing code ...

// ... existing code ...

// Study mode parameters
extern int time_per_block;  // Seconds per block (e.g., 150)
extern int study_counter;
extern int num_leds_lit;
extern unsigned long start_block_time;
extern bool study_started;

// Animation durations (seconds)
const float fade_up_duration = 1.0;
const float fade_down_duration = 0.5;
const float off_duration = 1.0;
const float total_anim_duration = fade_up_duration + fade_down_duration + off_duration;

// Fade steps for full reset
const int full_fade_steps = 20;
const int full_fade_delay_ms = 20;

extern int32_t encoder_offset;
extern int32_t last_position;
extern unsigned long encoder_last_activity;
const unsigned long ENCODER_IDLE_TIMEOUT = 10000;  // 10 seconds in ms

// ... existing code ...

// Break mode parameters
extern int time_per_block_break;  // Seconds per break block (e.g., 60)
extern int break_interval;
extern int current_led_index;
extern unsigned long break_start_block_time;
extern bool break_started;

// Break animation durations (seconds)
const float break_fade_down_duration = 1.0;
const float break_fade_up_duration = 0.5;
const float break_stay_on_duration = 1.0;
const float break_total_anim_duration = break_fade_down_duration + break_fade_up_duration + break_stay_on_duration;

// Add button state tracking for pressed/released detection (optional, for accuracy)
extern bool buttonStates[5];  // Track previous states for 5 buttons

const int CENTER_BUTTON = 5;
const int RIGHT_BUTTON = 3;
const int LEFT_BUTTON = 11;
const int UP_BUTTON = 0;
const int DOWN_BUTTON = 4;

// Color options (excluding white and green)
extern const CRGB colorOptions[];
extern const int numColors;
extern int currentColorIndex;  // Track which color is active

typedef enum {
    STATE_STARTUP,
    STATE_MAIN_MENU,
    STATE_BRIGHTNESS_CONFIG,
    STATE_MODE_CONFIG,
    STATE_STUDY_MODE,
    STATE_BREAK_MODE,
    STATE_BREAK_OVERFLOW,
} AppState_t;

extern AppState_t currentState;

#endif  // DEFINES_H