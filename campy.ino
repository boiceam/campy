// Campy LED Stakes
// Arduino code designed for Seeed Studio Seeeduino XIAO Module
// Controls 20 NeoPixel LEDs (10 per side)

#include <Adafruit_SleepyDog.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN          0
#define LED_COUNT_ALL    20
#define LED_COUNT        10
#define LED_BRIGHTNESS   10
#define PI        3.14

// Low power mode put the device to sleep between LED updates to conserve energy
// When in low power mode programming mode must be entered using the boot pads on the device
// Recommend only enabling low power mode once development is complete
#define LOW_POWER        0

// Define the update period of the display
#define LOOP_DELAY_MS    33

#define SHOW_ROTATION_DURATION     12000 // ms
#define SHOW_TRANSITION_DURATION     1000 // ms

//
// Sleep schedule
//
#define SLEEP_DELAY_MS          (10 * 1000) // ms
#define SLEEP_ON_DURATION       (10 * 60 * 60 * 1000) // ms
#define SLEEP_DAY_DURATION      (24 * 60 * 60 * 1000) // ms


// Declare the NeoPixel strip object
Adafruit_NeoPixel strip(LED_COUNT_ALL, LED_PIN, NEO_GRB + NEO_KHZ800);

uint16_t loop_count = 0;
unsigned long last_pattern_start = 0;

// Declare shows

uint32_t Show_Rainbow_Bouncing_Dot(long time_ms, size_t light_index) {
  float colorT = (float)(time_ms % 30000) / 30000;
  float t = (float)(time_ms % 2500) / 2500;
  float t2 = easeInOutQuint(t);

  float on_light = abs(t2 * (LED_COUNT + 1 ) * 2 - (LED_COUNT + 1));
  float dist_from_on_light = ((float)light_index - on_light);

  if (t >= 0.5) dist_from_on_light *= -1;
  if (dist_from_on_light < 0) return strip.Color(0, 0, 0);

  uint32_t color = GetWheelColor(colorT * 255);
  uint16_t rA = (color >> 16) & 0x000000FF;
  uint16_t gA = (color >> 8) & 0x000000FF;
  uint16_t bA = (color >> 0) & 0x000000FF;

  float val = min(1, max(1 - dist_from_on_light * 0.4, 0));
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

uint32_t Show_Pulse(long time_ms, size_t light_index) {
  uint8_t n = abs(256 - (time_ms % 2500) / 2500. * 512);
  return strip.Color(n, 0, 0);
}

uint32_t Show_Red(long time_ms, size_t light_index) {
  return strip.Color(255, 0, 0);
}

uint32_t Show_Blue(long time_ms, size_t light_index) {
  return strip.Color(0, 0, 255);
}

uint32_t Show_White(long time_ms, size_t light_index) {
  return strip.Color(255, 255, 255);
}

// ----------

uint32_t (*SHOWS[]) (long time_ms, size_t light_index) = {
  Show_Rainbow_Bouncing_Dot,
  // Show_Pulse,
  // Show_Rainbow_Cycle,
  // Show_Bounce_Up_And_Down_With_Flicker,
  // Show_Flicker,
  // Show_Bounce_Up_And_Down_Rainbow,
};
size_t SHOW_COUNT = sizeof(SHOWS) / sizeof(SHOWS[0]);


void setup() {
  // Turn off the built-in status LED to save power
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Connect the USB serial port for debuging
  USBDevice.attach();
  Serial.begin(115200);
  Serial.println("Starting Campy...");

  // Setup the LED strip and clear the color buffer initially
  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.clear();
  strip.show();

  last_pattern_start = millis();
}

void loop() {
  // Get number of milliseconds passed since the Arduino board began running the current program.
  // This number will overflow after approximately 50 days.
  unsigned long current_time_ms = millis();

  // Run the light show for SLEEP_ON_DURATION, then sleep until SLEEP_DAY_DURATION
  unsigned long day_time_ms = current_time_ms % SLEEP_DAY_DURATION;
  bool show_should_run = day_time_ms <= SLEEP_ON_DURATION;
  
  if (!show_should_run) {
    // Serial.println("Campy is sleeping until the next day");
    strip.clear();
    strip.show();
    sleepFor(SLEEP_DELAY_MS);
    return;
  }

  unsigned long show_index = (current_time_ms / SHOW_ROTATION_DURATION) % SHOW_COUNT;
  float show_transition_fraction = min(1.,
    max(0, ((int32_t)(current_time_ms % SHOW_ROTATION_DURATION) - (SHOW_ROTATION_DURATION - SHOW_TRANSITION_DURATION))) / (float)SHOW_TRANSITION_DURATION
  );
  // Serial.println("show_index: " + String(show_index) + ", fraction..." + String(show_transition_fraction));

  // Update the color of the pixels in the buffer
  for (size_t i = 0; i < strip.numPixels(); i++) {
    // Show the same show on both sides, so pass in the mirrored version if i to the show
    uint8_t light_index = i % LED_COUNT;
    if (i >= LED_COUNT) {
      light_index = LED_COUNT - light_index - 1;
    }
    uint32_t colorA = (*SHOWS[show_index])(current_time_ms, light_index);
    uint32_t colorB = (*SHOWS[(show_index + 1) % SHOW_COUNT])(current_time_ms, light_index);
    uint32_t colorBlended = interpolate(colorA, colorB, show_transition_fraction * 255);

    strip.setPixelColor(i, colorBlended);
  }

  // send the color update to the LEDs
  strip.show();

  sleepFor(LOOP_DELAY_MS);
}

// delay between updates
void sleepFor(long sleep_time) {
  #if LOW_POWER
    Watchdog.sleep(sleep_time);
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