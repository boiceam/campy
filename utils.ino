
#include <Adafruit_SleepyDog.h>
#include <Adafruit_NeoPixel.h>

struct Color {
  uint8_t  r;
  uint8_t  g;
  uint8_t  b;
} color;

// delay between updates
void sleepFor(long sleep_time) {
  #if LOW_POWER
    Watchdog.reset();
    cum_sleep_time += sleep(sleep_time);
    Watchdog.disable();
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


int sleep(int maxPeriodMS) {

int actualPeriodMS = Watchdog.enable(maxPeriodMS, true); // true = for sleep

  // Enable standby sleep mode (deepest sleep) and activate.
  // Insights from Atmel ASF library.
#if (SAMD20_SERIES || SAMD21_SERIES)
  // Don't fully power down flash when in sleep
  NVMCTRL->CTRLB.bit.SLEEPPRM = NVMCTRL_CTRLB_SLEEPPRM_DISABLED_Val;
#endif
#if defined(__SAMD51__)
  PM->SLEEPCFG.bit.SLEEPMODE = 0x4; // Standby sleep mode
  while (PM->SLEEPCFG.bit.SLEEPMODE != 0x4)
    ; // Wait for it to take
#else
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  // Due to a hardware bug on the SAMD21, the SysTick interrupts become 
  // active before the flash has powered up from sleep, causing a hard fault.
  // To prevent this the SysTick interrupts are disabled before entering sleep mode.
  SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;  // Disable SysTick interrupts
#endif

  __DSB(); // Data sync to ensure outgoing memory accesses complete
  __WFI(); // Wait for interrupt (places device in sleep mode)

#if (SAMD20_SERIES || SAMD21_SERIES)
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;   // Enable SysTick interrupts
#endif
  
  // Code resumes here on wake (WDT early warning interrupt).
  // Bug: the return value assumes the WDT has run its course;
  // incorrect if the device woke due to an external interrupt.
  // Without an external RTC there's no way to provide a correct
  // sleep period in the latter case...but at the very least,
  // might indicate said condition occurred by returning 0 instead
  // (assuming we can pin down which interrupt caused the wake).

  return actualPeriodMS;
}