#include "FastLED.h"
#define NUM_LEDS  50     // Enter the total number of LEDs on the strip
#include <FastLED.h>
#define DATA_PIN          6
#define COLOR_ORDER       GRB
#define CHIPSET           NEOPIXEL
#define BRIGHTNESS        255
#define GRAVITY           -9.81              // Downward (negative) acceleration of gravity in m/s^2
#define h0                1                  // Starting height, in meters, of the ball (strip length)
#define NUM_BALLS         6                 // Number of bouncing balls you want (recommend < 7, but 20 is fun in its own way)

float h[NUM_BALLS] ;                         // An array of heights
float vImpact0 = sqrt( -2 * GRAVITY * h0 );  // Impact velocity of the ball when it hits the ground if "dropped" from the top of the strip
float vImpact[NUM_BALLS] ;                   // As time goes on the impact velocity will change, so make an array to store those values
float tCycle[NUM_BALLS] ;                    // The time since the last time the ball struck the ground
int   pos[NUM_BALLS] ;                       // The integer position of the dot on the strip (LED index)
long  tLast[NUM_BALLS] ;                     // The clock time of the last ground strike
float COR[NUM_BALLS] ;                       // Coefficient of Restitution (bounce damping)
CRGB leds[NUM_LEDS];
bool fireModeS = true;  // Initial mode is Fire
void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);    // Set power limit of LED strip to 5V, 1500mA
  FastLED.clear();// Initialize all LEDs to "OFF"

  delay(2000);                               // sanity check delay - allows reprogramming if accidently blowing power w/leds
  FastLED.addLeds<CHIPSET, DATA_PIN>(leds, NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);
  Serial.begin(115200);
  for (int i = 0 ; i < NUM_BALLS ; i++) {    // Initialize variables
    tLast[i] = millis();
    h[i] = h0;
    pos[i] = 0;                              // Balls start on the ground
    vImpact[i] = vImpact0;                   // And "pop" up at vImpact0
    tCycle[i] = 0;
    COR[i] = 0.90 - float(i) / pow(NUM_BALLS, 2);
  }
}


void loop() {
  if (fireModeS) {
    Fire(50, 100, 10);
  } else {
    NewLED();
  }

  // Toggle mode in each iteration
  fireModeS = !fireModeS;

  // You can add some delay if needed
  delay(1);  // Adjust the delay as necessary
}

void setPixelHeatColor(int Pixel, byte temperature) {
  // Rescale heat from 0-255 to 0-191
  byte t192 = round((temperature / 255.0) * 191);
  
  // Calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0...63
  heatramp <<= 2; // scale up to 0...252
  
  // Figure out which third of the spectrum we're in:
  if(t192 > 0x80) {                    // hottest
    leds[Pixel].setRGB(255, 255, heatramp);
  }
  else if(t192 > 0x40) {               // middle
    leds[Pixel].setRGB(255, heatramp, 0);
  }
  else {                               // coolest
    leds[Pixel].setRGB(heatramp, 0, 0);
  }
}
void Fire(int FlameHeight, int Sparks, int DelayDuration) 

{
  static byte heat[NUM_LEDS];
  int cooldown;

  // Cool down each cell a little
  for (int i = 0; i < NUM_LEDS; i++) {
    cooldown = random(0, ((FlameHeight * 10) / NUM_LEDS) + 2);

    if (cooldown > heat[i]) {
      heat[i] = 0;
    }
    else {
      heat[i] = heat[i] - cooldown;
    }
  }

  // Heat from each cell drifts up and diffuses slightly
  for (int k = (NUM_LEDS - 1); k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Randomly ignite new Sparks near bottom of the flame
  if (random(255) < Sparks) {
    int y = random(7);
    heat[y] = heat[y] + random(160, 255);
  }

  // Convert heat to LED colors
  for (int j = 0; j < NUM_LEDS; j++) {
    setPixelHeatColor(j, heat[j]);
  }

  FastLED.show();
  delay(DelayDuration);
}
void NewLED()
{
for (int i = 0 ; i < NUM_BALLS ; i++) {
  tCycle[i] =  millis() - tLast[i] ;     // Calculate the time since the last time the ball was on the ground

  // A little kinematics equation calculates positon as a function of time, acceleration (gravity) and intial velocity
  h[i] = 0.5 * GRAVITY * pow( tCycle[i] / 1000 , 2.0 ) + vImpact[i] * tCycle[i] / 1000;

  if ( h[i] < 0 ) {
    h[i] = 0;                            // If the ball crossed the threshold of the "ground," put it back on the ground
    vImpact[i] = COR[i] * vImpact[i] ;   // and recalculate its new upward velocity as it's old velocity * COR
    tLast[i] = millis();

    if ( vImpact[i] < 0.01 ) vImpact[i] = vImpact0;  // If the ball is barely moving, "pop" it back up at vImpact0
  }
  pos[i] = round( h[i] * (NUM_LEDS - 1) / h0);       // Map "h" to a "pos" integer index position on the LED strip
}

//Choose color of LEDs, then the "pos" LED on
for (int i = 0 ; i < NUM_BALLS ; i++) leds[pos[i]] = CHSV( uint8_t (i * 40) , 255, 255);
FastLED.show();
//Then off for the next loop around
for (int i = 0 ; i < NUM_BALLS ; i++) {
  leds[pos[i]] = CRGB::Black;
}
}
