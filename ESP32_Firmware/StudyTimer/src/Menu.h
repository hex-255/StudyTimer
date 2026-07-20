// Menu.h file

#pragma once

#include "defines.h"
#include "FuelGauge.h"

void debug_animation_lightup();

void sound_debug(int frequency = 1000, unsigned long durationMs = 300, int loudness = 200);

void buttonImpactAnimation(int center);

void scanI2CDevices(void);

void updateStatusArrayNoShow();

void handleMainMenu(void) {
    bool activity = false;
    unsigned long now = millis();

    // Get current encoder position
    int32_t raw_position = ss.getEncoderPosition();
    int32_t position = raw_position - encoder_offset;

    // Compute valid LED indices (wrap around for negative positions)
    int current_led_index = (position % NUM_LEDS + NUM_LEDS) % NUM_LEDS;
    int last_led_index = (last_position % NUM_LEDS + NUM_LEDS) % NUM_LEDS;


    // Check buttons for activity (pressed or released)
    int buttonPins[5] = {CENTER_BUTTON, UP_BUTTON, LEFT_BUTTON, DOWN_BUTTON, RIGHT_BUTTON};
    for (int b = 0; b < 5; b++) {
        bool currentState = (ss.digitalRead(buttonPins[b]) == LOW);  // LOW = pressed
        if (currentState != buttonStates[b]) {
            activity = true;
            buttonStates[b] = currentState;
            if (currentState) {  // On press
                FastLED.clear();
                updateStatusArrayNoShow();
                FastLED.show();
            }
        }
    }

    // Check encoder movement
    if (position != last_position) {
        activity = true;
    }

    if (activity) {
        encoder_last_activity = now;
    }

    // Update pixel position if encoder moved
    if (position != last_position) {
        if(Main_Menu_debug) {
            Serial.print("Encoder position: "); Serial.println(current_led_index);
        }
        // Turn off previous pixel
        leds[last_led_index] = CRGB::Black;
        
        // Turn on new pixel with current color
        leds[current_led_index] = colorOptions[currentColorIndex];
        FastLED.show();
        
        last_position = position;
    }

    // Idle timeout: turn off LEDs after 10 seconds
    if ((now - encoder_last_activity) > ENCODER_IDLE_TIMEOUT) {
        if(Main_Menu_debug) {
            Serial.println("Idle timeout - turning off LEDs");
        }
        FastLED.clear();
        updateStatusArrayNoShow();
        FastLED.show();

        encoder_last_activity = now;  // Reset timer to prevent repeated clearing
    }

    //readPCA9536();  // Check magnet status on each loop (optional, for real-time updates)   

    if(magnet_detected){    
        // Turn status LED with index 1 on when magnet is detected -> color Blue
        status_leds[1] = CRGB::Aqua;
        } 
        else 
        {
            status_leds[1] = CRGB::Black;
        }

    // UP button: cycle color (buttons[1] = UP_BUTTON)
    if (ss.digitalRead(UP_BUTTON) == LOW) {

        delay(200);  // Debounce
        currentColorIndex = (currentColorIndex + 1) % numColors;
        
        if(Main_Menu_debug) {
            Serial.println("UP button pressed - cycling color");
            Serial.print("Current color: "); Serial.println(colorOptions_strings[currentColorIndex]);
        }

        // Reset encoder offset and position
        encoder_offset = ss.getEncoderPosition();
        last_position = 0;
        
        FastLED.clear();
        updateStatusArrayNoShow();
        leds[0] = colorOptions[currentColorIndex];
        FastLED.show();
    }

    // LEFT button: toggle Fixed study mode (buttons[2] = LEFT_BUTTON)
    if (ss.digitalRead(LEFT_BUTTON) == LOW) {
        delay(200);
        // Add your FixedStudy toggle logic here (e.g., state = STATE_CONFIG_FIXED)
        // For now, just print or set a flag
        Serial.println("Toggling Fixed Study Mode");
        currentState = STATE_MODE_CONFIG;  // Uncomment when implemented
    }

    // DOWN button: Run I2C scan (buttons[3] = DOWN_BUTTON)
    if (ss.digitalRead(DOWN_BUTTON) == LOW) {
        delay(200);
        // Add debug mode toggle logic
        Serial.println("Turning on Debug Mode");
        // Print battery voltage if you have analog pin
        // int analog_value = analogRead(A1);  // Adjust pin
        // float voltage = (analog_value / 4095.0) * 3.3 * 2;
        // Serial.print("Voltage: "); Serial.println(voltage);
        debug_animation_lightup();

        if(audio_enabled){  
            sound_debug(1000, 200, 80);
            delay(50);
            sound_debug(3000, 100, 250);
        }

        if(FuelGauge_debug) {

            Serial.print("Fuel Gauge Voltage: ");
            Serial.println(getFuelGaugeVoltage());

            Serial.print("Fuel Gauge Temperature: ");
            Serial.println(getFuelGaugeTemperature());

            Serial.print("Fuel Gauge Current: ");
            Serial.println(getFuelGaugeCurrent());
            // smaller than zero is charging 

            Serial.print("Battery Status: ");
            Serial.println(getFuelGaugeSOC());

        }

        if(I2C_debug){
            scanI2CDevices();
        }

        time_per_block = 5;
        time_per_block_break = 5;
    }

    // RIGHT button: brightness config (buttons[4] = RIGHT_BUTTON)
    if (ss.digitalRead(RIGHT_BUTTON) == LOW) {
        delay(200);
        currentState = STATE_BRIGHTNESS_CONFIG;
    }

    // CENTER button: game mode (buttons[0] = CENTER_BUTTON)
    if (ss.digitalRead(CENTER_BUTTON) == LOW) {
        delay(200);
        if(magnet_detected){
            strict_mode = true;
            if(Main_Menu_debug) {
                Serial.println("CENTER button pressed - entering Study Mode with strict mode ON");
            }
        }
        currentState = STATE_STUDY_MODE;  // Or STATE_GAME_MODE if you add it
    }

    if(right_click){
        buttonImpactAnimation(4);  // Animate the LED above right shoulder button
    }

    if(left_click){
        buttonImpactAnimation(20);  // Animate the LED above left shoulder button
    }

    // Ensure updates are shown but keep status LEDs unchanged
    updateStatusArrayNoShow();
    FastLED.show();
}


