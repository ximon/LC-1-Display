#ifndef _LC1_H_
#define _LC1_H_

#include <Arduino.h>
#include <stdio.h>

struct State
{
    float AFR;
    float O2;
    float Warmup;
    float Lambda;
    int CalibrationCountdown;
    int StatusId;
    int ErrorCode;
};

#define AFR_MIN 9
#define AFR_MAX 19

#define STATE_UNKNOWN -1
#define STATE_NORMAL 0
#define STATE_O2 1
#define STATE_FREE_AIR_CALIBRATION_IN_PROGRESS 2
#define STATE_NEEDS_FREE_AIR_CALIBRATION 3
#define STATE_WARMUP 4
#define STATE_HEATER_CALIBRATION 5
#define STATE_ERROR 6
#define STATE_RESERVED 7

#define ERROR_HEATER_SHORT 1
#define ERROR_HEATER_OPEN 2
#define ERROR_PUMP_SHORT 3
#define ERROR_PUMP_OPEN 4
#define ERROR_REF_SHORT 5
#define ERROR_REF_OPEN 6
#define ERROR_SOFTWARE_ERROR 7
#define ERROR_SENSOR_TIMING 8
#define ERROR_SUPPLY_LOW 9

typedef void (*afrCallback) (float afr, float lambda); 
typedef void (*o2Callback) (float);
typedef void (*stateIdCallback) (int);
typedef void (*warmupCallback) (float);
typedef void (*errorCallback) (int);
typedef void (*calibrationCountdownCallback) (int);
typedef void (*dataCallback) (int);

class LC1 {
    public:
        LC1();
        
        void setAFRCallback(afrCallback callback);
        void setO2Callback(o2Callback callback);
        void setStateIdCallback(stateIdCallback callback);
        void setWarmupCallback(warmupCallback callback);
        void setDataCallback(dataCallback callback);
        void setErrorCallback(errorCallback callback);
        void setCalibrationCountdownCallback(calibrationCountdownCallback callback);

        void processData(byte data);
    private:
        byte packet[10];
        int packetIndex = 0;

        byte packetCheck[6] = { 0xA2, 0x80, 0x42, 0x00, 0x00, 0x00 };

        State state;
        State prevState;

        afrCallback afrChanged;
        o2Callback o2Changed;
        stateIdCallback stateChanged;
        warmupCallback warmupChanged;
        dataCallback dataChanged;
        errorCallback errorChanged;
        calibrationCountdownCallback calibrationCountdownChanged;

        void parsePackets();
        void updateState();
};


#endif
