
#include <RTCZero.h>

RTCZero rtc;

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 00;
const byte hours = 12;

/* Change these values to set the current initial date */
const byte day = 1;
const byte month = 1;
const byte year = 21;

void handleRtcAlarm()
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void intialize_rtc() {
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);
  rtc.setAlarmTime(hours, minutes, seconds+10);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  rtc.attachInterrupt(handleRtcAlarm);
  
  // Turn off the built-in status LED to save power
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void rtc_test_loop(){
  rtc.setAlarmTime(hours, minutes, (rtc.getSeconds()+10)%60);
  rtc.standbyMode(); 
}
