/**
 * @file EnicFace.h
 * @authors Sertac ALAN & Kaan GUNER
 * @brief OLED Display manager and facial expression engine
 * @version 1.0
 * @date 2026-02-10
 * @copyright Copyright (c) 2026
 */
#ifndef ENIC_FACE_H
#define ENIC_FACE_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Yüz tipleri
enum FaceType {
  NORMAL,
  BLINK,
  DEAD,
  TONGUE,
  LISTEN,
  SPEAK,
  SHOCK,
  SNEAKY,
  CRY,
  FEAR
};

class EnicFace {
private:
  Adafruit_SSD1306 display;

  // ---------------- IDLE: Base expression timing ----------------
  FaceType baseFace = NORMAL;
  unsigned long nextBaseChangeMs = 0; // 5-7 sn sonra "büyük ifade" seç
  unsigned long baseFaceUntilMs  = 0; // seçilen büyük ifade 2 sn kalsın

  // ---------------- IDLE: Blink overlay timing ----------------
  bool blinkActive = false;
  unsigned long nextBlinkMs  = 0; // sık ve rastgele
  unsigned long blinkUntilMs = 0; // blink 90-180ms

  FaceType pickRandomBaseFace() {
    // BLINK burada yok; blink ayrı scheduler
    int r = random(0, 100);
    if (r < 18) return SNEAKY;
    if (r < 36) return TONGUE;
    if (r < 52) return SHOCK;
    if (r < 68) return FEAR;
    if (r < 84) return CRY;
    if (r < 92) return LISTEN;
    return SPEAK;
  }

public:
  EnicFace() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1) {}

  void begin() {
    Wire.begin(21, 22);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println("OLED init failed!");
    }
    display.clearDisplay();
    display.display();

    unsigned long now = millis();
    baseFace = NORMAL;
    nextBaseChangeMs = now + (unsigned long)random(5000, 7000);
    baseFaceUntilMs  = 0;

    blinkActive = false;
    nextBlinkMs  = now + (unsigned long)random(400, 1400);
    blinkUntilMs = 0;

    draw(NORMAL);
  }

  // IDLE davranışı:
  // - 5-7 saniyede bir "büyük ifade" (2 saniye kalır)
  // - aralarda blink sık ve rastgele (ifadelerin üstüne de gelebilir)
  void updateIdle() {
    unsigned long now = millis();

    // 1) Blink bitti -> base yüzü geri çiz
    if (blinkActive && now >= blinkUntilMs) {
      blinkActive = false;
      draw(baseFace);
    }

    // 2) Base ifade (NORMAL dışı) süresi bitti -> NORMAL'e dön
    if (baseFace != NORMAL && now >= baseFaceUntilMs) {
      baseFace = NORMAL;
      if (!blinkActive) draw(baseFace);
      nextBaseChangeMs = now + (unsigned long)random(5000, 7000);
    }

    // 3) 5-7 saniyede bir büyük ifade seç, 2 saniye tut
    if (baseFace == NORMAL && now >= nextBaseChangeMs) {
      baseFace = pickRandomBaseFace();
      baseFaceUntilMs = now + 2000UL;
      if (!blinkActive) draw(baseFace);

      // Tekrar tetiklenmesin diye ileriye at; normal'e dönünce yeniden planlanacak
      nextBaseChangeMs = now + 999999UL;
    }

    // 4) Blink sık ve rastgele
    if (!blinkActive && now >= nextBlinkMs) {
      blinkActive = true;
      blinkUntilMs = now + (unsigned long)random(90, 180);
      nextBlinkMs  = now + (unsigned long)random(600, 1800);
      draw(BLINK);
    }
  }

  // Tek seferlik yüz çizimi
  void draw(FaceType type) {
    display.clearDisplay();

    int lx = 40, rx = 88, y = 25, r = 8;
    int mx = 64, my = 50;

    // ---- EYES ----
    if (type == DEAD) {
      display.drawLine(lx-6,y-6,lx+6,y+6,1); display.drawLine(lx+6,y-6,lx-6,y+6,1);
      display.drawLine(rx-6,y-6,rx+6,y+6,1); display.drawLine(rx+6,y-6,rx-6,y+6,1);
    }
    else if (type == BLINK) {
      display.fillRect(lx-8,y,16,2,1);
      display.fillRect(rx-8,y,16,2,1);
    }
    else if (type == SHOCK) {
      display.drawCircle(lx,y,r+3,1); display.drawCircle(rx,y,r+3,1);
      display.fillCircle(lx,y,2,1);   display.fillCircle(rx,y,2,1);
    }
    else if (type == TONGUE) {
      display.fillCircle(lx,y,r,1);
      display.fillCircle(lx+2,y-2,2,0);
      display.fillRect(rx-8,y,16,3,1);
    }
    else if (type == SNEAKY) {
      display.fillCircle(lx,y,r,1); display.fillCircle(rx,y,r,1);
      display.fillRect(lx-10,y-10,20,8,0);
      display.fillRect(rx-10,y-10,20,8,0);
    }
    else if (type == LISTEN) {
      display.fillCircle(lx,y,r,1); display.fillCircle(rx,y,r,1);
      display.fillCircle(lx+2,y-1,3,0); display.fillCircle(rx+2,y-1,3,0);
      display.drawLine(lx-10,y-12,lx+6,y-10,1);
      display.drawLine(rx-6,y-10,rx+10,y-12,1);
    }
    else if (type == SPEAK) {
      display.fillCircle(lx,y,r,1); display.fillCircle(rx,y,r,1);
      display.fillCircle(lx+2,y-2,2,0); display.fillCircle(rx-2,y-2,2,0);
    }
    else if (type == FEAR) {
      display.drawCircle(lx,y,r+4,1); display.drawCircle(rx,y,r+4,1);
      display.fillCircle(lx,y,2,1);   display.fillCircle(rx,y,2,1);
      display.drawLine(lx-10,y-14,lx+2,y-10,1);
      display.drawLine(rx-2,y-10,rx+10,y-14,1);
    }
    else if (type == CRY) {
      display.fillCircle(lx,y,r,1); display.fillCircle(rx,y,r,1);
      display.fillCircle(lx+2,y+2,2,0); display.fillCircle(rx-2,y+2,2,0);
      display.drawLine(lx+6,y+6,lx+6,y+16,1);
      display.drawLine(rx-6,y+6,rx-6,y+16,1);
    }
    else {
      // NORMAL
      display.fillCircle(lx,y,r,1); display.fillCircle(rx,y,r,1);
      display.fillCircle(lx+2,y-2,2,0); display.fillCircle(rx-2,y-2,2,0);
    }

    // ---- MOUTH ----
    if (type == DEAD) {
      display.drawLine(mx-10,my+5,mx+10,my-5,1);
    }
    else if (type == TONGUE) {
      display.fillCircle(mx,my-2,8,1);
      display.fillRect(mx-10,my-12,20,10,0);
      display.fillCircle(mx+2,my+5,4,1);
      display.drawLine(mx+2,my+3,mx+2,my+7,0);
    }
    else if (type == SHOCK || type == SPEAK) {
      display.fillCircle(mx,my+2,6,1);
    }
    else if (type == LISTEN) {
      display.fillRect(mx-8,my+2,16,2,1);
    }
    else if (type == FEAR) {
      display.drawCircle(mx,my+3,5,1);
    }
    else if (type == CRY) {
      display.drawLine(mx-10,my+6,mx+10,my+6,1);
      display.drawLine(mx-10,my+6,mx-6,my+2,1);
      display.drawLine(mx+10,my+6,mx+6,my+2,1);
    }
    else if (type == SNEAKY) {
      display.fillRect(mx-10,my,20,2,1);
    }
    else {
      // normal kapalı ağız
      display.fillCircle(mx,my-2,8,1);
      display.fillRect(mx-10,my-12,20,10,0);
    }

    display.display();
  }

  // Dans animasyonu
  void drawDance(int frame) {
    display.clearDisplay();
    display.drawLine(0,60,128,60,1);

    display.setCursor(30,0);
    display.setTextSize(1);
    display.setTextColor(1);
    display.print("DANS MODU!");

    int cx=64, cy=30;
    display.drawCircle(cx,cy-6,5,1);
    display.drawLine(cx,cy,cx,cy+15,1);

    if (frame==0) {
      display.drawLine(cx,cy+2,cx-15,cy-10,1); display.drawLine(cx,cy+2,cx+15,cy-10,1);
      display.drawLine(cx,cy+15,cx-10,cy+28,1); display.drawLine(cx,cy+15,cx+10,cy+28,1);
    } else if (frame==1) {
      display.drawLine(cx,cy+2,cx-15,cy+5,1);  display.drawLine(cx,cy+2,cx+15,cy+5,1);
      display.drawLine(cx,cy+15,cx-12,cy+25,1); display.drawLine(cx,cy+15,cx+12,cy+25,1);
    } else if (frame==2) {
      display.drawLine(cx,cy+2,cx-15,cy+10,1); display.drawLine(cx,cy+2,cx+15,cy-15,1);
      display.drawLine(cx,cy+15,cx-8,cy+28,1); display.drawLine(cx,cy+15,cx+8,cy+28,1);
    } else {
      display.drawLine(cx,cy+2,cx-15,cy-15,1); display.drawLine(cx,cy+2,cx+15,cy+10,1);
      display.drawLine(cx,cy+15,cx-8,cy+28,1); display.drawLine(cx,cy+15,cx+8,cy+28,1);
    }

    display.display();
  }

  // "Video hissi" veren bomba sahnesi (faz + progress ile)
  // phase:
  // 0: 0-5s fitil + geri sayım
  // 1: 5-7s flash
  // 2: 7-12s patlama
  // 3: 12-30s duman/sonrası
  void drawBombScene(uint8_t phase, uint8_t progress, unsigned long elapsedMs) {
    display.clearDisplay();

    // Basit deterministic pseudo-random (frame'e göre)
    uint32_t seed = 1469598103UL ^ (uint32_t)(elapsedMs / 70UL);
    auto rnd = [&seed]() -> uint8_t {
      seed = seed * 1103515245UL + 12345UL;
      return (uint8_t)((seed >> 16) & 0xFF);
    };

    const int cx = 64;
    const int cy = 34;

    // Zemin
    display.drawLine(0, 60, 127, 60, 1);

    // Faz 0: fitil + geri sayım
    if (phase == 0) {
      display.fillCircle(cx, cy, 10, 1);
      display.fillRect(cx - 3, cy - 18, 6, 8, 1); // üst parça

      // fitil
      display.drawLine(cx + 5, cy - 18, cx + 16, cy - 28, 1);

      // kıvılcım (blink)
      bool spark = ((elapsedMs / 250UL) % 2UL) == 0UL;
      if (spark) {
        display.drawCircle(cx + 18, cy - 30, 2, 1);
        display.drawLine(cx + 18, cy - 33, cx + 18, cy - 27, 1);
        display.drawLine(cx + 15, cy - 30, cx + 21, cy - 30, 1);
      }

      int secLeft = 5 - (int)(elapsedMs / 1000UL);
      if (secLeft < 0) secLeft = 0;

      display.setTextSize(1);
      display.setTextColor(1);
      display.setCursor(0, 0);
      display.print("BOMB MODE");
      display.setCursor(0, 10);
      display.print("T-");
      display.print(secLeft);

      display.display();
      return;
    }

    // Faz 1: flash
    if (phase == 1) {
      bool flash = ((elapsedMs / 120UL) % 2UL) == 0UL;
      if (flash) {
        display.fillRect(0, 0, 128, 64, 1);
        display.setTextColor(0);
      } else {
        display.setTextColor(1);
      }
      display.setTextSize(1);
      display.setCursor(34, 28);
      display.print("!!!");
      display.display();
      return;
    }

    // Faz 2: patlama
    if (phase == 2) {
      int radius = 2 + (int)((progress * 30UL) / 255UL); // 2..32
      display.drawCircle(cx, cy, radius, 1);
      if (radius > 6)  display.drawCircle(cx, cy, radius - 4, 1);
      if (radius > 10) display.drawCircle(cx, cy, radius - 8, 1);

      // şarapnel çizgileri
      for (int i = 0; i < 10; i++) {
        int a = (int)rnd();
        int x1 = cx + (int)((a % 17) - 8);
        int y1 = cy + (int)(((a >> 4) % 17) - 8);
        int x2 = cx + (int)((a % 65) - 32);
        int y2 = cy + (int)(((a >> 2) % 65) - 32);
        display.drawLine(x1, y1, x2, y2, 1);
      }

      display.setTextSize(1);
      display.setTextColor(1);
      display.setCursor(0, 0);
      display.print("BOOOOM!");
      display.display();
      return;
    }

    // Faz 3: duman
    if (phase == 3) {
      int density = 60 - (int)((progress * 45UL) / 255UL); // 60..15
      if (density < 12) density = 12;

      for (int i = 0; i < density; i++) {
        int dx = (int)((rnd() % 81) - 40);
        int dy = (int)((rnd() % 41) - 20);
        int x = cx + dx;
        int yy = cy + dy;

        if (x < 0) x = 0; if (x > 127) x = 127;
        if (yy < 0) yy = 0; if (yy > 59) yy = 59;

        display.drawPixel(x, yy, 1);
      }

      int r2 = 18 - (int)((progress * 10UL) / 255UL);
      if (r2 < 6) r2 = 6;
      display.drawCircle(cx, cy, r2, 1);

      display.setTextSize(1);
      display.setTextColor(1);
      display.setCursor(0, 0);
      display.print("SMOKE...");
      display.display();
      return;
    }

    display.display();
  }
};

#endif