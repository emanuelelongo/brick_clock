#include <Arduino.h>
#include <time.h>

typedef struct {
    tm time;
    int16_t pressureMin;
    int16_t pressureMax;
    int8_t outTempMin;
    int8_t outTempMax;
    int8_t indTempMin;
    int8_t indTempMax;
    int8_t outHumMin;
    int8_t outHumMax;
    int8_t indHumMin;
    int8_t indHumMax;
} Sample;

extern Sample data_curPage[16];
extern uint8_t data_curPageIndex;

bool data_setup();
void data_collect(tm time, float pressure, float outTemp, float indTemp, float outHum, float indHum);
Sample* data_getPage(int n);
void data_erase();