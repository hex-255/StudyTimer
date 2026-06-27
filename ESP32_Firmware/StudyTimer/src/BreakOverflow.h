// BreakOverflow.h file

#include "defines.h"

void moving_ring_animation();
void controlPCF8574_LEDs(uint8_t ledMask);

void streakCount_Mapping(int streak_counter);

void handleBreak_Overflow(){
    
    // Run startup once (but without animation)
    if (!break_started) {
        if (Break_Mode_debug) {
            Serial.println("Entered Break Overflow Mode");
        }
        break_started = true;
        break_start_block_time = millis();
        current_led_index = NUM_LEDS - 1;  // Start from 23
        break_interval = 0;
    }

    // Check for exit via CENTER button - jump to study mode
    if (ss.digitalRead(CENTER_BUTTON) == LOW) {
        streak_counter += 1;  // Increment streak counter on break exit
        if (Break_Mode_debug) {
            Serial.println("CENTER button pressed - jumping to Study Mode from break_interval: " + String(break_interval));
            Serial.println("Current streak count: " + String(streak_counter));
        }
        delay(200);  // Debounce
        FastLED.clear();
        FastLED.show();

        // Map streak counter to PCF8574 LEDs 
        streakCount_Mapping(streak_counter);

        currentState = STATE_STUDY_MODE;
        break_started = false;  // Reset for next entry
        study_started = false;  // Reset study mode
        return;
    }

    unsigned long now = millis();
    float block_elapsed = (now - break_start_block_time) / 1000.0;  // Seconds

    // Calculate animation cycles
    float num_cycles = (float)time_per_block_break / total_anim_duration;
    int cycles_completed = (int)(block_elapsed / total_anim_duration);
    bool in_last_cycle = (cycles_completed >= (int)(num_cycles - 1));

    // Calculate animation phase
    float t = fmod(block_elapsed, total_anim_duration);
    float brightness = 0.0;

    if (in_last_cycle) {
        // Last cycle: fade up only, then stay solid
        brightness = min((float)brightness_limit, brightness_limit * t / fade_up_duration);
    } else {
        // Normal: fade up → fade down → off
        if (t < fade_up_duration) {
            brightness = brightness_limit * t / fade_up_duration;
        } else if (t < fade_up_duration + fade_down_duration) {
            brightness = brightness_limit - brightness_limit * ((t - fade_up_duration) / fade_down_duration);
        } else {
            brightness = 0.0;
        }
    }

    // Render LEDs (orange color, counting down from index 23)
    CRGB base_color = CRGB::Orange;

    // Previous LEDs: solid at brightness_limit (filled from 23 downward)
    for (int i = NUM_LEDS - 1; i > current_led_index; i--) {
        leds[i] = base_color;
    }

    // Current LED: animated brightness
    if (current_led_index >= 0) {
        leds[current_led_index] = CRGB(
            (int)(base_color.r * brightness / brightness_limit),
            (int)(base_color.g * brightness / brightness_limit),
            (int)(base_color.b * brightness / brightness_limit)
        );
    }

    FastLED.show();

    // Check if block time elapsed
    if (block_elapsed >= time_per_block_break) {
        // Lock in current LED at brightness_limit
        if (current_led_index >= 0) {
            leds[current_led_index] = base_color;
        }
        FastLED.show();

        current_led_index--;
        break_interval++;
        break_start_block_time = now;

        if (Break_Mode_debug) {
            Serial.println("break_overflow_count: " + String(break_interval));
        }
    }

        // Check if 5 blocks reached: animate moving LEDs
    if (break_interval >= 5) {
        if (Break_Mode_debug) {
            Serial.println("Break Overflow complete (5 blocks) - moving LED animation");
        }

        moving_ring_animation();

        FastLED.clear();
        FastLED.show();

        streak_counter = 0; // Reset streak counter on break overflow

        controlPCF8574_LEDs(0b11111111); // turn off all streak LEDs

        currentState = STATE_MAIN_MENU;
        break_started = false;  // Reset for next entry
        return;
    }

    if(right_click or left_click){
        if (Break_Mode_debug) {
            Serial.println("Break Overflow aborted -> to Main Menu");
        }

        moving_ring_animation();

        FastLED.clear();
        FastLED.show();

        streak_counter = 0; // Reset streak counter on break overflow

        controlPCF8574_LEDs(0b11111111); // turn off all streak LEDs

        currentState = STATE_MAIN_MENU;
        break_started = false;  // Reset for next entry
        return;
    }
}

// Function to control LEDs via PCF8574 I2C expander
// ledMask: 8-bit value where each bit controls one LED (1 = ON, 0 = OFF)
// Example: controlPCF8574_LEDs(0b11001010) controls LEDs on specific pins
void controlPCF8574_LEDs(uint8_t ledMask) {
    Wire.beginTransmission(PCF8574_I2C_ADDRESS);
    int status = Wire.write(ledMask);
    Wire.endTransmission();
    
    if (Break_Mode_debug || Startup_debug) {
        Serial.print("PCF8574 LED Control: 0x");
        Serial.println(ledMask, HEX);
    }
}

void streakCount_Mapping(int streak_counter){
        // P3 and P4 on for streak count at 1
        if(streak_counter == 1){
            controlPCF8574_LEDs(0b11100111);
        }
        // P2 and P5 turn on too if streak count at 2
        if(streak_counter == 2){
            controlPCF8574_LEDs(0b11000011);
        }
        // P1 and P6 turn on too if streak count at 3
        if(streak_counter == 3){
            controlPCF8574_LEDs(0b10000001);
        }
        // all LEDs on if streak count at 4 (max)
        if(streak_counter == 4){
            controlPCF8574_LEDs(0b00000000);
        }

        // if streak counter is zero 
        if(streak_counter == 0){
            controlPCF8574_LEDs(0b11111111);
        }
        
}

void moving_ring_animation(){
    // Animation: 5 LEDs moving in descending direction, gradually disappearing
    int start_position = 23;
    int total_steps = 40;  // Number of animation steps
    
    for (int step = 0; step < total_steps; step++) {
        FastLED.clear();
        
        // Calculate how many LEDs to show (starts at 5, decreases to 1)
        float progress = (float)step / total_steps;
        int num_moving_leds = max(1, (int)(5.0 * (1.0 - progress * 0.8)));
        
        // Calculate brightness fade
        float brightness_factor = 1.0 - (progress * 0.8);
        
        // Draw the moving LEDs in descending index direction
        for (int j = 0; j < num_moving_leds; j++) {
            int led_index = (start_position - step - j + NUM_LEDS * 3) % NUM_LEDS;
            
            CRGB faded = CRGB::Orange;
            faded.fadeToBlackBy(255 * (1.0 - brightness_factor));
            
            leds[led_index] = faded;
        }
        
        FastLED.show();
        delay(50);  // Animation frame delay
        
        // Stop when the single remaining LED reaches index 5
        if (num_moving_leds == 1) {
            int final_position = (start_position - step + NUM_LEDS * 3) % NUM_LEDS;
            if (final_position <= 5) break;
        }
    }
}