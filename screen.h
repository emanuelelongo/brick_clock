#include <Arduino.h>
#include <time.h>

int screen_setup();
void screen_nextPage();
void screen_renderAlarmInfo();
void screen_renderTime(tm *time);
void screen_renderIndoorData(float temp, float hum);
void screen_renderOutdoorData(float temp, float hum);
void screen_renderWeather(const char *weather);
void screen_renderPressure(float temp, float hum, float pressure);
void screen_showAPInfo(char *ssid, IPAddress ip);
void screen_renderIPInfo(IPAddress ip);