/**
 * @file EnicBomb.cpp / .h
 * @authors Sertac ALAN & Kaan GUNER
 * @brief Specialized countdown and animation sequence logic
 * @version 1.0
 * @date 2026-02-10
 * @copyright Copyright (c) 2026
 */
#ifndef ENIC_BOMB_H
#define ENIC_BOMB_H

#include <Arduino.h>

class EnicFace;
class EnicSense;
class EnicMotor;

class EnicBomb {
public:
  EnicBomb() = default;

  void begin(EnicMotor* m, EnicFace* f, EnicSense* s);
  void start();             // bomb modu başlat
  void stop();              // bomb modu bitir
  bool isActive() const;    // şu an bomb modunda mı?
  bool update();            // her loop'ta çağır; biterse false döner

private:
  EnicMotor* motor = nullptr;
  EnicFace*  face  = nullptr;
  EnicSense* sense = nullptr;

  bool active = false;

  unsigned long startMs = 0;
  unsigned long nextFrameMs = 0;

  // toplam 30 saniye
  static const unsigned long DURATION_MS = 30000;

  // ekran update aralığı (FPS hissi)
  static const unsigned long FRAME_MS = 70; // ~14 FPS

  // küçük efekt zamanlayıcıları
  unsigned long nextBeepMs = 0;
};

#endif