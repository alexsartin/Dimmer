/**
 * Control 3 dimmable lamps
 *
 * Copyright (c) 2015 Circuitar
 * All rights reserved.
 *
 * This software is released under a BSD license. See the attached LICENSE file for details.
 *
 * http://www.circuitar.com.br
 *
 */

#include <TimerOne.h>
#include "DimmerV0.h"

Dimmer dimmer;

void setup() {
  Serial.begin(9600);
  dimmer.attachZeroCross(2,0);
  dimmer.attachTriac(3);
  dimmer.attachTriac(5);
  dimmer.attachTriac(6);
  dimmer.init();
  randomSeed(analogRead(0));
}

void loop(){
  char lamp=random(1,4);
  char value=random(0,256);
  dimmer.set(lamp, value);
  delay(1000);
};

