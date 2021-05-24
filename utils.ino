
#include <Adafruit_SleepyDog.h>
#include <Adafruit_NeoPixel.h>


// delay between updates
void sleepFor(long sleep_time) {
  #if LOW_POWER
    cum_sleep_time += Watchdog.sleep(sleep_time);
  #else
    delay(sleep_time);
  #endif
}

// Input a value 0 to 255 to get a color value.
// The colors transition r -> g -> b -> r
uint32_t GetWheelColor(uint8_t position) {
  position = 255 - position;
  if (position < 85) {
    return strip.Color(255 - position * 3, 0, position * 3);
  } else if (position < 170) {
    position -= 85;
    return strip.Color(0, position * 3, 255 - position * 3);
  } else {
    position -= 170;
    return strip.Color(position * 3, 255 - position * 3, 0);
  }
}

uint32_t interpolate(uint32_t colA, uint32_t colorB, uint8_t percent) {
  uint16_t rA = (colA >> 16) & 0x000000FF;
  uint16_t gA = (colA >> 8) & 0x000000FF;
  uint16_t bA = (colA >> 0) & 0x000000FF;
  uint16_t rB = (colorB >> 16) & 0x000000FF;
  uint16_t gB = (colorB >> 8) & 0x000000FF;
  uint16_t bB = (colorB >> 0) & 0x000000FF;
  uint8_t inverse = 255 - percent;

 return strip.Color(
   (rA * 255 * inverse + rB * 255 * percent) / (255*255),
   (gA * 255 * inverse + gB * 255 * percent) / (255*255),
   (bA * 255 * inverse + bB * 255 * percent) / (255*255)
  );
}
 
float easeInOutQuint(float x) {
  return -(cos(PI * x) - 1) / 2;
}