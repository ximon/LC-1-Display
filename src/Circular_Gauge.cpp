#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Circular_Gauge.h"

#define WIDTH 128
#define HEIGHT 64

Adafruit_SSD1306 oled(WIDTH, HEIGHT, &Wire, -1, 800000UL, 100000UL);

#define PI2 2 * PI

float startAngleD, startAngle;
float endAngleD, endAngle;
int centerX, centerY, radius;

int gaugeMin, gaugeMax;
int gaugeType;
const char * gaugeLabel;

Circular_Gauge::Circular_Gauge() {
    
    float centerD = 270; //Angle where the center of the gauge will be
    float widthD = 40; //Angle that the gauge will be wide
    
    startAngleD = centerD - widthD;
    endAngleD   = centerD + widthD;
  
    centerX    = 63;  //    Center of arc X (pixels)
    centerY    = 100; //    Center of arc Y (pixels)
    radius     = 65;  //    Radious of arc (pixels)
  
    startAngle = startAngleD / 360 * PI2;
    endAngle   = endAngleD   / 360 * PI2;
}

void Circular_Gauge::begin()
{
    oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    oled.clearDisplay();
    oled.display();
}


  
void Circular_Gauge::print(uint16_t x, uint16_t y, uint8_t size, uint16_t color, const char* value)
{
    oled.setTextColor(color);
    oled.setTextSize(size);
    oled.setCursor(x, y);
    oled.print(value);
    oled.display();
}

void Circular_Gauge::setupGauge(int type, int min, int max, const char* label)
{
    gaugeType = type;
    gaugeMin = min;
    gaugeMax = max;
    gaugeLabel = label;

    drawGaugeBackground();
}
  
float scale(float inScaleMin, float inScaleMax, float outScaleMin, float outScaleMax, float value){
    return ((value - inScaleMin) / (inScaleMax - inScaleMin) * (outScaleMax-outScaleMin)) + outScaleMin;
} 
  
float angleToXD(float centerX, float radius, float angleD) {
    float angle = (angleD / 360) * PI2;
    return centerX + radius * cos(angle); // Calculate arc point (X)
}

float angleToYD(float centerY, float radius, float angleD) {
    float angle = (angleD / 360) * (PI2);
    return centerY + radius * sin(angle); // Calculate arc point (Y)
}

void drawArc(float startAngle, float endAngle, float segments, int centerX, int centerY, int radius) {
    float resolution = (endAngle-startAngle)/segments; // Calculates steps in arc based on segments
    float x = centerX + radius * cos(startAngle); // Calculate start point of arc (X)
    float y = centerY + radius * sin(startAngle); // Calculate start point of arc (Y)
    oled.writePixel(x, y, WHITE); // Place starting point of arc
  
    for (float angle = startAngle; angle < endAngle; angle += resolution) { // Sweep arc
        x = centerX + radius * cos(angle); // Calculate arc point (X)
        y = centerY + radius * sin(angle); // Calculate arc point (Y)
        oled.writePixel(x, y, WHITE);
    }
}

void drawNeedle(float angle, float startAngle, float endAngle, float centerX, float centerY, int radius, int color){
    int bottomRadius = radius + 1;
    int topRadius = radius + 27;
    
    float leftX = angleToXD(centerX, bottomRadius, angle - 5);
    float leftY = angleToYD(centerY, bottomRadius, angle - 5);
  
    float rightX = angleToXD(centerX, bottomRadius, angle + 5);
    float rightY = angleToYD(centerY, bottomRadius, angle + 5);
  
    float topX = angleToXD(centerX, topRadius, angle);
    float topY = angleToYD(centerY, topRadius, angle);
  
    oled.fillTriangle(leftX, leftY, topX, topY, rightX, rightY, color);
}

void drawGaugeLines(float startAngle, float endAngle, float centerX, float centerY, int radius){
    drawArc(startAngle, endAngle, 150, centerX, centerY, radius + 30);
    drawArc(startAngle - 0.1, endAngle + 0.1, 110, centerX, centerY, radius - 1);
    //drawArc(startAngle - 0.1, endAngle + 0.1, 110, centerX, centerY, radius - 4);
}

void Circular_Gauge::drawGaugeBackground() {
    oled.clearDisplay();

    switch (gaugeType) {
        case GAUGE_TYPE_ARC:
            drawArcGaugeBackground();
            break;
        case GAUGE_TYPE_BAR:
            drawBarGaugeBackground();
            break;
        case GAUGE_TYPE_NUM:
            drawNumGaugeBackground();
            break;
        case GAUGE_TYPE_TXT:
            drawTxtGaugeBackground();
            break;
    }
}

