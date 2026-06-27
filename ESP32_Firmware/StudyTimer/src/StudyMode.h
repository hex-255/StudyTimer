// StudyMode.h file

#include "defines.h"

void Study_Startup_Animation();

void Streak_Counter_Startup_Animation(int streak_counter);

void handleStudyMode(void) {

    // Handle magnet detection with warning timer
    static unsigned long magnet_lost_time = 0;
    static bool magnet_loss_started = false;
    
    if(!magnet_detected && strict_mode) {
        // Start timer on first detection of magnet loss
        if (!magnet_loss_started) {
            magnet_lost_time = millis();
            magnet_loss_started = true;
            if (Study_Mode_debug) {
                Serial.println("Magnet lost - starting warning timer");
            }
        }
        
        unsigned long magnet_lost_elapsed = millis() - magnet_lost_time;
        
        // Update status LED based on time elapsed
        if (magnet_lost_elapsed < 5000) {
            // 0-5 seconds: no LED
            status_leds[1] = CRGB::Black;
        } else if (magnet_lost_elapsed < 15000) {
            // 5-15 seconds: yellow
            status_leds[1] = CRGB::Yellow;
        } else if (magnet_lost_elapsed < 20000) {
            // 15-20 seconds: orange
            status_leds[1] = CRGB::Orange;
        } else if (magnet_lost_elapsed < 21000) {
            // 20-21 seconds: red
            status_leds[1] = CRGB::Red;
        } else {
            // 21+ seconds: exit study mode
            if (Study_Mode_debug) {
                Serial.println("Magnet not detected for too long - exiting study mode");
            }
            FastLED.clear();
            FastLED.show();
            fill_solid(status_leds, NUM_LEDS_STATUS, CRGB::Black);
            FastLED.show();
            
            currentState = STATE_MAIN_MENU;
            study_started = false;
            magnet_loss_started = false;  // Reset for next time
            strict_mode = false;  // Reset strict mode
            return;
        }
        
        FastLED.show();
    }
    if(magnet_detected && strict_mode) {
        // Magnet detected - reset timer and turn status LED to Aqua
        magnet_loss_started = false;
        status_leds[1] = CRGB::Aqua;  // Magnet detected - set to Aqua
    }
    
    // Run startup animation once and reset streak enabled
    if (!study_started) {
        if (Study_Mode_debug) {
            Serial.println("Entered Study Mode");
        }
        if(streak_counter > 0){
            Streak_Counter_Startup_Animation(streak_counter);
            delay(300);  // Short pause after streak animation
        }
        Study_Startup_Animation();
        study_started = true;
        start_block_time = millis();
        //num_leds_lit = 0;
        //study_counter = 0;
        // Reset streak on new study session
        streak_enabled = false;
    }

    // Check for exit (CENTER button) or if fixed study reached index
    if ((ss.digitalRead(CENTER_BUTTON) == LOW && !check_FixedStudy)) {
        if (Study_Mode_debug) {
            Serial.println("study_counter at exit: " + String(study_counter));
        }
        delay(200);  // Debounce
        FastLED.clear();
        FastLED.show();
        currentState = STATE_BREAK_MODE;  
        study_started = false;  // Reset for next entry
        return;
    }

    // in fixed mode exit if index is reached 
    if((check_FixedStudy && study_counter == (FixedStudy_Index+1))){
        if (Study_Mode_debug) {
            Serial.println("break triggered by FixedStudy_Index");
        }
        delay(200);  // Debounce
        FastLED.clear();
        FastLED.show();
        currentState = STATE_BREAK_MODE;  
        study_started = false;  // Reset for next entry
        return;
    }

    unsigned long now = millis();
    float block_elapsed = (now - start_block_time) / 1000.0;  // Seconds

    // Calculate animation cycles
    float num_cycles = (float)time_per_block / total_anim_duration;
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

    // Render LEDs
    CRGB base_color = colorOptions[currentColorIndex];

    // Previous LEDs: solid at brightness_limit
    for (int i = 0; i < num_leds_lit; i++) {
        leds[i] = base_color;  // Global brightness caps to brightness_limit
    }

    // Current LED: animated brightness
    int current_led = num_leds_lit;
    if (current_led < NUM_LEDS) {
        leds[current_led] = CRGB(
            (int)(base_color.r * brightness / brightness_limit),
            (int)(base_color.g * brightness / brightness_limit),
            (int)(base_color.b * brightness / brightness_limit)
        );
        // if fixed study then also blink end pixel
        if(check_FixedStudy){
            leds[FixedStudy_Index] = CRGB(
                (int)(base_color.r * brightness / brightness_limit),
                (int)(base_color.g * brightness / brightness_limit),
                (int)(base_color.b * brightness / brightness_limit)
            );
        }
    }

    FastLED.show();

    // Check if block time elapsed
    if (block_elapsed >= time_per_block) {
        // Lock in current LED at brightness_limit
        leds[current_led] = base_color;
        FastLED.show();

        num_leds_lit++;
        study_counter++;
        start_block_time = now;

        if (Study_Mode_debug) {
            Serial.println("study_counter: " + String(study_counter));
        }
    }

    // if study time is bigger than 20 minutes enable streak counting
    if(study_counter >= 8){
        streak_enabled = true;
    }

    // All LEDs full: fade down and reset
    if (num_leds_lit == NUM_LEDS) {
        if (Study_Mode_debug) {
            Serial.println("LEDs full - fading down");
        }

        // Fade down all
        for (int i = full_fade_steps; i >= 0; i--) {
            float fade_brightness = brightness_limit * ((float)i / full_fade_steps);
            CRGB faded_color = base_color;
            faded_color.fadeToBlackBy(255 - (int)fade_brightness);
            fill_solid(leds, NUM_LEDS, faded_color);
            FastLED.show();
            delay(full_fade_delay_ms);
        }

        FastLED.clear();
        FastLED.show();
        num_leds_lit = 0;  // Reset
    }
}


