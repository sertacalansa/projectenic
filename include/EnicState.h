/**
 * @file EnicState.h
 * @authors Sertac ALAN & Kaan GUNER
 * @brief Finite State Machine (FSM) logic for autonomous behavior
 * @version 1.0
 * @date 2026-02-10
 * @copyright Copyright (c) 2026
 */
#ifndef ENIC_STATE_H
#define ENIC_STATE_H

#include <Arduino.h>
#include "EnicMotor.h"
#include "EnicFace.h"
#include "EnicSense.h"
#include "EnicBomb.h"

enum AppState { IDLE, MANUAL, MANUAL_OBSTACLE, AUTO, AVOIDING, DANCE, BOMB };

class EnicStateMachine {
private:
  EnicMotor* motor;
  EnicFace*  face;
  EnicSense* sense;

  EnicBomb bomb;

  AppState currentState = IDLE;
  unsigned long now = 0;

  // AUTO pattern
  unsigned long timerAutoMove = 0;
  bool isAutoMoving = false;

  // DANCE
  unsigned long timerDance = 0;
  int danceFrame = 0;

  // AVOID
  enum AvoidPhase { AV_START, AV_BACK, AV_TURN, AV_DONE };
  AvoidPhase avoidPhase = AV_START;
  unsigned long avoidUntil = 0;
  int avoidTurnDir = 1;

  // thresholds
  const float AUTO_OBS_ENTER = 20.0f;
  const float AUTO_OBS_EXIT  = 28.0f;
  const float AVOID_EARLY_CLEAR = 35.0f;

  const float MANUAL_OBS_LIMIT = 15.0f;
  const float MANUAL_CLEAR_LIMIT = 25.0f;

  bool autoObstacleLatched = false;

  // Face override for commands
  bool faceOverrideActive = false;
  FaceType faceOverrideType = NORMAL;
  unsigned long faceOverrideUntil = 0;

  void setFaceOverride(FaceType t, unsigned long ms, int soundType = 0) {
    faceOverrideActive = true;
    faceOverrideType = t;
    faceOverrideUntil = millis() + ms;
    face->draw(t);
    if (soundType > 0) sense->playEffect(soundType);
  }

  void enterState(AppState st) {
    if (st == IDLE) {
      motor->drive(0, 0);
    }
    else if (st == MANUAL) {
      face->draw(NORMAL);
    }
    else if (st == MANUAL_OBSTACLE) {
      motor->drive(0, 0);
      face->draw(FEAR);
      sense->playEffect(1);
    }
    else if (st == AUTO) {
      face->draw(NORMAL);
      sense->playEffect(2);
      isAutoMoving = true;
      timerAutoMove = millis() + random(2000, 5000);
    }
    else if (st == AVOIDING) {
      avoidPhase = AV_START;
      avoidUntil = millis();
      avoidTurnDir = (random(0, 2) == 0) ? 1 : -1;
    }
    else if (st == DANCE) {
      motor->drive(0, 0);
      timerDance = millis();
      danceFrame = 0;
    }
    else if (st == BOMB) {
      motor->drive(0, 0);
      bomb.start();
    }
  }

  void changeState(AppState st) {
    if (st == currentState) return;
    currentState = st;
    enterState(st);
  }

  void startAvoiding() {
    autoObstacleLatched = true;
    changeState(AVOIDING);
  }

  void updateAvoiding(float dist) {
    bool canEarlyFinish = (dist >= AVOID_EARLY_CLEAR);

    switch (avoidPhase) {
      case AV_START:
        motor->drive(0, 0);
        face->draw(SHOCK);
        sense->playEffect(1);
        avoidUntil = now + 250;
        avoidPhase = AV_BACK;
        break;

      case AV_BACK:
        if (now < avoidUntil) return;
        face->draw(SNEAKY);
        motor->drive(-180, -180);
        avoidUntil = now + (canEarlyFinish ? 220UL : 480UL);
        avoidPhase = AV_TURN;
        break;

      case AV_TURN:
        if (now < avoidUntil) return;
        if (avoidTurnDir > 0) motor->drive(-180, 180);
        else motor->drive(180, -180);
        avoidUntil = now + (canEarlyFinish ? (unsigned long)random(180, 320)
                                           : (unsigned long)random(420, 780));
        avoidPhase = AV_DONE;
        break;

      case AV_DONE:
        if (now < avoidUntil) return;
        motor->drive(0, 0);
        changeState(AUTO);
        break;
    }
  }

public:
  EnicStateMachine(EnicMotor* m, EnicFace* f, EnicSense* s)
    : motor(m), face(f), sense(s) {}

