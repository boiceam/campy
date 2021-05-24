
// Declare shows

#include <Adafruit_SleepyDog.h>
#include <Adafruit_NeoPixel.h>


size_t GRADIENT_SIZE = LED_COUNT * 2;
uint32_t RED_BLUE_COLORS[] = {
  strip.Color(180, 0, 50),
  strip.Color(60, 20, 205),
  // strip.Color(100, 90, 205),
  strip.Color(180, 0, 50),
};
size_t RED_BLUE_COLORS_SIZE = sizeof(RED_BLUE_COLORS) / sizeof(RED_BLUE_COLORS[0]);
uint32_t *GRADIENT_RED_BLUE;

uint16_t sparkle_light_state[LED_COUNT];
uint16_t sparkle_light_color[LED_COUNT];

uint32_t Setup_Shows() {
  GRADIENT_RED_BLUE = get_grad_colors(RED_BLUE_COLORS, RED_BLUE_COLORS_SIZE, GRADIENT_SIZE);

  for (size_t i = 0; i < LED_COUNT; ++i) {
    sparkle_light_state[i] = 255 * 255;
    sparkle_light_color[i] = 0;
  }
}
/////////////////////


uint32_t Show_Rainbow_Bouncing_Dot(long time_ms, size_t light_index);
uint32_t Show_Rainbow_Cycle(long time_ms, size_t light_index);
uint32_t Show_Pulse(long time_ms, size_t light_index);

////////////////

uint32_t Show_Rainbow_Bouncing_Dot(long time_ms, size_t light_index) {
  float colorT = (float)(time_ms % 30000) / 30000;
  float t = (float)(time_ms % 4000) / 4000;
  float t2 = easeInOutQuint(t);

  float on_light = abs(t2 * (LED_COUNT + 2) * 2 - (LED_COUNT + 2));
  float dist_from_on_light = ((float)light_index - on_light);

  if (t >= 0.5) dist_from_on_light *= -1;
  if (dist_from_on_light < 0) return strip.Color(0, 0, 0);

  uint32_t color = GetWheelColor(colorT * 255);
  uint16_t r = COLOR_SLICE_R(color);
  uint16_t g = COLOR_SLICE_G(color);
  uint16_t b = COLOR_SLICE_B(color);

  float n = min(1, max(1 - dist_from_on_light * 0.25, 0));
  return strip.Color(r * n, g * n, b * n);
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


uint32_t Show_Pulse(long time_ms, size_t light_index) {
  uint16_t shift = time_ms / 2500;
  uint32_t color = GRADIENT_RED_BLUE[((light_index + shift) % (GRADIENT_SIZE))];
  uint16_t r = COLOR_SLICE_R(color);
  uint16_t g = COLOR_SLICE_G(color);
  uint16_t b = COLOR_SLICE_B(color);
  float n = 1 - abs(2 * (time_ms % 2500) / 2500. - 1);
  return strip.Color(r * n, g * n, b * n);
}



long last_light_time_ms;
uint32_t Show_Sparkle(long time_ms, size_t light_index) {
  for (size_t i = 0; i < LED_COUNT; ++i) {
    sparkle_light_state[i] = max(((int32_t)sparkle_light_state[i]) - 40, 0);
  }

  if (random(10000) <= 11 || (time_ms - last_light_time_ms) > 750) {
    size_t light_to_enable = random(LED_COUNT);
    if (sparkle_light_state[light_to_enable] == 0) {
      sparkle_light_state[light_to_enable] = 255 * 255;
      sparkle_light_color[light_to_enable] = random(1500, 10000);
    }
    last_light_time_ms = time_ms;
  }

  return strip.ColorHSV(sparkle_light_color[light_index], 255, min(max(sparkle_light_state[light_index] / 255, 0), 255));
}


////////////

uint32_t* get_grad_colors(uint32_t colors[], size_t colors_count, size_t size) {
  uint32_t* ptr1 = (uint32_t*) malloc (size * sizeof(uint32_t));
  uint8_t whole_percent, gradient_percent, color_index;
  for (size_t i = 0; i < size; ++i) {
    whole_percent = (i * 255 / size * 255) / 255;
    gradient_percent = (whole_percent * (colors_count) * 255) / (255);
    color_index = (whole_percent * (colors_count) * 255) / (255*255);
    ptr1[i] = interpolate(colors[color_index], colors[(color_index + 1) % colors_count], gradient_percent);
    // Serial.println(
    // "i: " + String(i) + 
    // ", color_index: " + String(color_index) + 
    // ", whole_percent: " + String(whole_percent) + 
    // ", gradient_percent: " + String(gradient_percent)
    // );
  }
  return ptr1;
}
