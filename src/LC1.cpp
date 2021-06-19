
#include <Arduino.h>
#include <stdio.h>

#include "LC1.h"

LC1::LC1()
{
    state.StatusId = STATE_UNKNOWN;
    state.O2 = -1;
    state.AFR = -1;
    state.Lambda = -1;
    state.ErrorCode = -1;
    state.Warmup = -1;
    state.CalibrationCountdown = -1;
}

void LC1::setDataCallback(dataCallback callback){
    dataChanged = callback;
}
void LC1::setAFRCallback(afrCallback callback){
    afrChanged = callback;
}
void LC1::setStateIdCallback(stateIdCallback callback){
    stateChanged = callback;
}
void LC1::setWarmupCallback(warmupCallback callback){
    warmupChanged = callback;
}
void LC1::setO2Callback(o2Callback callback){
    o2Changed = callback;
}
void LC1::setCalibrationCountdownCallback(calibrationCountdownCallback callback){
    calibrationCountdownChanged = callback;
}
void LC1::setErrorCallback(errorCallback callback){
    errorChanged = callback;
}

#define INITIAL_PACKETCOUNT 2

int packetCount = INITIAL_PACKETCOUNT;
int wordCount = 0;
void LC1::processData(byte data)
{
    byte checkData = packetCheck[packetIndex];

    if (packetIndex < packetCount)
    {
        if (packetIndex == 0)
            wordCount = data & 0b00000001;
        
        if (packetIndex == 1)
        {
            wordCount += data & 0b01111111;
            packetCount = (wordCount + 1) * 2;         //+1 for the header word, * 2 for bytes
        }

        //if the incoming data isn't as expected then reset and wait
        if (checkData != 0x00 && (data & checkData) != checkData)
        {
            packetIndex = 0;
            packetCount = INITIAL_PACKETCOUNT;
            wordCount = 0;
            dataChanged(packetIndex);
            return;
        }
    
        packet[packetIndex] = data; // Store buffer data to packet in series
        packetIndex++;              // all else are data packets, increment the packet counter by one
        dataChanged(packetIndex);

        if (packetIndex < packetCount) //skip the rest until we've collected all the packets
            return;
    }

    if (packetIndex == packetCount)
    {
        packetCount = INITIAL_PACKETCOUNT;
        packetIndex = 0;
        parsePackets();
    }
}

void LC1::parsePackets()
{
    int status = (packet[2] & 0b00011100) >> 2;  // F2-F0
    int afrMult = ((packet[2] & 0b00000001) << 7) + (packet[3] & 0b01111111);           // AF7, AF6-AF0
    int lambda = ((packet[4] & 0b00111111) << 7) + (packet[5] & 0b01111111);        // L12-L7, L6-L0  

    prevState.Warmup = state.Warmup;
    prevState.O2 = state.O2;
    prevState.Lambda = state.Lambda;
    prevState.AFR = state.AFR;
    prevState.CalibrationCountdown = state.CalibrationCountdown;
    prevState.ErrorCode = state.ErrorCode;
    prevState.StatusId = state.StatusId;

    state.Warmup = status == STATE_WARMUP ? ((float)lambda) / 10 : -1;
    state.O2 = status == STATE_O2 ? ((float)lambda) / 10 : -1;
    state.Lambda = status == STATE_NORMAL ? ((float)(lambda + 500)) / 1000 : -1;
    state.AFR = status == STATE_NORMAL ? state.Lambda * ((float)afrMult / 10) : -1; //AFR = ((L12..L0) + 500) * (AF7..AF0) / 10000
    state.CalibrationCountdown = status == STATE_HEATER_CALIBRATION ? lambda : -1;
    state.ErrorCode = status == STATE_ERROR ? lambda : -1;
    state.StatusId = status;

    if (state.StatusId != prevState.StatusId)
        stateChanged(state.StatusId);
    
    updateState();
}

void LC1::updateState()
{
    // Output data depends on Status
    switch (state.StatusId)
    {
    case STATE_NORMAL:
        // Lambda valid and Aux data valid, Normal operation
        if (state.AFR != prevState.AFR)
            afrChanged(state.AFR, state.Lambda);
        break;
    case STATE_O2:
        // Lambda value contains O2 level in 1/10%
        if (state.O2 != prevState.O2)
            o2Changed(state.O2);
        break;
    case STATE_FREE_AIR_CALIBRATION_IN_PROGRESS:
        // Free air calibration in progress, Lambda data not valid
        break;
    case STATE_NEEDS_FREE_AIR_CALIBRATION:
        // Need Free air Calibration Request, Lambda data not valid
        break;
    case STATE_WARMUP:
        // Warming up, Lambda value is temp in 1/10% of operating temp
        if (state.Warmup != prevState.Warmup)
            warmupChanged(state.Warmup);
        break;
    case STATE_HEATER_CALIBRATION:
        // Heater Calibration, Lambda value contains calibration countdown
        if (state.CalibrationCountdown != prevState.CalibrationCountdown)
            calibrationCountdownChanged(state.CalibrationCountdown);
        break;
    case STATE_ERROR:
        // Error code in Lambda value
        if (state.ErrorCode != prevState.ErrorCode)
            errorChanged(state.ErrorCode);
        break;
    case STATE_RESERVED:
        // Reserved
        break;
    default:
        break;
    }
}
