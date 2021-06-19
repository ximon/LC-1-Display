#include <Adafruit_SSD1306.h>

#ifndef _Circular_Gauge_H_
#define _Circular_Gauge_H_

#define GAUGE_TYPE_ARC 0
#define GAUGE_TYPE_BAR 1
#define GAUGE_TYPE_NUM 2
#define GAUGE_TYPE_TXT 3

class Circular_Gauge {
    public:
        Circular_Gauge();

        void begin();
        void setupGauge(int type, int min, int max, const char* label);
        void setGaugeValue(float value);
        void setGaugeValue(const char* value);

        void print(uint16_t x, uint16_t y, uint8_t size, uint16_t color, const char* value);

    private:
        void drawGaugeBackground();

        void drawArcGaugeBackground();
        void drawArcGaugeData(float);

        void drawBarGaugeBackground();
        void drawBarGaugeData(float);

        void drawNumGaugeBackground();
        void drawNumGaugeData(float);

        void drawTxtGaugeBackground();
        void drawTxtGaugeData(const char*);
};

#endif
