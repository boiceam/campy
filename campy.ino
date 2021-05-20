// Campy LED Stakes
// Arduino code designed for Seeed Studio Seeeduino XIAO Module
// Controls 20 NeoPixel LEDs (10 per side)

#include <Adafruit_SleepyDog.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN          0
#define LED_COUNT        20
#define LED_BRIGHTNESS   50

// Low power mode put the device to sleep between LED updates to conserve energy
// When in low power mode programming mode must be entered using the boot pads on the device
// Recommend only enabling low power mode once development is complete
#define LOW_POWER        0

// Define the update period of the display
#define LOOP_DELAY_MS    100

#define MAX_PATTERN      2
#define PATTERN_TIME     5000 // ms

// Declare the NeoPixel strip object
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint16_t loop_count = 0;
uint8_t pattern = 0;
unsigned long last_pattern_start = 0;

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
  pattern = 0;
}

void loop() {
  unsigned long active_counter = 0;

  // Get number of milliseconds passed since the Arduino board began running the current program.
  // This number will overflow after approximately 50 days.
  unsigned long current_time_ms = millis();

  // move through patterns based on time
  if (current_time_ms > (last_pattern_start + PATTERN_TIME))
  {
    pattern++;
    if (pattern > MAX_PATTERN)
    {
      pattern = 0;
    }
    last_pattern_start = current_time_ms;
  }

  // update the color of the pixels in the buffer
  for (size_t i = 0; i < strip.numPixels(); i++) {
    if (pattern == 0) {
      strip.setPixelColor(i, GetWheelColor(((i * 256 / strip.numPixels()) + loop_count) & 255));
    } else if (pattern == 1) {
      strip.setPixelColor(i, strip.Color(loop_count, 0, 0));
    } else if (pattern == 2) {
      strip.setPixelColor(i, GetWheelColor(loop_count));
    }
  }

  // send the color update to the LEDs
  strip.show();

  // manage our loop counter used for some patterns
  loop_count++;
  if (loop_count == 256) {
    loop_count = 0;
  }

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
