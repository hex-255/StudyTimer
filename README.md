# StudyTimer
study timer with neopixel feedback and adaptive break time calculation

study for amount of time you decide and get break time allotted accordingly. 

break time = time studied divided by 5

formfactor inspired by [Adafruit NeoPixel rotary fidget](https://learn.adafruit.com/neopixel-rotary-fidget/overview) 

## Features
- Neopixel visual feedback (one pixel equals 2.5 minutes of study time or 1 minute of breaktime)
- Adaptive break time calculation
- Audio feedback with mini speaker
- interface through [Adafruit ANO Rotary Encoder](https://learn.adafruit.com/ano-rotary-encoder)



https://github.com/user-attachments/assets/7cb4c156-3706-4063-9436-d46780e94b0a

## User Interface

### Button Up 
- changes color

### Encoder Wheel
- moves pixel


https://github.com/user-attachments/assets/a1adc233-345f-403a-b1bf-03c040c178a4

### Button Left: Mode Select
- changes mode between infinite and fixed interval
- infinite mode animation

https://github.com/user-attachments/assets/7b64e9f7-16fe-4978-8f38-6b15f6c3b560

- fixed interval select between 25 or 50 minutes
- animation shows selection
- last pixel of interval blinks during study period in fixed mode


https://github.com/user-attachments/assets/2e6db207-5c3a-48b9-bd2f-25875b27d2fb




### Button Right: Brightness
- left and right increases or decreases brightness
- pixels on left side show current brightness
- up arrow toggles status LED on or off


https://github.com/user-attachments/assets/31a64ca0-6e88-40d8-a793-a3a0c24d96c2


### Button Down: toggle Debug Mode on or off
- device counts much quicker in debug mode

### Status LEDs and slide switches
- left LED: Power -> green: on; orange: low battery;
- left slide switch: turn device off
- right LED: Audio -> orange: speaker enabled; blue: speaker disabled;
- right slide switch: toggle speaker

