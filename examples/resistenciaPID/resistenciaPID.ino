/*
 * Arquivo alterado para funcionar com a biblioteca DimmerV0 (versao inicial)
 */


#include <TimerOne.h>
#include "DimmerV0.h"
#include <SPI.h>
#include <PID_v1.h>
#include "Nanoshield_Thermocouple.h"
Dimmer dimmer;

//PID
//Define PID controller variables here
#define KP  30
#define KI  0.01
#define KD  0.3

double Setpoint, Input, Output;  
double Kp = KP, Ki = KI, Kd = KD;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

//Termopar
Nanoshield_Thermocouple thermocouple;
#define  lamp  6
#define  Triac  3
#define   stepDelay 500  //ms
char dado[10];
boolean  flagi=false, setting=false, isOK=false, work=false, controller=false, turned=false;
byte count=0, settingState=0, pointsCount=0, controlCount=0;
float initialTemp, pointsI[10], finalPoint;
unsigned long pointsTime[10], myTime = 0, currentTime = 0, timeLeft;
unsigned int erro=0;

void setup() {
  Serial.begin(115200);
  Serial.print("Intel Edison - Controle de Temperatura\n\n");

  dimmer.attachZeroCross(2, 2);
  dimmer.attachTriac(Triac);
  dimmer.initCount();
  dimmer.set(1, 0);
  thermocouple.begin();
  thermocouple.read();
  Input = thermocouple.getExternal();
  pinMode(lamp, OUTPUT);
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0, 100);
}

void loop() {
  if (turned) {
    thermocouple.read();
    Input = thermocouple.getExternal();
    myPID.Compute();
    refresh(); //imprime serial + set pot + delay
  }
  else if(!work) { //waiting for input
    flagi = !flagi;
    analogWrite(lamp, flagi ? 100 : 0);
    delay(300);
  }
 
  if(controller){
    if (!work) {
      erro = abs(Setpoint - Input);
      if ( erro < 5 ) {
        work = true;
        myTime = millis();
        currentTime = myTime + stepDelay;
      }
    }
    //if work
    else {
      if ( millis() >= (myTime + pointsTime[controlCount]) ) {
          controlCount++;
          if ( controlCount >= pointsCount ) {
            controller=false;
            turned=false;
            Output=0;
          }
      }  
      if (millis() > currentTime) {
        currentTime += stepDelay;
        Setpoint += (pointsI[controlCount] / 2);  //increment inclination/2 because this happens 2 times per second
        timeLeft = abs( (myTime + pointsTime[controlCount]) - millis() );
        timeLeft /= 1000L;
      }  
    }
  }
  //if !controller
  else if(work){ //waiting for cool off
    thermocouple.read();
    Input = thermocouple.getExternal();
    refresh();
    if(Input<=finalPoint){
      //Serial.print("{ \"message\": \"Completed\" }\n\n");
      work=false;
      dimmer.set(1, 0);
    }
  }
  
  serialEvent();
}

void refresh() {
  analogWrite(lamp, 2 * Output);
  dimmer.set(1, Output);
  
  Serial.print("{ \"temp\": ");
  Serial.print(Input);
  Serial.print(", \"ref\": ");
  Serial.print(controller ? Setpoint : finalPoint);
  Serial.print(", \"output\": ");
  //Serial.print(Output);
  Serial.print(0);
  Serial.print(" }\n\n");
  delay(200);
}

void printSettings() {
  if (pointsCount == 0) {
    Serial.println("Nenhum ponto configurado!");
  }
  else {
    Serial.print("->Temperatura inicial: ");
    Serial.print(initialTemp);
    Serial.println(" graus.");
    Serial.println("->Pontos configurados:");
    for (byte i = 0; i < pointsCount; i++) {
      Serial.print("   P");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(pointsI[i]);
      Serial.print(" graus/s  por ");
      Serial.print(pointsTime[i]);
      Serial.println("s");
    }
    Serial.print("->Temperatura final de esfriamento: ");
    Serial.print(finalPoint);
    Serial.println(" graus.");
  }
}

void print1() {
  Serial.print("Digite <end> para finalizar ou a inclinacao (graus/s) do set point ");
  Serial.print(pointsCount + 1);
  Serial.println(": ");
}

void settingParameters() {
  Serial.print(dado);
  switch (settingState) {
    //Intro
    case 0:
      isOK = false;
      pointsCount = 0;
      Serial.println("Configuracoes de temperatura:");
      Serial.println("Digite a temperatura inicial desejada: ");
      settingState = 1;
      break;

    case 1:
      initialTemp = atof(dado);
      print1();
      settingState = 2;
      break;

    case 2:
      if (dado[0] == 'e' && dado[1] == 'n' && dado[2] == 'd') {
        Serial.println("Digite a temperatura final de esfriamento:");
        settingState = 4;
      }
      else {
        pointsI[pointsCount] = atof(dado);
        Serial.print("Digite o tempo (s) do set point");
        Serial.print(pointsCount + 1);
        Serial.println(": ");
        settingState = 3;
      }
      break;

    case 3:
      isOK = true;
      pointsTime[pointsCount] = atoi(dado);
      pointsCount++;
      print1();
      settingState = 2;
      break;
      
    case 4:
      finalPoint = atof(dado);
      printSettings();
      Serial.println("Digite <i> para Iniciar o processo, <r> para resetar os pontos.");
      settingState = 10;
      break;
    
    case 10:
      if (dado[0] == 'i') {
        setting = false;
        settingState = 0;
        if (isOK) {
          for (byte i = 0; i < pointsCount; i++) {
            pointsTime[i] = 1000L * pointsTime[i];
            if (i > 0) 
              pointsTime[i] += pointsTime[i - 1];
          }
          Setpoint = initialTemp;
          turned = true;
          controller = true;
          delay(100);
          Serial.print("\n\n");
        }
        else
          Serial.println("Sem dados para processo");
      }
      else if (dado[0] == 'r') {
        //reset
        isOK = false;
        pointsCount = 0;
        Serial.println("Configuracoes de temperatura:");
        Serial.println("Digite a temperatura inicial para iniciar o processo: ");
        settingState = 1;
      }
      break;
  }
}

void serialEvent() {
  while (Serial.available()) {
    dado[count++] = Serial.read();
    if ( count - 2 == sizeof(dado) ) //Quando estoura o vetor
      dado[count - 1] = '\n';
    if (dado[count - 1] == '\n') {
      if (setting)  
        settingParameters();
      else {
        switch (dado[0]) {
          case 'S':
            controller=false;
            work=false;
            turned=false;
            setting = true;
            settingParameters();
            break;
            
          case 'Q':
            controller=false;
            work=false;
            turned=false;
            break;

          case 'P':
            Kp = atof(&dado[1]);
            break;

          case 'I':
            Ki = atof(&dado[1]);
            break;

          case 'D':
            Kd = atof(&dado[1]);
            myPID.SetTunings(Kp, Ki, Kd);
            delayMicroseconds(50);
            Serial.print("P="); Serial.print(Kp);
            Serial.print(" I="); Serial.print(Ki);
            Serial.print(" D="); Serial.println(Kd);
            break;
        }
      }
      count = 0;
      memset(dado, 0, sizeof(dado) );
    }
  }
}
