/**
 * @file EnicMotor.h
 * @authors Sertac ALAN & Kaan GUNER
 * @brief Motor control driver and PWM logic for ENIC
 * @version 1.0
 * @date 2026-02-10
 * @copyright Copyright (c) 2026
 */
#ifndef ENIC_MOTOR_H
#define ENIC_MOTOR_H

#include <Arduino.h>

#define M1_IN1 26
#define M1_IN2 27
#define M2_IN3 14
#define M2_IN4 12

static const uint8_t M1_IN1_CH = 1;
static const uint8_t M1_IN2_CH = 2;
static const uint8_t M2_IN3_CH = 3;
static const uint8_t M2_IN4_CH = 4;

static const uint32_t MOTOR_PWM_FREQ = 20000;
static const uint8_t  MOTOR_PWM_BITS = 8; // 0..255

class EnicMotor {
private:
  int targetLeft  = 0;
  int targetRight = 0;
  int currentLeft  = 0;
  int currentRight = 0;
  int rampStep = 14;

  static inline int clamp255(int v) {
    if (v > 255) return 255;
    if (v < -255) return -255;
    return v;
  }

  static inline int approach(int cur, int tgt, int step) {
    if (cur < tgt) { cur += step; if (cur > tgt) cur = tgt; }
    else if (cur > tgt) { cur -= step; if (cur < tgt) cur = tgt; }
    return cur;
  }

  void writeMotor(int leftSpeed, int rightSpeed) {
    leftSpeed  = clamp255(leftSpeed);
    rightSpeed = clamp255(rightSpeed);

    // Left
    if (leftSpeed >= 0) { ledcWrite(M1_IN1_CH, leftSpeed);  ledcWrite(M1_IN2_CH, 0); }
    else                { ledcWrite(M1_IN1_CH, 0);          ledcWrite(M1_IN2_CH, -leftSpeed); }

    // Right
    if (rightSpeed >= 0){ ledcWrite(M2_IN3_CH, rightSpeed); ledcWrite(M2_IN4_CH, 0); }
    else                { ledcWrite(M2_IN3_CH, 0);          ledcWrite(M2_IN4_CH, -rightSpeed); }
  }

public:
  void begin() {
    pinMode(M1_IN1, OUTPUT); pinMode(M1_IN2, OUTPUT);
    pinMode(M2_IN3, OUTPUT); pinMode(M2_IN4, OUTPUT);

    ledcSetup(M1_IN1_CH, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
    ledcSetup(M1_IN2_CH, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
    ledcSetup(M2_IN3_CH, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
    ledcSetup(M2_IN4_CH, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);

    ledcAttachPin(M1_IN1, M1_IN1_CH);
    ledcAttachPin(M1_IN2, M1_IN2_CH);
    ledcAttachPin(M2_IN3, M2_IN3_CH);
    ledcAttachPin(M2_IN4, M2_IN4_CH);

    stop();
  }

  void setRampStep(int step) {
    if (step < 1) step = 1;
    if (step > 60) step = 60;
    rampStep = step;
  }

  void drive(int leftSpeed, int rightSpeed) {
    targetLeft  = clamp255(leftSpeed);
    targetRight = clamp255(rightSpeed);
  }

  void stop() {
    targetLeft = targetRight = 0;
    currentLeft = currentRight = 0;
    writeMotor(0, 0);
  }

  void update() {
    currentLeft  = approach(currentLeft,  targetLeft,  rampStep);
    currentRight = approach(currentRight, targetRight, rampStep);
    writeMotor(currentLeft, currentRight);
  }
};

#endif