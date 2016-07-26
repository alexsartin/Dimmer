/*
 This is the Dimmer library to control AC loads, including dimmable lamps.
 
 Copyright (c) 2015 Circuitar
 This software is released under the MIT license. See the attached LICENSE file for details.
 */

#include "Arduino.h"
#include <TimerOne.h>
#include "DimmerV0.h"

//Time table to set triac
const short triac_time[] ={
    147, 142, 139, 136, 133, 131, 129, 127, 125, 124, 
    122, 121, 119, 118, 117, 115, 114, 113, 112, 111, 
    110, 109, 108, 107, 106, 105, 103, 102, 101, 100, 
    100,  99,  98,  97,  96,  95,  94,  93,  92,  92, 
     91,  90,  89,  88,  87,  87,  86,  85,  84,  83, 
     82,  82,  81,  80,  79,  78,  77,  77,  76,  75, 
     74,  73,  72,  71,  70,  70,  69,  68,  67,  66, 
     65,  64,  63,  62,  61,  60,  59,  58,  57,  56, 
     55,  54,  53,  51,  50,  49,  47,  46,  45,  43, 
     41,  40,  38,  36,  34,  31,  28,  24,  19,  0, 0
};

//ZeroCross Interrupt
void callZeroCross(){
    dimmerObject.zeroCross();
}

//Timer interrupt
void callTriac(){
    dimmerObject.triac();
}

//Class Constructor
Dimmer::Dimmer(){
    zeroCrosAttached=false;
    triacAttached=false;
    triacs=0;
}

void Dimmer::attachZeroCross(int zc_pin, int inter){
    zeroCrosAttached=true;
    pinMode(zc_pin, INPUT);
    attachInterrupt(inter, callZeroCross, RISING);
}

void Dimmer::attachTriac(byte pin){
    triacAttached=true;
    triacPins[triacs]= pin;
    triacs++;
}

int Dimmer::init(){
    ramp_mode=false;
    countMode=false;
    return Dimmer::start();
}

int Dimmer::initRamp(){
    ramp_mode=true;
    countMode=false;
    return Dimmer::start();
}

int Dimmer::initCount(byte resolution){
    ramp_mode=false;
    countMode=true;
    countResolution = resolution;
    return Dimmer::start();
}

void Dimmer::off(){
    for(byte i=0; i < triacs; i++){
        lampSwitch[i]=false;
    }
}

void Dimmer::off(byte lamp){
    lampSwitch[--lamp]=false;
}

void Dimmer::on(){
    for(byte i=0; i < triacs; i++){
        lampSwitch[i]=true;
    }
}

void Dimmer::on(byte lamp){
    lampSwitch[--lamp]=true;
}

void Dimmer::toggle(byte lamp){
    lamp--;
    lampSwitch[lamp]=!lampSwitch[lamp];
}

void Dimmer::set(byte lamp, byte value){
    lamp--;
    if (ramp_mode){
        lampValue[lamp]=value;
    }
    else{
        lampValue_ramp[lamp]=value;
    }
}

int Dimmer::start(){
    if (!zeroCrosAttached || !triacAttached) {
        return 0;
    }
    
    if (!countMode) {
        // Initialize Timer1
        Timer1.initialize();
        // Attach timer 1 to triac function
        //Called every 50 micro-seconds
        Timer1.attachInterrupt(callTriac, 50);
    }
    
    halfCycleCounter = 0;
    
    for(byte i=0; i < triacs; i++){
        pinMode(triacPins[i], OUTPUT);
        lampSwitch[i]=true;
        lampValue[i]= 50;
        lampValue_ramp[i]= 50;
    }
    
    return 1;
}

void Dimmer::zeroCross(){
    if (countMode) {
        for(byte i=0; i<triacs; i++){
            if (halfCycleCounter < lampValue_ramp[i] / countResolution && lampSwitch[i]) {
                digitalWrite(triacPins[i], HIGH);
            } else {
                digitalWrite(triacPins[i], LOW);
            }
        }

        if (++halfCycleCounter >= 100 / countResolution) {
            halfCycleCounter = 0;
        }
    } else {
        //Clear counter
        msCounter=0;
        //Turn-off triacs
        for (byte i=0; i<triacs; i++){
            digitalWrite(triacPins[i], LOW);
        }
    }
}

void Dimmer::triac(){
    if (!countMode) {
        msCounter++;
        //With ramp mode
        if (ramp_mode && msCounter % 150 == 0 ) {
                for(byte a=0; a<triacs; a++){
                    if(lampValue_ramp[a] > lampValue[a])
                        lampValue_ramp[a]--;
                    else if(lampValue_ramp[a] < lampValue[a])
                        lampValue_ramp[a]++;
                }
        }
    
        for(byte i=0; i<triacs; i++){
            if(lampValue_ramp[i] > 0 && lampSwitch[i]){
                if(triac_time[ lampValue_ramp[i] ] == msCounter){
                    digitalWrite(triacPins[i], HIGH);
                }
            }
        }
    }
}
