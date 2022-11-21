
//================= วาง TEMPLATE_ID จาก Web blynk =============//

#define BLYNK_TEMPLATE_ID "TMPLD7tqaAjB"
#define BLYNK_DEVICE_NAME "TEST"

//============================================================//

#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

#include <Servo.h>
#include "BlynkEdgent.h"
#include <TimeLib.h>
BlynkTimer timer;

Servo myservo;
int work = 0;
int Value_Fix = 3; // จำนวนครั้งให้อาหาร
int cc = 0;
long timer_;
long timer_start_set[2] = {0xFFFF, 0xFFFF};
long timer_stop_set[2] = {0xFFFF, 0xFFFF};
unsigned char weekday_set[2];
long rtc_sec;
unsigned char day_of_week;
bool update_blynk_status[2];

//============================= ฟังก์ชั่นให้อาหาร ============================//
BLYNK_WRITE(V0)
{
  int val = param.asInt();
  if (val == 1) {
    work = 1;
    Serial.println("ON");
  }
}

//============================ timer to blynk ===========================//
BLYNK_WRITE(V1)
{
  unsigned char week_day;
  TimeInputParam t(param);

  if (t.hasStartTime() && t.hasStopTime() )
  {
    timer_start_set[0] = (t.getStartHour() * 60 * 60) + (t.getStartMinute() * 60) + t.getStartSecond();
    timer_stop_set[0] = (t.getStopHour() * 60 * 60) + (t.getStopMinute() * 60) + t.getStopSecond();

    Serial.println(String("Start Time: ") +
                   t.getStartHour() + ":" +
                   t.getStartMinute() + ":" +
                   t.getStartSecond());

    Serial.println(String("Stop Time: ") +
                   t.getStopHour() + ":" +
                   t.getStopMinute() + ":" +
                   t.getStopSecond());

    for (int i = 1; i <= 7; i++)
    {
      if (t.isWeekdaySelected(i))
      {
        week_day |= (0x01 << (i - 1));
        Serial.println(String("Day ") + i + " is selected");
      }
      else
      {
        week_day &= (~(0x01 << (i - 1)));
      }
    }
    weekday_set[0] = week_day;
  }
  else
  {
    timer_start_set[0] = 0xFFFF;
    timer_stop_set[0] = 0xFFFF;
  }
}
BLYNK_WRITE(InternalPinRTC)
{
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  unsigned long blynkTime = param.asLong();

  if (blynkTime >= DEFAULT_TIME)
  {
    setTime(blynkTime);

    day_of_week = weekday();

    if ( day_of_week == 1 )
      day_of_week = 7;
    else
      day_of_week -= 1;

    rtc_sec = (hour() * 60 * 60) + (minute() * 60) + second();
    Serial.println(blynkTime);
    Serial.println(String("RTC Server: ") + hour() + ":" + minute() + ":" + second());
    Serial.println(String("Day of Week: ") + weekday());
  }
}
BLYNK_CONNECTED()
{
  Blynk.sendInternal("rtc", "sync");
}
void checkTime()
{
  Blynk.sendInternal("rtc", "sync");
}
void Logic_state()
{
  bool time_set_overflow;
  for (int i = 0; i < 2; i++)
  {
    time_set_overflow = 0;
    if ( timer_start_set[i] != 0xFFFF && timer_stop_set[i] != 0xFFFF) {
      if ( timer_stop_set[i] < timer_start_set[i] ) {
        time_set_overflow = 1;
      }
      if ((((time_set_overflow == 0 && (rtc_sec >= timer_start_set[i]) && (rtc_sec < timer_stop_set[i])) ||
            (time_set_overflow  && ((rtc_sec >= timer_start_set[i]) || (rtc_sec < timer_stop_set[i])))) &&
           (weekday_set[i] == 0x00 || (weekday_set[i] & (0x01 << (day_of_week - 1) )))) ) {
        work = 1;
      }
      else {
        //        work = 0;
      }
    }
  }
}

// #########################################################################################################
void setup()
{
  Serial.begin(115200);
  myservo.attach(D4);
  myservo.write(0); 
  BlynkEdgent.begin();
  timer.setInterval(1000L, checkTime);

}

// #########################################################################################################
void loop()
{
  BlynkEdgent.run();
  timer.run();
  Logic_state();
  Serial.println(work);
  if (work == 0) {
    myservo.write(0);
  } else if (work == 1) {
    if (cc < Value_Fix) {
      Serial.println("Open > ");
      myservo.write(80);
      delay(2000);
      myservo.write(0);
      delay(1000);
      cc++;
    } else if (cc >= Value_Fix) {
      Serial.println("END");
      work = 2;
    }
  } else if (work == 2) {
    delay(5000);
    cc = 0;
    work = 0;
  }
}