void Circular_Gauge::setGaugeValue(float value) {
    switch (gaugeType) {
        case GAUGE_TYPE_ARC:
            drawArcGaugeData(value);
            break;
        case GAUGE_TYPE_BAR:
            drawBarGaugeData(value);
            break;
        case GAUGE_TYPE_NUM:
            drawNumGaugeData(value);
            break;
    }
}

void Circular_Gauge::setGaugeValue(const char* value) {
    switch (gaugeType) {
        case GAUGE_TYPE_TXT:
            drawTxtGaugeData(value);
            break;
    }
}

void drawLabel(int x, int y, const char* label) {
    oled.setTextColor(INVERSE);
    oled.setTextSize(2);
    oled.setCursor(x,y);
    oled.print(label);
}

#define CHAR_WIDTH 10
#define SPACE_BETWEEN 2
int stringWidth(const char* label) {
    int strLen = strlen(label);
    return  (strLen * (CHAR_WIDTH + SPACE_BETWEEN)) - SPACE_BETWEEN;
}

int calcLabelX(const char* label) {
    int strWidth = stringWidth(label);   
    int halfScreen = WIDTH / 2;
    int halfString = strWidth / 2;

    return halfScreen - halfString;
}

void drawValue(const char* value) {
    int labelX = calcLabelX(value);

    oled.setTextColor(INVERSE);
    oled.setTextSize(2);
    oled.setCursor(labelX, 48);
    oled.print(value);
}

void drawValue(float value) {
    char strValue[6];
    dtostrf( value, 5, 2, strValue);       // float, width, precision, buffer
    drawValue(strValue);
}



void Circular_Gauge::drawArcGaugeBackground() {
    int x = calcLabelX(gaugeLabel);
    drawLabel(x, 15, gaugeLabel);
    drawGaugeLines(startAngle, endAngle, centerX, centerY, 65);
}

void Circular_Gauge::drawArcGaugeData(float value) {
    float angle = scale(gaugeMin,gaugeMax,startAngleD,endAngleD,value); 

    drawValue(value);
    drawNeedle(angle, startAngle, endAngle, centerX, centerY, radius, INVERSE); 
    oled.display();
    drawNeedle(angle, startAngle, endAngle, centerX, centerY, radius, INVERSE); //erase the needle
    drawValue(value);
}









#define BARGRAPH_X 0
#define BARGRAPH_Y 10
#define BARGRAPH_W WIDTH
#define BARGRAPH_H 24

#define BAR_INSET 2

#define BAR_MIN BARGRAPH_X + (BAR_INSET + 1)
#define BAR_MAX BARGRAPH_W - (2 * (BAR_INSET + 1))

void Circular_Gauge::drawBarGaugeBackground() {
    int labelX = calcLabelX(gaugeLabel);

    oled.drawRect(BARGRAPH_X, BARGRAPH_Y, BARGRAPH_W, BARGRAPH_H, WHITE);
    drawLabel(labelX, 15, gaugeLabel);
}

void Circular_Gauge::drawBarGaugeData(float value) {
    int barWidth = scale(gaugeMin, gaugeMax, 0, BAR_MAX, value);

    oled.fillRect(BAR_MIN,
                     BARGRAPH_Y + (BAR_INSET + 1),
                     barWidth, 
                     BARGRAPH_H - (2 * (BAR_INSET + 1)), 
                     INVERSE);
    drawValue(value);
    oled.display();
    drawValue(value);
    oled.fillRect(BAR_MIN, 
                     BARGRAPH_Y + (BAR_INSET + 1), 
                     barWidth,
                     BARGRAPH_H - (2 * (BAR_INSET + 1)),
                     INVERSE);
}









void Circular_Gauge::drawNumGaugeBackground() {
    int x = calcLabelX(gaugeLabel);
    drawLabel(x, 15, gaugeLabel);
}

void Circular_Gauge::drawNumGaugeData(float value) {
    drawValue(value);
    oled.display();
    drawValue(value);
}







void Circular_Gauge::drawTxtGaugeBackground() {
    int x = calcLabelX(gaugeLabel);
    drawLabel(x, 15, gaugeLabel);
}
void Circular_Gauge::drawTxtGaugeData(const char* value) {
    drawValue(value);
    oled.display();
    drawValue(value);
}