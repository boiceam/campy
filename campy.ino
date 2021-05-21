// Campy LED Stakes
// Arduino code designed for Seeed Studio Seeeduino XIAO Module
// Controls 20 NeoPixel LEDs (10 per side)

#include <Adafruit_SleepyDog.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN          0
#define LED_COUNT_ALL    20
#define LED_COUNT        10
#define LED_BRIGHTNESS   50

// Low power mode put the device to sleep between LED updates to conserve energy
// When in low power mode programming mode must be entered using the boot pads on the device
// Recommend only enabling low power mode once development is complete
#define LOW_POWER        0

// Define the update period of the display
#define LOOP_DELAY_MS    33

#define SHOW_ROTATION_DURATION     5000 // ms

// Declare the NeoPixel strip object
Adafruit_NeoPixel strip(LED_COUNT_ALL, LED_PIN, NEO_GRB + NEO_KHZ800);

uint16_t loop_count = 0;
unsigned long last_pattern_start = 0;

// Declare shows

uint32_t Show_Bounce_Up_And_Down(long time_ms, size_t light_index) {
  uint8_t on_light = abs((LED_COUNT - 1) - (time_ms / 65) % (LED_COUNT * 2 - 1));
  if (light_index == on_light) {
    return strip.Color(255, 255, 255);
  } else {
    return strip.Color(0, 0, 0);
  }
}

uint32_t Show_A(long current_time_ms, size_t light_index) {
  return GetWheelColor(light_index);
}

// ----------

uint32_t (*SHOWS[]) (long current_time_ms, size_t light_index) = {
  Show_A,
  Show_Bounce_Up_And_Down,
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

  unsigned long show_index = (current_time_ms / SHOW_ROTATION_DURATION) % SHOW_COUNT;

  // Update the color of the pixels in the buffer
  for (size_t i = 0; i < strip.numPixels(); i++) {
    // Show the same show on both sides, so pass in the mirrored version if i to the show
    uint32_t light_index = i % LED_COUNT;
    if (i >= LED_COUNT) {
      light_index = LED_COUNT - light_index - 1;
    }
    uint32_t color = (*SHOWS[show_index])(current_time_ms, light_index);
    strip.setPixelColor(i, color);
  }

  // send the color update to the LEDs
  strip.show();

  // manage our loop counter used for some patterns
  // delay between updates
  #if LOW_POWER
    Watchdog.sleep(LOOP_DELAY_MS);
  #else
    delay(LOOP_DELAY_MS);
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
