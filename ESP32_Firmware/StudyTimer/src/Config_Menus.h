// Config_Menus.h file

#include "defines.h"

// Preset brightness levels (5 values)
const int brightnessLevels[] = { 5, 20, 50, 100, 250 };
const int brightnessLevelsCount = sizeof(brightnessLevels) / sizeof(brightnessLevels[0]);

// Index into the preset list (0..4)
int brightness_bar_count = 1;

static CRGB scaleColor(const CRGB &c, float scale) {
    return CRGB((uint8_t)(c.r * scale),
                (uint8_t)(c.g * scale),
                (uint8_t)(c.b * scale));
}

void handleBrightnessConfig(void) {
    // Update button states (pressed only once per press)
    int buttonPins[5] = {CENTER_BUTTON, UP_BUTTON, LEFT_BUTTON, DOWN_BUTTON, RIGHT_BUTTON};
    for (int b = 0; b < 5; b++) {
        bool pressed = (ss.digitalRead(buttonPins[b]) == LOW);
        if (pressed && !buttonStates[b]) {
            // Button just pressed
            buttonStates[b] = true;

            // LEFT increases (up to 5 LEDs)
            if (buttonPins[b] == LEFT_BUTTON) {
                if (brightness_bar_count < 5) {
                    brightness_bar_count++;
                    brightness_limit = brightnessLevels[brightness_bar_count - 1];
                    FastLED.setBrightness(brightness_limit);
                }
            }

            // RIGHT decreases (down to 1 LED)
            if (buttonPins[b] == RIGHT_BUTTON) {
                if (brightness_bar_count > 1) {
                    brightness_bar_count--;
                    brightness_limit = brightnessLevels[brightness_bar_count - 1];
                    FastLED.setBrightness(brightness_limit);
                }
            }

            // UP toggles status LED
            if (buttonPins[b] == UP_BUTTON) {
                enable_status_LED = !enable_status_LED;
            }

            // CENTER exits to main menu
            if (buttonPins[b] == CENTER_BUTTON) {
                preferences.putInt("br_bar", brightness_bar_count);
                // optional: also save brightness_limit
                preferences.putInt("br_lim", brightness_limit);
                currentState = STATE_MAIN_MENU;
                delay(200); // debounce to avoid immediate start of study mode
                FastLED.clear();
                FastLED.show();
                if (Main_Menu_debug) {
                    Serial.println("Returned from brightness menu");
                    Serial.print("brightness_bar_count: "); Serial.println(brightness_bar_count);
                    Serial.print("brightness_limit: "); Serial.println(brightness_limit);
                }
                return;
            }
            if(Config_Menus_debug){
                Serial.print("brightness_bar_count: "); Serial.println(brightness_bar_count);
                Serial.print("brightness_limit: "); Serial.println(brightness_limit);
            }
        } else if (!pressed) {
            // Reset state when released
            buttonStates[b] = false;
        }
    }

    // Render frame
    FastLED.clear();

    // (Optional) Status LED at index 0
    if (enable_status_LED) {
        leds[0] = scaleColor(CRGB::White, 0.05f);
    }

    // Bar LED range 6–10
    for (int i = 0; i < brightness_bar_count; i++) {
        if(brightness_bar_count == 1){
            Serial.println("brightness_bar_count is 1");
        }
        leds[6 + i] = scaleColor(colorOptions[currentColorIndex], 0.10f);
    }

    // Brightness indicator LEDs 17–19 show the current brightness limit
    CRGB brightColor = scaleColor(colorOptions[currentColorIndex], (float)brightness_limit / 255.0f);
    for (int i = 17; i <= 19; i++) {
        leds[i] = brightColor;
    }

    FastLED.show();
}

void handleModeConfig(void) {
    if (!mode_config_entered) {
        // Toggle mode on first entry
        check_FixedStudy = !check_FixedStudy;
        mode_config_entered = true;

        // save to NVM 
        preferences.putInt("mode_fix", check_FixedStudy);

        if (!check_FixedStudy) {
            // Unlimited mode: play animation once, then exit
            // Light up all LEDs sequentially
            for (int i = 0; i < 22; i++) {
                leds[i] = colorOptions[currentColorIndex];
                FastLED.show();
                delay(50);
            }
            delay(300);
            FastLED.clear();
            FastLED.show();

            // Light first 11 green
            for (int i = 0; i < 11; i++) {
                leds[i] = CRGB::Green;
            }
            FastLED.show();
            delay(500);
            FastLED.clear();
            FastLED.show();

            // Light first 16
            for (int i = 0; i < 16; i++) {
                leds[i] = colorOptions[currentColorIndex];
            }
            FastLED.show();
            delay(500);
            FastLED.clear();
            FastLED.show();

            // Light first 8 green
            for (int i = 0; i < 8; i++) {
                leds[i] = CRGB::Green;
            }
            FastLED.show();
            delay(250);

            // Turn off 7 to 0 sequentially
            for (int i = 7; i >= 0; i--) {
                leds[i] = CRGB::Black;
                FastLED.show();
                delay(50);
            }

            currentState = STATE_MAIN_MENU;
            mode_config_entered = false;  // Reset for next entry
            return;
        } else {
            // Fixed mode: start blinking
            config_blink_start = millis();
        }
    }

    if (check_FixedStudy) {
        // Fixed mode: blinking LED
        unsigned long now = millis();
        float t = fmod((now - config_blink_start) / 1000.0, config_blink_period);
        bool blink_on = t < (config_blink_period / 2);

        FastLED.clear();
        if (blink_on) {
            leds[FixedStudy_Index] = colorOptions[currentColorIndex];
        }
        FastLED.show();

        // LEFT or RIGHT: toggle index
        if (ss.digitalRead(LEFT_BUTTON) == LOW || ss.digitalRead(RIGHT_BUTTON) == LOW) {
            delay(200);
            FixedStudy_Index = (FixedStudy_Index == 9) ? 19 : 9;
            config_blink_start = now;
            Serial.print("FixedStudy_Index: "); Serial.println(FixedStudy_Index);
        }

        // CENTER: save and exit with animation
        if (ss.digitalRead(CENTER_BUTTON) == LOW) {
            delay(200);
            // Save to NVM (add later if needed)

            // Turn on FixedStudy_Index briefly
            leds[FixedStudy_Index] = colorOptions[currentColorIndex];
            FastLED.show();
            delay(50);

            // Sequentially turn on LEDs up to FixedStudy_Index
            for (int i = 0; i <= FixedStudy_Index; i++) {
                leds[i] = colorOptions[currentColorIndex];
                FastLED.show();
                delay(50);
            }

            // Turn off
            FastLED.clear();
            FastLED.show();

            preferences.putInt("fix_index", FixedStudy_Index);

            currentState = STATE_MAIN_MENU;
            mode_config_entered = false;
        }
    }
}