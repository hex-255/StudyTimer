// BreakMode.h file

#include "defines.h"
#include "BreakOverflow.h"
#include "Menu.h"

void Break_Startup_Animation();

void handleBreakMode(void) {
    // Initialize on first entry
    if (!break_started) {
        break_interval = study_counter / 2;
        if(Break_Mode_debug) {
            Serial.println("Entered Break Mode");
            Serial.print("study_counter: "); Serial.println(study_counter);
            Serial.print("break_interval (half): "); Serial.println(break_interval);
        }
        //reset study counter and num_Leds_lit
        study_counter = 0;
        num_leds_lit = 0;
        
        current_led_index = break_interval - 1;
        break_start_block_time = millis();
        break_started = true;
    }

    // Play break startup animation once
    if (!breakStartupPlayed) {
        if(Break_Mode_debug) {
            Serial.println("Playing Break Startup Animation");
        }
        Break_Startup_Animation();
        breakStartupPlayed = true;
    }


    // If countdown complete, exit
    if (current_led_index < 0) {
        // play sound if audio enabled
        if(audio_enabled){
            sound_debug(1000, 200, 80);
            delay(50);
            sound_debug(3000, 100, 250);
        }
        // if streak enabled go to break overflow, else go to main menu
        if(streak_enabled){
            if(Break_Mode_debug) {
            Serial.println("Break complete - going to break overflow");
            }
            FastLED.clear();
            FastLED.show();
            currentState = STATE_BREAK_OVERFLOW;
            break_started = false;
            return;
        }
        if(!streak_enabled){
            if(Break_Mode_debug) {
            Serial.println("Break complete - going to Main Menu");
            }
            FastLED.clear();
            FastLED.show();
            streak_counter = 0; // Reset streak counter because study time was not long enough
            currentState = STATE_MAIN_MENU;
            controlPCF8574_LEDs(0b11111111); // turn off all streak LEDs
            break_started = false;
            return;
        }
        
    }

    unsigned long now = millis();
    float block_elapsed = (now - break_start_block_time) / 1000.0;  // Seconds

    // Calculate animation cycles
    float num_cycles = (float)time_per_block_break / break_total_anim_duration;
    int cycles_completed = (int)(block_elapsed / break_total_anim_duration);
    bool in_last_cycle = (cycles_completed >= (int)(num_cycles - 1));

    // Calculate animation phase
    float t = fmod(block_elapsed, break_total_anim_duration);
    float brightness = 0.0;
    bool reached_zero = false;

    if (in_last_cycle) {
        // Last cycle: fade down only, then stay off
        if (t < break_fade_down_duration) {
            brightness = brightness_limit - brightness_limit * (t / break_fade_down_duration);
        } else {
            brightness = 0.0;
            reached_zero = true;
        }
    } else {
        // Normal: fade down → fade up → stay on
        if (t < break_fade_down_duration) {
            brightness = brightness_limit - brightness_limit * (t / break_fade_down_duration);
        } else if (t < break_fade_down_duration + break_fade_up_duration) {
            brightness = brightness_limit * ((t - break_fade_down_duration) / break_fade_up_duration);
        } else {
            brightness = brightness_limit;
        }
    }

    // Render LEDs (green)
    CRGB solid_green = CRGB::Green;
    for (int i = 0; i < break_interval; i++) {
        if (i > current_led_index) {
            // LEDs already counted down: OFF
            leds[i] = CRGB::Black;
        } else if (i < current_led_index) {
            // LEDs that are "solid": fully on
            leds[i] = solid_green;
        } else {
            // Current LED: animated
            leds[i] = CRGB(
                (int)(solid_green.r * brightness / brightness_limit),
                (int)(solid_green.g * brightness / brightness_limit),
                (int)(solid_green.b * brightness / brightness_limit)
            );
        }
    }
    FastLED.show();

    // Check if time to move to next LED
    if (block_elapsed >= time_per_block_break || reached_zero) {
        if(Break_Mode_debug){
            Serial.print("Blocks left: "); Serial.println(current_led_index);
        }
        current_led_index--;
        break_start_block_time = now;
    }

    // Pee Break -> if user clicks shoulder buttons jump back into study mode
    if(right_click or left_click){

        study_counter = (current_led_index + 1) * 2; // Set study counter according to remaining break
        num_leds_lit = study_counter; // Set lit LEDs according to study counter

        if (Break_Mode_debug) {
            Serial.println("Pee Break aborted -> to Study Mode");
            Serial.print("Study counter set to: ");
            Serial.println(study_counter);
        }

        FastLED.clear();
        FastLED.show();


        currentState = STATE_STUDY_MODE;
        break_started = false;  // Reset for next entry
        return;
    }
}

void Break_Startup_Animation() {
    CRGB base_color = CRGB::Green;  // You can choose a different color for the break animation
    
    // Fade up (to full 255, capped by global brightness_limit)
    for (int i = 0; i <= fade_steps; i++) {
        int brightness = 255 * i / fade_steps;  // Full range
        CRGB scaled_color = base_color;
        scaled_color.fadeToBlackBy(255 - brightness);
        
        fill_solid(leds, break_interval, scaled_color);
        FastLED.show();
        delay(fade_delay_ms);
    }
    
    // Fade down
    for (int i = fade_steps; i >= 0; i--) {
        int brightness = 255 * i / fade_steps;
        CRGB scaled_color = base_color;
        scaled_color.fadeToBlackBy(255 - brightness);
        
        fill_solid(leds, break_interval, scaled_color);
        FastLED.show();
        delay(fade_delay_ms);
    }

    // Fade up (to full 255, capped by global brightness_limit)
    for (int i = 0; i <= fade_steps; i++) {
        int brightness = 255 * i / fade_steps;  // Full range
        CRGB scaled_color = base_color;
        scaled_color.fadeToBlackBy(255 - brightness);
        
        fill_solid(leds, break_interval, scaled_color);
        FastLED.show();
        delay(fade_delay_ms);
    }
    
}