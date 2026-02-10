/**
 * @file EnicBomb.cpp / .h
 * @authors Sertac ALAN & Kaan GUNER
 * @brief Specialized countdown and animation sequence logic
 * @version 1.0
 * @date 2026-02-10
 * @copyright Copyright (c) 2026
 */
#include "EnicBomb.h"
#include "EnicMotor.h"
#include "EnicFace.h"
#include "EnicSense.h"

void EnicBomb::begin(EnicMotor* m, EnicFace* f, EnicSense* s) {
  motor = m;
  face  = f;
  sense = s;
}

void EnicBomb::start() {
  if (!motor || !face || !sense) return;

  active = true;
  startMs = millis();
  nextFrameMs = startMs;
  nextBeepMs = startMs;

  motor->drive(0, 0);     // hareket yok
  // ilk “fuse” hissi
  sense->playEffect(3);   // kısa chirp
}

void EnicBomb::stop() {
  active = false;
}

bool EnicBomb::isActive() const {
  return active;
}

// true: devam ediyor, false: bitti
bool EnicBomb::update() {
  if (!active) return false;

  unsigned long now = millis();
  unsigned long elapsed = now - startMs;

  // 30 saniye dolduysa bitir
  if (elapsed >= DURATION_MS) {
    active = false;
    return false;
  }

  // küçük beep’ler (ilk 5 saniyede “fitil” hissi)
  if (elapsed < 5000 && now >= nextBeepMs) {
    nextBeepMs = now + (unsigned long)random(250, 600);
    sense->playEffect(3);
  }

  // frame timing
  if (now < nextFrameMs) return true;
  nextFrameMs = now + FRAME_MS;

  // Fazlar:
  // 0-5s: fitil + geri sayım
  // 5-7s: hızlı flash
  // 7-12s: patlama büyüme
  // 12-30s: duman/sonrası
  uint8_t phase = 0;
  if      (elapsed < 5000)  phase = 0;
  else if (elapsed < 7000)  phase = 1;
  else if (elapsed < 12000) phase = 2;
  else                      phase = 3;

  // 0..255 progress (faz içinde)
  unsigned long phaseStart = 0, phaseLen = 1;
  if (phase == 0) { phaseStart = 0;     phaseLen = 5000;  }
  if (phase == 1) { phaseStart = 5000;  phaseLen = 2000;  }
  if (phase == 2) { phaseStart = 7000;  phaseLen = 5000;  }
  if (phase == 3) { phaseStart = 12000; phaseLen = 18000; }

  unsigned long p = elapsed - phaseStart;
  uint8_t progress = (uint8_t)min(255UL, (p * 255UL) / phaseLen);

  // asıl “video” hissi veren sahne çizimi
  face->drawBombScene(phase, progress, elapsed);

  // patlama anında ekstra efekt (phase 2 başında)
  if (phase == 2 && progress < 15) {
    sense->playEffect(1); // korku sweep -> patlama hissi
  }
  // sonrası "boom" (phase 3'e geçerken)
  if (phase == 3 && progress < 10) {
    sense->playEffect(2); // mutlu bip gibi ama "aftershock" hissi
  }

  return true;
}