void debug_animation_lightup(){
    const int sequential_delay = 100;  // Delay in ms between turning on each even LED
    const int jump_delay = 500;        // Delay in ms for jumps and final off
    CRGB animation_color = colorOptions[currentColorIndex];  // Use current color index

    // Step 1: Startup - Turn on even indices one after the other in a ring
    FastLED.clear();
    for (int i = 0; i < NUM_LEDS; i += 2) {
        leds[i] = animation_color;
        FastLED.show();
        delay(sequential_delay);
    }

    // Step 2: Jump to odd indices (turn even off, odd on all at once)
    FastLED.clear();
    for (int i = 1; i < NUM_LEDS; i += 2) {
        leds[i] = animation_color;
    }
    FastLED.show();
    delay(jump_delay);

    // Step 3: Jump back to even indices (turn odd off, even on all at once)
    FastLED.clear();
    for (int i = 0; i < NUM_LEDS; i += 2) {
        leds[i] = animation_color;
    }
    FastLED.show();
    delay(jump_delay);

    // Step 4: Turn off all LEDs
    FastLED.clear();
    FastLED.show();
}

void sound_debug(int frequency, unsigned long durationMs, int loudness) {
    if (Main_Menu_debug) {
        Serial.print("Playing debug tone: ");
        Serial.print(frequency);
        Serial.println(" Hz");
    }

    ledcWriteTone(TONE_CHANNEL, frequency);
    ledcWrite(TONE_CHANNEL, loudness);
    delay(durationMs);
    ledcWriteTone(TONE_CHANNEL, 0);
}

void buttonImpactAnimation(int center) {
    FastLED.clear();

    CRGB baseColor = colorOptions[currentColorIndex];
    int stepDelay = random(60, 100);   // random delay per step
    int maxSpread = random(1, 4);      // random propagation distance: 1..4 pairs

    // Step 1: center on
    leds[center] = baseColor;
    updateStatusArrayNoShow();
    FastLED.show();
    delay(stepDelay);

    for (int step = 1; step <= maxSpread; ++step) {
        int left  = (center - step + NUM_LEDS) % NUM_LEDS;
        int right = (center + step) % NUM_LEDS;

        leds[left] = baseColor;
        leds[right] = baseColor;
        updateStatusArrayNoShow();
        FastLED.show();
        delay(stepDelay - step * 20);   

        // Turn the center off when the second ring appears
        if (step == 2) {
            leds[center] = CRGB::Black;
            updateStatusArrayNoShow();
            FastLED.show();
            delay(stepDelay - step * 20);
        }

        if(step >= 2){
            leds[left+1] = CRGB::Black;  
            leds[right-1] = CRGB::Black;
            updateStatusArrayNoShow();
            FastLED.show();
            delay(stepDelay - step * 20);
        }
    }

    // Keep the final frame visible briefly, then clear
    delay(stepDelay);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    updateStatusArrayNoShow();
    FastLED.show();
}

// this function updates status LEDs so they don't get overwritten by FastLED.show()
void updateStatusArrayNoShow() {
    if (enable_status_LED) {
        // audio status LED at index 2: orange if enabled, blue if disabled
        status_leds[2] = audio_enabled ? CRGB::Orange : CRGB::Blue;
        if(FuelGauge_available){
        // battery status LED at index 0: green if >60%, orange if 20-60%, red if <20%
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
    } else {
        status_leds[0] = CRGB::Black;
        status_leds[2] = CRGB::Black;
    }
    status_leds[1] = magnet_detected ? CRGB::Aqua : CRGB::Black;
}

void scanI2CDevices(void) {
    Serial.println("\n=== I2C Scanner ===");
    Serial.println("Scanning I2C addresses...\n");
    
    int deviceCount = 0;
    
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        
        if (error == 0) {
            if(address == 0x38){
                Serial.println("PCF8574 detected at 0x38");
            }
            else if(address == 0x41){
                Serial.println("PCA9536 detected at 0x41");
            }
            else if(address == 0x49){
                Serial.println("Attiny detected at 0x49");
            }
            else if(address == 0x55){
                Serial.println("Fuel Gauge detected at 0x55");
            }
            else{
                Serial.print("Unknown Device found at address 0x");
                if (address < 16) Serial.print("0");
                Serial.print(address, HEX);
                Serial.println(" !");
            }
            deviceCount++;
        }
    }
    
    Serial.print("\nTotal devices found: ");
    Serial.println(deviceCount);
    Serial.println("===================\n");
    
}