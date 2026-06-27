// StartUp.h file

#include "defines.h"
#include "FuelGauge.h"

void StreakLED_Test(void);

void battery_SoC_animation(int SoC);

void startUpAnimation(void);

void handleStartup(void) {

    if(Startup_debug) {
        Serial.println("Starting Startup Animation");
    }

    if(FuelGauge_available){
        battery_SoC_animation(getFuelGaugeSOC());
    }

    startUpAnimation();

    //StreakLED_Test();

    // done: go to menu
    currentState = STATE_MAIN_MENU;
}

void StreakLED_Test(void) {
    // Light up the first 4 LEDs (P0-P3) on PCF8574 at address 0x38
    // Outputs are sinks: LOW = LED on, HIGH = LED off
    // Byte: 0b11110000 (P0-P3 = 0/LOW, P4-P7 = 1/HIGH)
    Wire.beginTransmission(0x38);
    Wire.write(0b11111111);  // 0xF0 in hex
    Wire.endTransmission();
    
    delay(2000);  // Hold the LEDs on for 2 seconds (adjust as needed)
    
    // Turn off all LEDs (set all outputs HIGH)
    //Wire.beginTransmission(0x38);
    //Wire.write(0xFF);  // All bits HIGH
    //Wire.endTransmission();
    
}

void battery_SoC_animation(int SoC) {
    SoC = constrain(SoC, 0, 100);
    int ledsToLight = map(SoC, 0, 100, 0, NUM_LEDS);

    CRGB color = CRGB::Red;
    if (ledsToLight > 15) {
        color = CRGB::Green;
    } else if (ledsToLight > 5) {
        color = CRGB::Orange;
    }

    FastLED.clear();
    FastLED.show();

    for (int i = 0; i < ledsToLight; i++) {
        leds[i] = color;
        FastLED.show();
        delay(40);
    }

    delay(1000);

    for (int i = (ledsToLight-1); i >= 0; i--) {
        leds[i] = CRGB::Black;
        FastLED.show();
        delay(20);
    }

    delay(200);
}

void startUpAnimation(){
    const CRGB color = CRGB::Green;
    const int walkers = 3;
    const int rotations = 2;
    const int baseDelayMs = 80;      // starting delay (slower)
    const int minDelayMs  = 12;      // fastest delay
    const int totalSteps  = rotations * NUM_LEDS;

    // Start positions adjacent (0,1,2)
    int positions[walkers];
    for (int w = 0; w < walkers; w++) {
        positions[w] = w;
    }

    // --- 1) wandering walkers with increasing speed ---
    for (int step = 0; step < totalSteps; step++) {
        FastLED.clear();
        for (int w = 0; w < walkers; w++) {
            int p = (positions[w] + step) % NUM_LEDS;
            leds[p] = color;
        }
        FastLED.show();

        // speed up over time
        int delayMs = baseDelayMs - ((baseDelayMs - minDelayMs) * step) / totalSteps;
        delay(delayMs);
    }

    // --- 2) fill ring one LED at a time ---
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
        FastLED.show();
        delay(30);
    }

    delay(500);  // Hold full ring for a moment

    // --- 3) turn off one LED at a time (from 0..NUM_LEDS-1) ---
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
        FastLED.show();
        delay(30);
    }
}