void Study_Startup_Animation() {
    CRGB base_color = colorOptions[currentColorIndex];
    
    // Fade up (to full 255, capped by global brightness_limit)
    for (int i = 0; i <= fade_steps; i++) {
        int brightness = 255 * i / fade_steps;  // Full range
        CRGB scaled_color = base_color;
        scaled_color.fadeToBlackBy(255 - brightness);
        
        fill_solid(leds, NUM_LEDS, scaled_color);
        FastLED.show();
        delay(fade_delay_ms);
    }
    
    // Fade down
    for (int i = fade_steps; i >= 0; i--) {
        int brightness = 255 * i / fade_steps;
        CRGB scaled_color = base_color;
        scaled_color.fadeToBlackBy(255 - brightness);
        
        fill_solid(leds, NUM_LEDS, scaled_color);
        FastLED.show();
        delay(fade_delay_ms);
    }
    
    // Clear LEDs
    FastLED.clear();
    FastLED.show();
}


void Streak_Counter_Startup_Animation(int streak_counter) {
    CRGB base_color = colorOptions[currentColorIndex];
    
    // Fade up (to full 255, capped by global brightness_limit)
    for (int i = 0; i <= fade_steps; i++) {
        int brightness = 255 * i / fade_steps;  // Full range
        CRGB scaled_color = base_color;
        scaled_color.fadeToBlackBy(255 - brightness);
        
        FastLED.clear();  // Clear all LEDs
        
        // Light up center (index 0)
        leds[0] = scaled_color;
        
        // Light up right side (indices 1 to streak_counter)
        for (int j = 1; j <= streak_counter && j < NUM_LEDS; j++) {
            leds[j] = scaled_color;
        }
        
        // Light up left side (indices NUM_LEDS-1 down to NUM_LEDS-streak_counter)
        for (int j = 1; j <= streak_counter; j++) {
            int left_index = NUM_LEDS - j;
            if (left_index >= 0) {
                leds[left_index] = scaled_color;
            }
        }
        
        FastLED.show();
        delay(fade_delay_ms);
    }
    
    // Fade down
    for (int i = fade_steps; i >= 0; i--) {
        int brightness = 255 * i / fade_steps;
        CRGB scaled_color = base_color;
        scaled_color.fadeToBlackBy(255 - brightness);
        
        FastLED.clear();  // Clear all LEDs
        
        // Light up center (index 0)
        leds[0] = scaled_color;
        
        // Light up right side (indices 1 to streak_counter)
        for (int j = 1; j <= streak_counter && j < NUM_LEDS; j++) {
            leds[j] = scaled_color;
        }
        
        // Light up left side (indices NUM_LEDS-1 down to NUM_LEDS-streak_counter)
        for (int j = 1; j <= streak_counter; j++) {
            int left_index = NUM_LEDS - j;
            if (left_index >= 0) {
                leds[left_index] = scaled_color;
            }
        }
        
        FastLED.show();
        delay(fade_delay_ms);
    }
    
    // Clear LEDs
    FastLED.clear();
    FastLED.show();
}