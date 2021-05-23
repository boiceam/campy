
// Declare shows

#include <Adafruit_SleepyDog.h>
#include <Adafruit_NeoPixel.h>


uint32_t Show_Rainbow_Bouncing_Dot(long time_ms, size_t light_index);
uint32_t Show_Rainbow_Cycle(long time_ms, size_t light_index);
uint32_t Show_Pulse(long time_ms, size_t light_index);

uint32_t Show_Rainbow_Bouncing_Dot(long time_ms, size_t light_index) {
  float colorT = (float)(time_ms % 30000) / 30000;
  float t = (float)(time_ms % 4000) / 4000;
  float t2 = easeInOutQuint(t);

  float on_light = abs(t2 * (LED_COUNT + 2) * 2 - (LED_COUNT + 2));
  float dist_from_on_light = ((float)light_index - on_light);

  if (t >= 0.5) dist_from_on_light *= -1;
  if (dist_from_on_light < 0) return strip.Color(0, 0, 0);

  uint32_t color = GetWheelColor(colorT * 255);
  uint16_t rA = COLOR_BREAKDOWN_R(color);
  uint16_t gA = (color >> 8) & 0x000000FF;
  uint16_t bA = (color >> 0) & 0x000000FF;

  float val = min(1, max(1 - dist_from_on_light * 0.25, 0));
  return strip.Color(rA * val, gA * val, bA * val);
}

uint32_t Show_Flicker(long time_ms, size_t light_index) {
  uint32_t color = Show_Pulse(time_ms, light_index);
  uint16_t rA = (color >> 16) & 0x000000FF;
  uint16_t gA = (color >> 8) & 0x000000FF;
  uint16_t bA = (color >> 0) & 0x000000FF;

  uint8_t n = abs(256 - (time_ms % 2500) / 2500. * 512 / 50);
  return strip.Color(rA * n, gA * n, bA * n);
}

uint32_t Show_Bounce_Up_And_Down_Rainbow(long time_ms, size_t light_index) {
  uint8_t on_light = abs((LED_COUNT - 1) - (time_ms / 120) % (LED_COUNT * 2 - 1));
  if (light_index == on_light) {
    return GetWheelColor((time_ms / 25 + light_index * 2) % (256));
  } else {
    return strip.Color(0, 0, 0);
  }
}

uint32_t Show_Rainbow_Cycle(long time_ms, size_t light_index) {
  return GetWheelColor((time_ms / 25 + light_index * 2) % (256));
}

////////////

uint32_t* get_grad_colors(uint32_t colors[], size_t size) {
  size_t count = sizeof(colors) / sizeof(colors[0]);
  Serial.println("count: " + String(count));
  Serial.println("size: " + String(size));
  uint32_t* ptr1 = (uint32_t*) malloc (size * sizeof(uint32_t));

  uint8_t percent;
  for (size_t i = 0; i < size; ++i) {
    percent = (i * 255 / size * 255) / 255;
    uint8_t color_index2 = (percent * (count - 2) * 255) / 255;
    Serial.println("i: " + String(i) + ", color_index2: " + String(color_index2));
    uint8_t color_index = 0;//(percent * (count - 2) * 255) / 255;
    ptr1[i] = interpolate(colors[color_index], colors[color_index + 1], percent);
  }
  return ptr1;
}

uint32_t GRADIENT_LOOKUP[3] = {
  strip.Color(255, 0, 0),
  strip.Color(255, 0, 255),
  strip.Color(0, 0, 255),
};
uint32_t *GRADIENT;

uint32_t Setup_Shows() {
  GRADIENT = get_grad_colors(GRADIENT_LOOKUP, LED_COUNT); 
}


uint32_t Show_Pulse(long time_ms, size_t light_index) {
  return strip.Color(0, 0, 255);
  // return GRADIENT[light_index];
}
