#include "data.h"
#include "common.h"

#include <FS.h>
#include <SD.h>
#include <math.h>


// Data will rotate when maximum size is reached (expressed in bytes)
#define MAX_DATA_FILE_SIZE 3072
// Data is collected into samples of the given time (expressed in minutes)
// inside each sample only min and max values are saved
#define DATA_SAMPLE_SIZE 180
// How many entries of sample will be kept on memory
#define NUM_OF_ENTRIES 16

Sample data_curPage[NUM_OF_ENTRIES];
uint8_t data_curPageIndex = 0;

static uint32_t fileIndex = 0;

#define HEADER_SIZE sizeof(fileIndex)
#define PAGE_SIZE sizeof(data_curPage)
#define CUR_SAMPLE data_curPage[data_curPageIndex]

// Initialize data module
bool data_setup() {
    DEBUG_PRINTLN("data_setup")

    // initialize local data to min or max values
    for (int i = 0; i < NUM_OF_ENTRIES; i++) {
        data_curPage[i].indHumMax = INT8_MIN;
        data_curPage[i].indHumMin = INT8_MAX;
        data_curPage[i].indTempMax = INT8_MIN;
        data_curPage[i].indTempMin = INT8_MAX;
        data_curPage[i].outHumMax = INT8_MIN;
        data_curPage[i].outHumMin = INT8_MAX;
        data_curPage[i].outTempMax = INT8_MIN;
        data_curPage[i].outTempMin = INT8_MAX;
        data_curPage[i].pressureMax = INT16_MIN;
        data_curPage[i].pressureMin = INT16_MAX;
    }

    if (!SD.exists("/data.dat")) {
        File file = SD.open("/data.dat", FILE_WRITE);
        fileIndex = sizeof(fileIndex);
        file.write((byte *)&fileIndex, HEADER_SIZE);
        file.close();
        DEBUG_PRINTF("created data.dat; index: %d\n", fileIndex);
        return true;
    }

    File file = SD.open("/data.dat", FILE_READ);
    if (!file) {
        DEBUG_PRINTLN("error reading data.dat");
        return false;
    }

    // the first 4 bytes (int32) contains the position in file
    // to be used for writing the next entry
    file.read((byte*)&fileIndex, HEADER_SIZE);

    size_t byteToRead = min(PAGE_SIZE, file.size()-HEADER_SIZE);
    data_curPageIndex = byteToRead / sizeof(Sample) - 1; // this should be an integer division
    DEBUG_PRINTF("saved data found\n\tfile size: %d\n\tfileIndex:%d\n\tcurPageIndex: %d\n\tbyte to read: %d\n",
        file.size(), fileIndex, data_curPageIndex, byteToRead)
    file.seek(file.size() - byteToRead);
    file.read((byte *)&data_curPage, byteToRead);
    file.close();

    DEBUG_PRINTLN("data_setup complete")
    return true;
}

int sampleFromTime(tm time) {
    return floor((time.tm_hour * 60 + time.tm_min) / DATA_SAMPLE_SIZE);
}

bool inSameSample(tm t) {
    return
        (CUR_SAMPLE.indTempMin == INT8_MAX) // current sample not yet initialized
        ||
        (
            t.tm_yday == CUR_SAMPLE.time.tm_yday
            && t.tm_year == CUR_SAMPLE.time.tm_year
            && sampleFromTime(t) == sampleFromTime(CUR_SAMPLE.time)
        );
}

void saveLatestEntry() {
    DEBUG_PRINTLN("save latest entry")

    File file = SD.open("/data.dat", FILE_WRITE);
    if (!file) {
        DEBUG_PRINTLN("Error saving data.dat");
        return;
    }
    // if maximum file size would be reached by writing
    // then move the index to the beginning
    if(fileIndex + sizeof(Sample) >= MAX_DATA_FILE_SIZE) {
        fileIndex = HEADER_SIZE;
    }
    DEBUG_PRINTF("fileIndex: %d\n", fileIndex)
    file.seek(fileIndex);
    file.write((byte *)&data_curPage[data_curPageIndex], sizeof(Sample));
    fileIndex = file.position();
    file.seek(0);
    file.write((byte *)&fileIndex, HEADER_SIZE);
    file.close();
}

void data_collect(tm time, float pressure, float outTemp, float indTemp, float outHum, float indHum) {
    DEBUG_PRINTLN("begin data_collect")

    if (!inSameSample(time)) {
        saveLatestEntry();
        (++data_curPageIndex)%=NUM_OF_ENTRIES;
    }

    CUR_SAMPLE.time = time;

    CUR_SAMPLE.pressureMax = max(CUR_SAMPLE.pressureMax, (int16_t)round(pressure));
    CUR_SAMPLE.pressureMin = min(CUR_SAMPLE.pressureMin, (int16_t)round(pressure));

    CUR_SAMPLE.outTempMax = max(CUR_SAMPLE.outTempMax, (int8_t)round(outTemp));
    CUR_SAMPLE.outTempMin = min(CUR_SAMPLE.outTempMin, (int8_t)round(outTemp));

    CUR_SAMPLE.indTempMax = max(CUR_SAMPLE.indTempMax, (int8_t)round(indTemp));
    CUR_SAMPLE.indTempMin = min(CUR_SAMPLE.indTempMin, (int8_t)round(indTemp));

    CUR_SAMPLE.outHumMax = max(CUR_SAMPLE.outHumMax, (int8_t)round(outHum));
    CUR_SAMPLE.outHumMin = min(CUR_SAMPLE.outHumMin, (int8_t)round(outHum));

    CUR_SAMPLE.indHumMax = max(CUR_SAMPLE.indHumMax, (int8_t)round(indHum));
    CUR_SAMPLE.indHumMin = min(CUR_SAMPLE.indHumMin, (int8_t)round(indHum));

    DEBUG_PRINTLN("end data_collect")
}

void data_erase() {
    SD.remove("/data.dat");
}

// Get the n-tn page of data (unordered, use the timestamp to sort)
Sample* data_getPage(int n) {
    if(n == 0) {
        return data_curPage;
    }

    File file = SD.open("/data.dat", FILE_READ);
    int pagesInFile = (file.size() - HEADER_SIZE) / PAGE_SIZE;

    DEBUG_PRINTF("getPage:\n\tfileSize: %d\n\theader size: %d\n\tpage size: %d\n\tpagesInFile: %d\n",
        file.size(), HEADER_SIZE, PAGE_SIZE, pagesInFile);


    // ToDo: make so that even the last partial page can be fetched
    if(n >= pagesInFile) {
        return NULL;
    }

    // compute the offset of the n-th page considering that the file is circular
    int pageOffset = (fileIndex - HEADER_SIZE) / PAGE_SIZE - n;
    DEBUG_PRINTF("pageOffset: %d\n", pageOffset);
    if(pageOffset < 0) {
        pageOffset = pagesInFile + pageOffset;
    }
    file.seek(HEADER_SIZE + pageOffset*PAGE_SIZE);
    Sample *page = (Sample*)malloc(PAGE_SIZE);
    file.read((byte*)page, PAGE_SIZE);
    return page;
}