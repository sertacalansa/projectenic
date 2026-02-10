/**
 * @file main.cpp
 * @authors Sertac ALAN & Kaan GUNER
 * @brief ENIC - Electronic Networked Intelligence Company
 * @version 1.0
 * @date 2026-02-10
 * * @copyright Copyright (c) 2026
 * */
#include <Arduino.h>
#include "EnicMotor.h"
#include "EnicFace.h"
#include "EnicSense.h"
#include "EnicState.h"

EnicMotor motor;
EnicFace  face;
EnicSense sense;

EnicStateMachine brain(&motor, &face, &sense);

void setup() {
  Serial.begin(115200);

  motor.begin();
  face.begin();
  sense.begin();

  brain.begin();

  randomSeed(analogRead(0));

  Serial.println("ENIC V1");
  Serial.println("Komutlar: ileri/geri/sol/sag | dur | otonom | dans | konus | dinle | sasir | kork | agla | dil");
}

void loop() {
  brain.update();

  // Non-blocking serial line buffer
  static String line;
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;

    if (c == '\n') {
      if (line.length() > 0) {
        brain.handleCommand(line);
        line = "";
      }
    } else {
      if (line.length() < 64) line += c;
    }
  }
}