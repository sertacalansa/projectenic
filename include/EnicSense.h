/**
 * @file EnicSense.h
 * @authors Sertac ALAN & Kaan GUNER
 * @brief Sensor fusion (Ultrasonic) and buzzer feedback system
 * @version 1.0
 * @date 2026-02-10
 * @copyright Copyright (c) 2026
 */
#ifndef ENIC_SENSE_H
#define ENIC_SENSE_H

#include <Arduino.h>

#define TRIG_PIN 5
#define ECHO_PIN 18

#define BUZZER_PIN 4
#define BUZZER_CHANNEL 0

class EnicSense {
private:
  float emaDist = 999.0f;
  bool  emaInit = false;
  float emaAlpha = 0.45f;

  unsigned long lastPingMs = 0;
  unsigned long pingIntervalMs = 80;

  struct SoundJob {
    bool active = false;
    int type = 0;
    int step = 0;
    unsigned long nextMs = 0;
  } sound;

  float readDistanceOnce() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 10000);
    if (duration <= 0) return 999.0f;

    float d = duration * 0.034f / 2.0f;
    if (d <= 0 || d > 400) return 999.0f;
    return d;
  }

  void startSound(int type) {
    sound.active = true;
    sound.type = type;
    sound.step = 0;
    sound.nextMs = millis();
  }

public:
  void begin() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    ledcSetup(BUZZER_CHANNEL, 2000, 8);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

    stopSound();
  }

  float getDistance() const { return emaDist; }

  void stopSound() {
    ledcWriteTone(BUZZER_CHANNEL, 0);
    ledcWrite(BUZZER_CHANNEL, 0);
    sound.active = false;
    sound.type = 0;
    sound.step = 0;
  }

  // 1:Korku, 2:Mutlu, 3:Konuşma, 4:Dans, 5:Ağlama
  void playEffect(int type) {
    if (type <= 0) return;
    startSound(type);
  }

  void update() {
    unsigned long now = millis();

    // Distance (EMA)
    if (now - lastPingMs >= pingIntervalMs) {
      lastPingMs = now;

      float d1 = readDistanceOnce();
      delayMicroseconds(200);
      float d2 = readDistanceOnce();
      float d = (d1 + d2) * 0.5f;

      if (!emaInit) { emaDist = d; emaInit = true; }
      else { emaDist = (emaAlpha * d) + ((1.0f - emaAlpha) * emaDist); }
    }

    // Sound scheduler
    if (!sound.active) return;
    if (now < sound.nextMs) return;

    switch (sound.type) {
      case 1: { // FEAR sweep
        static const int tones[] = {2000, 1700, 1400, 1100, 900, 750, 650, 550};
        if (sound.step >= (int)(sizeof(tones)/sizeof(tones[0]))) { stopSound(); break; }
        ledcWriteTone(BUZZER_CHANNEL, tones[sound.step++]);
        ledcWrite(BUZZER_CHANNEL, 200);
        sound.nextMs = now + 18;
        break;
      }
      case 2: { // HAPPY 2 bips
        if (sound.step == 0) {
          ledcWriteTone(BUZZER_CHANNEL, 1000); ledcWrite(BUZZER_CHANNEL, 220);
          sound.step = 1; sound.nextMs = now + 90;
        } else if (sound.step == 1) {
          ledcWriteTone(BUZZER_CHANNEL, 0); ledcWrite(BUZZER_CHANNEL, 0);
          sound.step = 2; sound.nextMs = now + 35;
        } else if (sound.step == 2) {
          ledcWriteTone(BUZZER_CHANNEL, 2000); ledcWrite(BUZZER_CHANNEL, 220);
          sound.step = 3; sound.nextMs = now + 90;
        } else stopSound();
        break;
      }
      case 3: { // SPEAK short chirp
        if (sound.step == 0) {
          ledcWriteTone(BUZZER_CHANNEL, random(700, 2600));
          ledcWrite(BUZZER_CHANNEL, 190);
          sound.step = 1; sound.nextMs = now + 90;
        } else stopSound();
        break;
      }
      case 4: { // DANCE beat
        if (sound.step == 0) { ledcWriteTone(BUZZER_CHANNEL, 120); ledcWrite(BUZZER_CHANNEL, 220); sound.step=1; sound.nextMs=now+70; }
        else if (sound.step == 1) { ledcWriteTone(BUZZER_CHANNEL, 0); ledcWrite(BUZZER_CHANNEL, 0); sound.step=2; sound.nextMs=now+25; }
        else if (sound.step == 2) { ledcWriteTone(BUZZER_CHANNEL, 850); ledcWrite(BUZZER_CHANNEL, 220); sound.step=3; sound.nextMs=now+55; }
        else stopSound();
        break;
      }
      case 5: { // CRY
        static const int tones[] = {420, 360, 300, 360, 420, 0, 420, 360, 300, 0};
        if (sound.step >= (int)(sizeof(tones)/sizeof(tones[0]))) { stopSound(); break; }
        int t = tones[sound.step++];
        ledcWriteTone(BUZZER_CHANNEL, t);
        ledcWrite(BUZZER_CHANNEL, t == 0 ? 0 : 170);
        sound.nextMs = now + 90;
        break;
      }
      default:
        stopSound();
        break;
    }
  }
};

#endif