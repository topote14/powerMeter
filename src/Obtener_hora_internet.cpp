#include "InternetTime.h"

#include <WiFiUdp.h>

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Week Days
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Month names
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

int lastHour = 0;
int lastMinute = 0;
int lastSecond = 0;

void setUpTime()
{
  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(-10800);
}

String getTime(bool internet)
{
  if (internet)
  {
    timeClient.update();
    String formattedTime = timeClient.getFormattedTime();
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
    int currentSecond = timeClient.getSeconds();
    lastHour = currentHour;
    lastMinute = currentMinute;
    lastSecond = currentSecond;
    String horaActual = (String)currentHour + "-" + (String)currentMinute + "-" + (String)currentSecond;
    return (horaActual);
  }
  else
  {
    lastSecond += 10;
    if (lastSecond >= 60)
    {
      lastMinute += lastSecond / 60;
      lastSecond = lastSecond % 60;
    }
    if (lastMinute >= 60)
    {
      lastHour += lastMinute / 60;
      lastMinute = lastMinute % 60;
    }
    if (lastHour >= 24)
    {
      lastHour = lastHour % 24;
    }
    String horaActual = (String)lastHour + "-" + (String)lastMinute + "-" + (String)lastSecond;
    return (horaActual);
  }
}

String getDate(bool internet)
{
  static time_t lastKnownEpoch = 0;
  if (internet)
  {
    timeClient.update();
    time_t epochTime = timeClient.getEpochTime();
    lastKnownEpoch = epochTime;
    String weekDay = weekDays[timeClient.getDay()];
    // Get a time structure
    struct tm *ptm = gmtime((time_t *)&epochTime);
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon + 1;
    String currentMonthName = months[currentMonth - 1];
    int currentYear = ptm->tm_year + 1900;
    // Print complete date:
    String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);


    return (currentDate);
  }
  else
  {
    // calculate the time since last update (update every 10 seconds)
    lastKnownEpoch += 10;
    struct tm *ptm = gmtime((time_t *)&lastKnownEpoch); 
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon + 1;
    int currentYear = ptm->tm_year + 1900;
    String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
    return currentDate;
  }
}