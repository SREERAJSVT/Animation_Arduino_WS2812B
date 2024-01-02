#include "FastLED.h"

#define NUM_LEDS 50
#define DATA_PIN 6
#define COLOR_ORDER GRB
#define CHIPSET NEOPIXEL
#define BRIGHTNESS 255
#define GRAVITY -9.81
#define h0 1
#define NUM_BALLS 6

float h[NUM_BALLS];
float vImpact0 = sqrt(-2 * GRAVITY * h0);
float vImpact[NUM_BALLS];
float tCycle[NUM_BALLS];
int pos[NUM_BALLS];
long tLast[NUM_BALLS];
float COR[NUM_BALLS];
CRGB leds[NUM_LEDS];

int FlameHeight = 50;
int Sparks = 50;
int DelayDuration = 10;

enum AnimationState
{
  NO_ANIMATION,
  FIRE_EFFECT,
  BOUNCING_BALLS
};

AnimationState currentState = NO_ANIMATION;
unsigned long animationStartTime;

void setup()
{
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
  FastLED.clear();
  delay(2000);
  FastLED.addLeds<CHIPSET, DATA_PIN>(leds, NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);
  Serial.begin(115200);

  for (int i = 0; i < NUM_BALLS; i++)
  {
    tLast[i] = millis();
    h[i] = h0;
    pos[i] = 0;
    vImpact[i] = vImpact0;
    tCycle[i] = 0;
    COR[i] = 0.90 - float(i) / pow(NUM_BALLS, 2);
  }

  animationStartTime = millis();
}

void loop()
{
  switch (currentState)
  {
  case NO_ANIMATION:
    runNoAnimation();
    break;
  case FIRE_EFFECT:
    runFireEffect();
    break;
  case BOUNCING_BALLS:
    runBouncingBalls();
    break;
  }

  delay(1);
}void setPixelHeatColor(int Pixel, byte temperature)
{
  // Rescale heat from 0-255 to 0-191
  byte t192 = round((temperature / 255.0) * 191);

  // Calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0...63
  heatramp <<= 2;              // scale up to 0...252

  // Figure out which third of the spectrum we're in:
  if (t192 > 0x80)
  { // hottest
    leds[Pixel].setRGB(255, 255, heatramp);
  }
  else if (t192 > 0x40)
  { // middle
    leds[Pixel].setRGB(255, heatramp, 0);
  }
  else
  { // coolest
    leds[Pixel].setRGB(heatramp, 0, 0);
  }
}


void runNoAnimation()
{
  // Wait for a while before starting the first animation
  if (millis() - animationStartTime >= 500)
  {
    FastLED.clear(); // Clear the LED strip
    currentState = FIRE_EFFECT;
    animationStartTime = millis();
  }
}

void runFireEffect()
{
  static byte heat[NUM_LEDS];
  int cooldown;

  // Cool down each cell a little
  for (int i = 0; i < NUM_LEDS; i++)
  {
    cooldown = random(0, ((FlameHeight * 10) / NUM_LEDS) + 2);

    if (cooldown > heat[i])
    {
      heat[i] = 0;
    }
    else
    {
      heat[i] = heat[i] - cooldown;
    }
  }

  // Heat from each cell drifts up and diffuses slightly
  for (int k = (NUM_LEDS - 1); k >= 2; k--)
  {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Randomly ignite new Sparks near the bottom of the flame
  if (random(255) < Sparks)
  {
    int y = random(7);
    heat[y] = heat[y] + random(160, 255);
  }

  // Convert heat to LED colors
  for (int j = 0; j < NUM_LEDS; j++)
  {
    setPixelHeatColor(j, heat[j]);
  }

  FastLED.show();

  // Check if the fire effect has been running for 5 seconds, then switch to bouncing balls
  if (millis() - animationStartTime >= 5000)
  {
    FastLED.clear(); // Clear the LED strip before switching to the next animation
    currentState = BOUNCING_BALLS;
    animationStartTime = millis();
  }

  delay(DelayDuration);
}

void runBouncingBalls()
{
  for (int i = 0; i < NUM_BALLS; i++)
  {
    tCycle[i] = millis() - tLast[i];

    h[i] = 0.5 * GRAVITY * pow(tCycle[i] / 1000, 2.0) + vImpact[i] * tCycle[i] / 1000;

    if (h[i] < 0)
    {
      h[i] = 0;
      vImpact[i] = COR[i] * vImpact[i];
      tLast[i] = millis();

      if (vImpact[i] < 0.01)
        vImpact[i] = vImpact0;
    }

    pos[i] = round(h[i] * (NUM_LEDS - 1) / h0);
  }

  for (int i = 0; i < NUM_BALLS; i++)
  {
    leds[pos[i]] = CHSV(uint8_t(i * 40), 255, 255);
  }
  FastLED.show();

  for (int i = 0; i < NUM_BALLS; i++)
  {
    leds[pos[i]] = CRGB::Black;
  }

  // Reset the animation state to no animation after running for 5 seconds
  if (millis() - animationStartTime >= 5000)
  {
    FastLED.clear(); // Clear the LED strip before switching to the next animation
    currentState = NO_ANIMATION;
    animationStartTime = millis();
  }
}
