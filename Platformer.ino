/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/Platfomer.ino
  Side scrolling platformer
	Lighteral (2026)
*/
#include "JOS_games.h"
static constexpr uint8_t PF_PLAYER_W        = 6;
static constexpr uint8_t PF_PLAYER_H        = 9;
static constexpr uint8_t PF_PLAYER_SCREEN_X = 30;
static constexpr uint8_t PF_GROUND_Y        = SCREEN_HEIGHT - 6;
static constexpr uint8_t PF_PLATFORM_COUNT  = 12;

static constexpr unsigned long PF_FRAME_MS = 30;
static constexpr int32_t PF_RUN_STEP       = 1;
static constexpr float PF_GRAVITY          = 0.45f;
static constexpr float PF_FAST_FALL        = 0.35f;
static constexpr float PF_JUMP_VELOCITY    = -5.0f;
static constexpr float PF_MAX_FALL         = 6.0f;

static void pfDraw(int32_t camera, const PfPlatform *platforms,
                   float playerY, uint16_t distance) {
  updateRTC();
  staticHeader("PLATFORMER");

  display.setTextColor(BLACK, WHITE);
  display.setCursor(72, (HEADER_HEIGHT - 8) / 2);
  display.print(distance % 10000);
  display.setTextColor(WHITE);

  for (uint8_t i = 0; i < PF_PLATFORM_COUNT; ++i) {
    const int32_t sx32 = platforms[i].x - camera;
    if (sx32 + platforms[i].w < 0 || sx32 >= SCREEN_WIDTH) continue;
    const int16_t sx = (int16_t)sx32;

    if (platforms[i].y >= PF_GROUND_Y) {
      display.fillRect(sx, platforms[i].y, platforms[i].w,
                       SCREEN_HEIGHT - platforms[i].y, WHITE);
    } else {
      // Floating platforms are thin ledges you can jump onto.
      display.fillRect(sx, platforms[i].y, platforms[i].w, 3, WHITE);
    }
  }

  const int16_t py = (int16_t)(playerY + 0.5f);
  display.fillRect(PF_PLAYER_SCREEN_X, py, PF_PLAYER_W, PF_PLAYER_H, BLACK);
  display.drawRect(PF_PLAYER_SCREEN_X, py, PF_PLAYER_W, PF_PLAYER_H, WHITE);
  display.fillRect(PF_PLAYER_SCREEN_X + 3, py + 2, 2, 2, WHITE);

  display.display();
}

static void pfStartOverlay() {
  display.fillRect(12, 14, 104, 44, BLACK);
  display.drawRect(12, 14, 104, 44, WHITE);
  display.setTextColor(WHITE);
  display.setCursor(22, 18);
  display.print("AUTO RUN RIGHT");
  display.setCursor(22, 28);
  display.print("UP/SEL: JUMP");
  display.setCursor(22, 38);
  display.print("DOWN: FAST FALL");
  display.setCursor(22, 48);
  display.print("BACK: EXIT");
  display.display();
}

static void pfGameOver(uint32_t distance) {
  display.fillRect(12, 16, 104, 44, BLACK);
  display.drawRect(12, 16, 104, 44, WHITE);
  display.setTextColor(WHITE);
  display.setCursor(37, 20);
  display.print("GAME OVER");
  display.setCursor(34, 30);
  display.print("DIST ");
  display.print(distance);
  display.setCursor(34, 40);
  display.print("SEL: RETRY");
  display.setCursor(34, 48);
  display.print("BACK: EXIT");
  display.display();
}