  void begin() {
    bomb.begin(motor, face, sense);
    changeState(IDLE);
  }

  void handleCommand(String cmd) {
    cmd.trim();
    cmd.toLowerCase();

    if (cmd == "dur")   { changeState(IDLE); return; }
    if (cmd == "otonom" || cmd == "gez") { changeState(AUTO); return; }
    if (cmd == "dans")  { changeState(DANCE); return; }

    // NEW: bomb mode
    if (cmd == "bomb")  { changeState(BOMB); return; }

    // expression commands
    if (cmd == "konus") { setFaceOverride(SPEAK, 1200, 3); changeState(IDLE); return; }
    if (cmd == "dinle") { setFaceOverride(LISTEN, 1400, 3); changeState(IDLE); return; }
    if (cmd == "sasir") { setFaceOverride(SHOCK, 1000, 1);  changeState(IDLE); return; }
    if (cmd == "kork")  { setFaceOverride(FEAR, 1300, 1);   changeState(IDLE); return; }
    if (cmd == "agla")  { setFaceOverride(CRY, 1600, 5);    changeState(IDLE); return; }
    if (cmd == "dil")   { setFaceOverride(TONGUE, 1400, 3); changeState(IDLE); return; }

    // manual motion
    if (cmd == "ileri") { changeState(MANUAL); motor->drive(200, 200); return; }
    if (cmd == "geri")  { changeState(MANUAL); motor->drive(-200,-200); return; }
    if (cmd == "sol")   { changeState(MANUAL); motor->drive(-180,180); return; }
    if (cmd == "sag")   { changeState(MANUAL); motor->drive(180,-180); return; }
  }

  void update() {
    now = millis();

    sense->update();
    motor->update();

    float dist = sense->getDistance();

    // BOMB mode: ekran animasyonu 30s
    if (currentState == BOMB) {
      motor->drive(0, 0);
      bool stillPlaying = bomb.update();
      if (!stillPlaying) {
        changeState(IDLE);
      }
      return;
    }

    // DANCE
    if (currentState == DANCE) {
      if (now - timerDance > 250) {
        timerDance = now;
        sense->playEffect(4);
        face->drawDance(danceFrame++);
        if (danceFrame > 3) danceFrame = 0;
      }
      return;
    }

    switch (currentState) {
      case AUTO: {
        if (!autoObstacleLatched) {
          if (dist < AUTO_OBS_ENTER) { startAvoiding(); break; }
        } else {
          if (dist > AUTO_OBS_EXIT) autoObstacleLatched = false;
        }

        if (now > timerAutoMove) {
          isAutoMoving = !isAutoMoving;
          if (isAutoMoving) timerAutoMove = now + (unsigned long)random(2500, 6500);
          else { timerAutoMove = now + (unsigned long)random(1500, 4500); motor->drive(0, 0); }
        }

        if (isAutoMoving) motor->drive(130, 130);
        else face->updateIdle();
        break;
      }

      case AVOIDING:
        updateAvoiding(dist);
        break;

      case MANUAL:
        if (dist < MANUAL_OBS_LIMIT) changeState(MANUAL_OBSTACLE);
        break;

      case MANUAL_OBSTACLE:
        motor->drive(0, 0);
        if (dist > MANUAL_CLEAR_LIMIT) changeState(IDLE);
        break;

      case IDLE:
      default:
        if (faceOverrideActive) {
          if (now >= faceOverrideUntil) {
            faceOverrideActive = false;
            face->draw(NORMAL);
          }
        } else {
          face->updateIdle();
        }
        break;
    }
  }
};

#endif