/*
 This is the Dimmer library to control AC loads, including dimmable lamps.
 
 This is an old version (v0.1) to work with older projects. Please look up for the newer version if nedeed.
 */

#ifndef DIMMERV0_H
#define DIMMERV0_H

#include "Arduino.h"
#include <TimerOne.h>

#define MAX_TRIAC 10

#ifndef dimmerObject
#define dimmerObject dimmer
#endif

class Dimmer{
    
public:
    Dimmer();
    void attachZeroCross(int zc_pin, int inter);
    void attachTriac(byte pin);
    int init();
    int initRamp();
    int initCount(byte resolution = 1);
    void off();
    void off(byte lamp);
    void on();
    void on(byte lamp);
    void toggle(byte lamp);
    void set(byte lamp, byte value);
    void zeroCross();
    void triac();

private:
    int start();
    bool ramp_mode;
    bool countMode;
    byte countResolution;
    boolean zeroCrosAttached;
    boolean triacAttached;
    
    //Triac pin vector
    byte triacPins[MAX_TRIAC];
    
    //Triac pin counter
    byte triacs;
    
    //Dimmer lamps value
    byte lampValue[MAX_TRIAC];
    
    //Temporary lamps value
    byte lampValue_ramp[MAX_TRIAC];
    
    //Virtual switch
    byte lampSwitch[MAX_TRIAC];
    
    //50ms Counter
    unsigned int msCounter;
    
    // Half cycle counter
    byte halfCycleCounter;
};

extern Dimmer dimmerObject;

void callZeroCross();
void callTriac();

#endif
