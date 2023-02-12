#include <ESP8266WiFi.h>
#include <WifiUDP.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

/**
 * Obtener la hora de internet. 
*/
String getTime(bool internet);

String getDate(bool internet);

void setUpTime();