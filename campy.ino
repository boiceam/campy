// Campy LED Stakes
// Arduino code designed for Seeed Studio Seeeduino XIAO Module
// Controls 20 NeoPixel LEDs (10 per side)

#include <Adafruit_SleepyDog.h>
#include <Adafruit_NeoPixel.h>
#include <RTCZero.h>


#define LED_PIN          0
#define LED_COUNT_ALL    20
#define LED_COUNT        10
#define LED_BRIGHTNESS   100

// Low power mode put the device to sleep between LED updates to conserve energy
// When in low power mode programming mode must be entered using the boot pads on the device
// Recommend only enabling low power mode once development is complete
#define LOW_POWER         1
#define DEBUG             0

#define PROGRAMMING_WINDOW_MS       (20*1000) // ms

// Define the update period of the display
#define LOOP_DELAY_MS     33

#define SHOW_ROTATION_DURATION      (15 * 60 * 1000) // ms
#define SHOW_TRANSITION_DURATION    (5000) // ms

// Sleep schedule
#define SLEEP_DELAY_MS          (4 * 1000) // ms
#define SLEEP_ON_DURATION       (10) // hours

// Constants
#define PI                      3.14

// Declare the NeoPixel strip object
Adafruit_NeoPixel strip(LED_COUNT_ALL, LED_PIN, NEO_GRB + NEO_KHZ800);

RTCZero rtc;

uint16_t loop_count = 0;
unsigned long last_pattern_start = 0;
unsigned long cum_sleep_time = 0;
bool allow_deepsleep = false;
bool usb_attached = false;
unsigned long last_time_log = 0;
unsigned long last_tick_time = 0;
uint8_t active_show_number = 0;


// ===========================================================================
uint32_t Show_Rainbow_Bouncing_Dot(long time_ms, size_t light_index);
uint32_t Show_Flickering_Flame(long time_ms, size_t light_index);
uint32_t Show_Pulse(long time_ms, size_t light_index);
uint32_t Show_Sparkle(long time_ms, size_t light_index);

uint32_t (*SHOWS[]) (long time_ms, size_t light_index) = {
  Show_Rainbow_Bouncing_Dot,
  Show_Pulse,
  Show_Flickering_Flame,
  Show_Sparkle,
};
size_t SHOW_COUNT = sizeof(SHOWS) / sizeof(SHOWS[0]);

// ===========================================================================

void setup() {
  // Initialize the RTC, if power has been lost (POR or BOR) this will set the RTC to 1/1/00 00:00
  // If power was not lost the previous time will be restored (though ~4 seconds may be lost)
  rtc.begin();
  
  // Turn on the built-in status LED intially
  // After the programming window elaspes it will go off to save power
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  LoggingStart();

  // Send the welcome message
  SerialUSB.println("Starting Campy...");
  SerialUSB.flush();

#if DEBUG
  Serial1.begin(115200);
  Serial1.println("Starting Campy...");
  Serial1.print("PM->RCAUSE = ");
  Serial1.println(PM->RCAUSE.reg, HEX);
  LogCurrentTime();
#endif

  // Setup the LED strip and clear the color buffer initially
  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.clear();
  strip.show();

  last_pattern_start = millis();
  Setup_Shows();
}

void loop() {
  // Check if our programming window has elasped and its time to start sleeping
  if (!allow_deepsleep && millis() > PROGRAMMING_WINDOW_MS) {
    digitalWrite(LED_BUILTIN, HIGH);
    allow_deepsleep = true;
    LoggingStart();
    SerialUSB.println("Allowing device to enter deep sleep");
    LoggingStop();
  }

  // Get number of milliseconds passed since the Arduino board began running the current program.
  // This number will overflow after approximately 50 days.
  unsigned long current_time_ms = (millis() + cum_sleep_time);

#if DEBUG
  if (last_time_log + (60 * 1000) < current_time_ms) {
    last_time_log = current_time_ms;
    LogCurrentTime();
  }
#endif

  // Run the light show for SLEEP_ON_DURATION, then sleep until SLEEP_DAY_DURATION
  bool show_should_run = rtc.getHours() < SLEEP_ON_DURATION;
  if (!show_should_run) {
    strip.clear();
    strip.show();
    cum_sleep_time += sleepFor(SLEEP_DELAY_MS, allow_deepsleep);
    return;
  }

  uint8_t next_show_number;
  if (rtc.getMinutes() < 15) {
    next_show_number = 0;
  } else if (rtc.getMinutes() < 30) {
    next_show_number = 1;
  } else if (rtc.getMinutes() < 45) {
    next_show_number = 2;
  } else {
    next_show_number = 3;
  }

  if (next_show_number != active_show_number) {
  #if DEBUG
    Serial1.print("Show change: ");
    Serial1.print(active_show_number);
    Serial1.print(" -> ");
    Serial1.println(next_show_number);
  #endif
    active_show_number = next_show_number;
  }
  
  // Update the color of the pixels in the buffer
  for (size_t i = 0; i < strip.numPixels(); i++) {
    // Show the same show on both sides, so pass in the mirrored version if i to the show
    uint8_t light_index = i % LED_COUNT;
    if (i >= LED_COUNT) {
      light_index = LED_COUNT - light_index - 1;
    }
    uint32_t color = (*SHOWS[active_show_number])(current_time_ms, light_index);
    uint32_t colorCorrected = strip.gamma32(color);
    strip.setPixelColor(i, colorCorrected);
  }

  // send the color update to the LEDs
  strip.show();

  cum_sleep_time += sleepFor(LOOP_DELAY_MS, allow_deepsleep);
}

void LogCurrentTime(){
    char buf[20];
    sprintf(buf, "%02d:%02d:%02d", rtc.getHours(), rtc.getMinutes(), rtc.getSeconds());
    Serial1.println(buf);
}

void LoggingStart() {
  if (!usb_attached) {
    USBDevice.init();
    USBDevice.attach();
    // Delay to allow the connection to establish
    delay(2000);
    usb_attached = true;

    // Connect serial USB connection to allow for debugging
    SerialUSB.begin(115200);
  }
}

void LoggingStop() {
  if (!allow_deepsleep) {
    return;
  }

  if (usb_attached) {
    SerialUSB.flush();
    delay(100);
    USBDevice.detach();
    delay(100);
    usb_attached = false;
  }
}
