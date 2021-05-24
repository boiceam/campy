// Campy LED Stakes
// Arduino code designed for Seeed Studio Seeeduino XIAO Module
// Controls 20 NeoPixel LEDs (10 per side)

#include <Adafruit_SleepyDog.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN          0
#define LED_COUNT_ALL    20
#define LED_COUNT        10
#define LED_BRIGHTNESS   100

#define COLOR_SLICE_R(color) ((color >> 16) & 0x000000FF)
#define COLOR_SLICE_G(color) ((color >> 8) & 0x000000FF)
#define COLOR_SLICE_B(color) ((color >> 0) & 0x000000FF)

// Low power mode put the device to sleep between LED updates to conserve energy
// When in low power mode programming mode must be entered using the boot pads on the device
// Recommend only enabling low power mode once development is complete
#define LOW_POWER        1

// Define the update period of the display
#define LOOP_DELAY_MS    33

#define SHOW_ROTATION_DURATION     (10 * 60 * 1000) // ms
#define SHOW_TRANSITION_DURATION    2000 // ms

//
// Sleep schedule
//
#define SLEEP_DELAY_MS          (12 * 1000) // ms
#define SLEEP_ON_DURATION       (10 * 60 * 60 * 1000) // ms
#define SLEEP_DAY_DURATION      (24 * 60 * 60 * 1000) // ms
// #define SLEEP_ON_DURATION       (60 * 1000) // ms        DEBUG!
// #define SLEEP_DAY_DURATION      (144 * 1000) // ms       DEBUG!

// Constants
#define PI        3.14

struct Color {
  uint8_t  r;
  uint8_t  g;
  uint8_t  b;
} color;

// Declare the NeoPixel strip object
Adafruit_NeoPixel strip(LED_COUNT_ALL, LED_PIN, NEO_GRB + NEO_KHZ800);

uint16_t loop_count = 0;
unsigned long last_pattern_start = 0;
unsigned long cum_sleep_time = 0;

// ===========================================================================
uint32_t Show_Rainbow_Bouncing_Dot(long time_ms, size_t light_index);
uint32_t Show_Sparkle(long time_ms, size_t light_index);
uint32_t Show_Pulse(long time_ms, size_t light_index);

uint32_t (*SHOWS[]) (long time_ms, size_t light_index) = {
  Show_Rainbow_Bouncing_Dot,
  Show_Pulse,
  Show_Sparkle,
};
size_t SHOW_COUNT = sizeof(SHOWS) / sizeof(SHOWS[0]);

// ===========================================================================


void setup() {
  // Turn off the built-in status LED to save power
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Connect the USB serial port for debuging
  USBDevice.attach();
  delay(1000);
  Serial.begin(9600);
  delay(1000);

  Serial.println("Starting Campy...");
  Serial.flush();

  // Setup the LED strip and clear the color buffer initially
  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.clear();
  strip.show();

  last_pattern_start = millis();
  Setup_Shows();

  Serial.println("Setup complete!");
  Serial.println("==========================");
}

void loop() {
  // Get number of milliseconds passed since the Arduino board began running the current program.
  // This number will overflow after approximately 50 days.
  unsigned long current_time_ms = millis() + cum_sleep_time;

  // Run the light show for SLEEP_ON_DURATION, then sleep until SLEEP_DAY_DURATION
  unsigned long day_time_ms = current_time_ms % SLEEP_DAY_DURATION;
  bool show_should_run = day_time_ms <= SLEEP_ON_DURATION;
  
  if (!show_should_run) {
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
    uint32_t colorCorrected = strip.gamma32(colorBlended);
    strip.setPixelColor(i, colorCorrected);
  }

  // send the color update to the LEDs
  strip.show();

  sleepFor(LOOP_DELAY_MS);
}
