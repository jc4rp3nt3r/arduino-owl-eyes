#include <Adafruit_NeoPixel.h>
#include "NeoPatterns.cpp"
#include <math.h>

#define NOCATCHUP 1   // Don't let the timer try to "keep time" using the interval, just exec the code as soon as we can
#include <Metro.h>

#define DATA_PIN_LEFT 6
#define DATA_PIN_RIGHT 7
#define DATA_PIN_BUTTON 8
#define LED_COUNT 12

// Parameter 1 = number of pixels
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
/// Adafruit_NeoPixel leftEye = Adafruit_NeoPixel(LED_COUNT, DATA_PIN_LEFT, NEO_RGBW + NEO_KHZ800);
/// Adafruit_NeoPixel rightEye = Adafruit_NeoPixel(LED_COUNT, DATA_PIN_RIGHT, NEO_RGBW + NEO_KHZ800);
void Ring1Complete();
void Ring2Complete();
NeoPatterns Ring1(LED_COUNT, DATA_PIN_LEFT, NEO_RGBW + NEO_KHZ800, &Ring1Complete);
NeoPatterns Ring2(LED_COUNT, DATA_PIN_RIGHT, NEO_RGBW + NEO_KHZ800, &Ring2Complete);

Metro ledTimer;
uint16_t iDelay = 0;
#define ANIMATION_LEN 41
uint16_t arrDelay[ANIMATION_LEN] = {
                      180, 150, 120, 90, 60, 30,      // 6 - eye open - slow
                      2000,                           // 1 pause - eyes open
                      30, 30, 30, 30, 30, 30,         // 6 - eye close - blink
                      30, 30, 30, 30, 30, 30,         // 6 - eye open - blink
                      800,                            // 1 - pause - blink open
                      30, 30, 30, 30, 30, 30,         // 6 - eye close - blink
                      30, 30, 30, 30, 30, 30,         // 6 - eye open - blink
                      2500,                           // 1 - pause - eyes open
                      180, 150, 120, 90, 60, 30,      // 6 - eye close - slow
                      1, 0                            // 2 - cloe eye and stop animation
              };       

const uint8_t PROGMEM gamma8[] = {                    // LED Gamma Correction matrix
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

// uint32_t eyeColorOn;
// uint32_t eyeColorOff;

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  Serial.begin(115200);

  // Initialize all the pixelStrips
  Ring1.setBrightness(30);
  Ring1.begin();
  Ring2.setBrightness(30);
  Ring2.begin();
  
  // Enable internal pullups on the switch inputs
  pinMode(DATA_PIN_BUTTON, INPUT_PULLUP);

  //attachInterrupt(0, magnet_detect, FALLING);            // Initialize the intterrupt pin 0 (Arduino digital pin 2)
  
  // Kick off a pattern
  uint32_t iColor = Ring1.Color(pgm_read_byte(&gamma8[187]), pgm_read_byte(&gamma8[255]), pgm_read_byte(&gamma8[64]));
  Ring1.Wipe(iColor, 10, FORWARD);     // setting internal delay to 10ms, because we are managing 
  Ring2.Wipe(iColor, 10, FORWARD);     // the delay animation in our loop code below

  ledTimer = Metro(arrDelay[iDelay++]);
}


void loop() {
  // Switch patterns on a button press:
  bool bPaused = arrDelay[iDelay] == 0;

  //Serial.println(digitalRead(DATA_PIN_BUTTON));
  if (bPaused && digitalRead(DATA_PIN_BUTTON) == LOW) // Button #1 pressed
  {
    Serial.print(millis());
    Serial.println("-reset");
    iDelay = 0;
    ledTimer.interval(arrDelay[iDelay++]);                        // update the delay based on the timeline
    ledTimer.reset();
  }
  // else {
  //   Ring1.Interval = Ring2.Interval = 200;
  //   Ring1.Color1 = Ring2.Color1 = Ring1.Color(109, 255,0);
  // }
  
  // Update the rings.
  if (!bPaused && ledTimer.check() == 1) {         // if we are not on a pause step && the timer elapsed
    Ring1.Update();                                             // update ring 1
    Ring2.Update();                                             // update ring 2

    if (iDelay == ANIMATION_LEN-1) iDelay = 0;                  // array bounds safty check, reset the timeline if we hit the end
    ledTimer.interval(arrDelay[iDelay++]);                      // update the delay based on the timeline
    ledTimer.reset();


    //Ring1.Color1 = Ring1.Wheel(Ring1.Index * 40);   // index 0 - 5
    //Serial.println(ledDelay);
  }
}


void magnet_detect()//This function is called whenever a magnet/interrupt is detected by the arduino
{
  iDelay = 0;
  ledTimer.interval(arrDelay[iDelay++]);                        // update the delay based on the timeline
  ledTimer.reset();
  Serial.print(millis());
  Serial.println("-reset");
  //Ring1.Color1 = Ring2.Color1 = Ring1.Color(0, 255,0);
}

// Ring1 Completion Callback
void Ring1Complete()
{
  //Ring1.Color2 = Ring2.Color2 = Ring1.Color(20, 100,30);
  Ring1.Reverse();

}

// Ring 2 Completion Callback
void Ring2Complete()
{
  Ring2.Reverse();
}