void MainPlatformer() {
  static PfPlatform platforms[PF_PLATFORM_COUNT];
  const float startY = PF_GROUND_Y - PF_PLAYER_H;

  for (;;) {
    int32_t worldX = PF_PLAYER_SCREEN_X;
    float playerY = startY;
    float velocity = 0.0f;
    bool grounded = true;
    bool jumpWasDown = false;
    bool gameOver = false;
    uint8_t genSlot = 1;
    int32_t spawnX = 72;
    unsigned long nextFrame = millis();

    randomSeed(millis() ^ 0x9E3779B9UL);

    // A safe starting ledge, then a recycled ring of generated platforms.
    platforms[0].x = -8;
    platforms[0].y = PF_GROUND_Y;
    platforms[0].w = 80;
    for (uint8_t i = 1; i < PF_PLATFORM_COUNT; ++i) {
      platforms[i].x = -1000;
      platforms[i].y = PF_GROUND_Y;
      platforms[i].w = 1;
    }

    int32_t camera = worldX - PF_PLAYER_SCREEN_X;
    while (spawnX < camera + SCREEN_WIDTH + 48) {
      PfPlatform &p = platforms[genSlot];
      genSlot = (uint8_t)((genSlot + 1) % PF_PLATFORM_COUNT);

      spawnX += (int32_t)random(8, 17);
      p.x = spawnX;
      p.w = (uint8_t)random(24, 44);
      p.y = PF_GROUND_Y;
      if (random(100) < 35) {
        p.y = (uint8_t)(PF_GROUND_Y - random(14, 23));
      }
      spawnX += p.w;
    }

    pfDraw(camera, platforms, playerY,
           (uint16_t)((worldX - PF_PLAYER_SCREEN_X) / 10));
    pfStartOverlay();

    bool started = false;
    bool startWasDown = false;
    while (!started) {
      if (digitalRead(BACK_BUTTON) == LOW) {
        while (digitalRead(BACK_BUTTON) == LOW) delay(10);
        return;
      }

      const bool jumpDown = digitalRead(UP_BUTTON) == LOW ||
                            digitalRead(SELECT_BUTTON) == LOW;
      if (jumpDown && !startWasDown) {
        started = true;
        grounded = false;
        velocity = PF_JUMP_VELOCITY;
      }
      startWasDown = jumpDown;
      delay(10);
    }

    jumpWasDown = true;
    nextFrame = millis() + PF_FRAME_MS;

    while (!gameOver) {
      if (digitalRead(BACK_BUTTON) == LOW) {
        while (digitalRead(BACK_BUTTON) == LOW) delay(10);
        return;
      }

      const bool jumpDown = digitalRead(UP_BUTTON) == LOW ||
                            digitalRead(SELECT_BUTTON) == LOW;
      if (jumpDown && !jumpWasDown && grounded) {
        velocity = PF_JUMP_VELOCITY;
        grounded = false;
      }
      jumpWasDown = jumpDown;

      if ((long)(millis() - nextFrame) < 0) {
        delay(2);
        continue;
      }
      nextFrame += PF_FRAME_MS;

      const bool fastFall = digitalRead(DOWN_BUTTON) == LOW;
      worldX += PF_RUN_STEP;
      velocity += PF_GRAVITY + (fastFall ? PF_FAST_FALL : 0.0f);
      if (velocity > PF_MAX_FALL) velocity = PF_MAX_FALL;

      const float previousY = playerY;
      playerY += velocity;

      // One-way platforms: land only while moving downwards.
      grounded = false;
      bool landed = false;
      uint8_t landedY = 0;
      for (uint8_t i = 0; i < PF_PLATFORM_COUNT; ++i) {
        const PfPlatform &p = platforms[i];
        if (worldX + PF_PLAYER_W <= p.x || worldX >= p.x + p.w) continue;
        if (velocity >= 0.0f &&
            previousY + PF_PLAYER_H <= (float)p.y &&
            playerY + PF_PLAYER_H >= (float)p.y) {
          if (!landed || p.y < landedY) {
            landed = true;
            landedY = p.y;
          }
        }
      }
      if (landed) {
        playerY = (float)landedY - PF_PLAYER_H;
        velocity = 0.0f;
        grounded = true;
      }

      camera = worldX - PF_PLAYER_SCREEN_X;
      while (spawnX < camera + SCREEN_WIDTH + 48) {
        PfPlatform &p = platforms[genSlot];
        genSlot = (uint8_t)((genSlot + 1) % PF_PLATFORM_COUNT);

        spawnX += (int32_t)random(8, 17);
        p.x = spawnX;
        p.w = (uint8_t)random(24, 44);
        p.y = PF_GROUND_Y;
        if (random(100) < 35) {
          p.y = (uint8_t)(PF_GROUND_Y - random(14, 23));
        }
        spawnX += p.w;
      }

      if (playerY > SCREEN_HEIGHT) {
        gameOver = true;
        beep(110, 180);
        break;
      }

      pfDraw(camera, platforms, playerY,
             (uint16_t)((worldX - PF_PLAYER_SCREEN_X) / 10));
      yield();
    }

    pfGameOver((uint32_t)((worldX - PF_PLAYER_SCREEN_X) / 10));

    delay(250);
    while (digitalRead(SELECT_BUTTON) == LOW) delay(10);
    while (digitalRead(SELECT_BUTTON) == HIGH &&
           digitalRead(BACK_BUTTON) == HIGH) {
      delay(10);
    }
    if (digitalRead(BACK_BUTTON) == LOW) {
      while (digitalRead(BACK_BUTTON) == LOW) delay(10);
      return;
    }
    while (digitalRead(SELECT_BUTTON) == LOW) delay(10);
  }
}
