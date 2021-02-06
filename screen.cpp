#include "screen.h"

#include <TFT_eSPI.h>

#include "config.h"
#include "data.h"

// defined in library by User_Setup.h:
// ToDo: find a way to define outside the library
// #define ST7735_DRIVER
// #define TFT_WIDTH  160
// #define TFT_HEIGHT 128
// #define TFT_MOSI 23
// #define TFT_SCLK 18
// #define TFT_CS    5
// #define TFT_DC    2
// #define TFT_RST   4

// Note: when thinking about coordinates on the screen, always
// keep in mind the orientation of the axis:
//  0 ------------> X+ (TFT_WIDTH)
//   |
//   |
//   |
//   |
//   V
//   Y+ (TFT_HEIGHT)

static TFT_eSPI tft;
static char buffer[1024];
static int page = 0;

int screen_setup() {
    TFT_eSPI tft = TFT_eSPI();
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
}

void screen_nextPage() {
    page++;
    if (page > 1) page = 0;
    tft.fillScreen(TFT_BLACK);
}

void screen_renderAlarmInfo() {
    if (page != 0) return;

    tft.fillRect(0, 0, 50, 16, TFT_BLACK);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    if (!curConfig.enabled) {
        tft.drawString("--:--", 0, 0, 2);
        return;
    }
    sprintf(buffer, "%02d:%02d", curConfig.hour, curConfig.minute);
    tft.drawString(buffer, 0, 0, 2);
}

void screen_renderTime(tm *time) {
    if (page != 0) return;

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    strftime(buffer, sizeof(buffer), "%a %d", time);
    tft.drawRightString(buffer, 160, 0, 2);

    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    strftime(buffer, sizeof(buffer), "%H:%M", time);
    tft.drawCentreString(buffer, 80, 25, 6);
}

void screen_renderIndoorData(float temp, float hum) {
    if (page != 0) return;

    tft.setTextColor(TFT_RED, TFT_BLACK);
    sprintf(buffer, "%d`   ", (int)round(temp));
    tft.drawString(buffer, 2, 75, 4);
    sprintf(buffer, "   %d%%", (int)round(hum));
    tft.drawRightString(buffer, 158, 75, 4);
}

void screen_renderOutdoorData(float temp, float hum) {
    if (page != 0) return;

    tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
    sprintf(buffer, "%d`   ", (int)round(temp));
    tft.drawString(buffer, 2, 75, 4);
    sprintf(buffer, "   %d%%", (int)round(hum));
    tft.drawRightString(buffer, 158, 75, 4);
}

void screen_renderWeather(const char *weather) {
    if (page != 0) return;

    tft.setTextColor(TFT_WHITE);
    tft.fillRect(0, 110, 160, 18, TFT_BLUE);
    tft.drawCentreString(weather, 80, 110, 2);
}

void screen_renderPressure(float temp, float hum, float pressure) {

    // Note: min and max value ever recorded: 870-1084
    if (page != 1) return;

    tft.fillRect(0, 0, 128, 26, TFT_BLACK);

    tft.setTextColor(TFT_YELLOW);
    sprintf(buffer, "%d` ", (int)round(temp));
    tft.drawString(buffer, 0, 0, 2);

    tft.setTextColor(TFT_GREEN);
    sprintf(buffer, "%d", (int)round(pressure));
    tft.drawCentreString(buffer, 80, 0, 4);

    tft.setTextColor(TFT_YELLOW);
    sprintf(buffer, "%d%%", (int)round(hum));
    tft.drawRightString(buffer, 160, 0, 2);

    int basePress = 1013; // pressure at sea level
    int baseTemp = 18; // average ref temperature
    int baseLine = 78; // a line indicating the base pressure and the base temperature
    int pixelForTempUnit = 1;
    int pixelForPressUnit = 2;

    // reading data circular array from oldest to newest value
    for(int i=0, ci=(data_curPageIndex+1)%16; i<16; i++, ++ci%=16) {
        // compute the y coordinate of the pressure bar (max pressure)
        int yPress = baseLine + (basePress - data_curPage[ci].pressureMax) * pixelForPressUnit;
        // compute the height of the pressure bar so that it touch the base of the screen
        int hPress = 128 - yPress;
        tft.drawRect(i * 10, yPress, 9, hPress, TFT_GREENYELLOW);

        // compute the y coordinate of the temperature bar (max temperature)
        int yTemp = baseLine + (baseTemp - data_curPage[ci].outTempMax) * pixelForTempUnit;
        // compute the height of the temperature bar as the difference between min and max temperature
        int hTemp = (data_curPage[ci].outTempMax - data_curPage[ci].outTempMin) * pixelForTempUnit;

        // if min and max temperature are the same the bar will have a height of two pixel
        // so that it is still visible
        if(hTemp == 0) {
            yTemp += 1;
            hTemp = 1;
        }
        tft.fillRect(i * 10 + 1, yTemp, 7, hTemp, TFT_RED);
    }
    // White line: base pressure and base temperature
    tft.drawLine(0, baseLine, 160, baseLine, TFT_WHITE);
    // Blue line: 0 deg (Celsius)
    tft.drawLine(0, baseLine + baseTemp * pixelForTempUnit, 160, baseLine + baseTemp * pixelForTempUnit, TFT_LIGHTGREY);
}

void screen_showAPInfo(char *ssid, IPAddress ip) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 10);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.printf("WiFi: %s\n", ssid);
    tft.println("IP: ");
    tft.println(ip);
}

void screen_renderIPInfo(IPAddress ip) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 10);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.print("IP: ");
    tft.println(ip);
    delay(3000);
    tft.fillScreen(TFT_BLACK);
}
