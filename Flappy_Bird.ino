/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/Flappy_Bird.ino
  The Flappy Bird clone game
	Lighteral (2026)
*/

static constexpr uint8_t FLAPPY_TOP_Y        = HEADER_HEIGHT + 2;
static constexpr uint8_t FLAPPY_GROUND_Y     = SCREEN_HEIGHT - 6;
static constexpr uint8_t FLAPPY_BIRD_X       = 26;
static constexpr uint8_t FLAPPY_BIRD_W       = 8;
static constexpr uint8_t FLAPPY_BIRD_H       = 7;
static constexpr uint8_t FLAPPY_PIPE_W       = 12;
static constexpr uint8_t FLAPPY_PIPE_GAP     = 24;
static constexpr uint8_t FLAPPY_PIPE_COUNT   = 3;
static constexpr uint8_t FLAPPY_PIPE_SPACING = 56;
static constexpr uint8_t FLAPPY_PIPE_SPEED   = 2;

static constexpr unsigned long FLAPPY_FRAME_MS = 40;
static constexpr float FLAPPY_GRAVITY          = 0.30f;
static constexpr float FLAPPY_FLAP_VELOCITY    = -3.6f;
static constexpr float FLAPPY_MAX_FALL         = 4.5f;

static uint8_t flappyGapY() {
  return (uint8_t)random(FLAPPY_TOP_Y + 6,
                         FLAPPY_GROUND_Y - FLAPPY_PIPE_GAP - 4);
}

static void flappyDraw(float birdY, const FlappyPipe *pipes, uint16_t score) {
  updateRTC();
  staticHeader("FLAPPY BIRD");

  display.setTextColor(BLACK, WHITE);
  display.setCursor(72, (HEADER_HEIGHT - 8) / 2);
  display.print(score);
  display.setTextColor(WHITE);

  // Ground strip with a little texture.
  display.fillRect(0, FLAPPY_GROUND_Y, SCREEN_WIDTH,
                   SCREEN_HEIGHT - FLAPPY_GROUND_Y, WHITE);
  for (uint8_t x = 2; x < SCREEN_WIDTH; x += 10) {
    display.drawPixel(x, FLAPPY_GROUND_Y + 2, BLACK);
  }

  for (uint8_t i = 0; i < FLAPPY_PIPE_COUNT; ++i) {
    const int16_t x = pipes[i].x;
    if (x + FLAPPY_PIPE_W < 0 || x >= SCREEN_WIDTH) continue;

    const int16_t topHeight = (int16_t)pipes[i].gapY - FLAPPY_TOP_Y;
    if (topHeight > 0) {
      display.fillRect(x, FLAPPY_TOP_Y, FLAPPY_PIPE_W, topHeight, WHITE);
      display.fillRect(x - 1, pipes[i].gapY - 4, FLAPPY_PIPE_W + 2, 4, WHITE);
    }

    const int16_t bottomY = (int16_t)pipes[i].gapY + FLAPPY_PIPE_GAP;
    if (bottomY < FLAPPY_GROUND_Y) {
      display.fillRect(x, bottomY, FLAPPY_PIPE_W,
                       FLAPPY_GROUND_Y - bottomY, WHITE);
      display.fillRect(x - 1, bottomY, FLAPPY_PIPE_W + 2, 4, WHITE);
    }
  }

  // Bird: black body with a white outline so it stays visible over pipes.
  const int16_t y = (int16_t)(birdY + 0.5f);
  display.fillRect(FLAPPY_BIRD_X, y, FLAPPY_BIRD_W, FLAPPY_BIRD_H, BLACK);
  display.drawRect(FLAPPY_BIRD_X, y, FLAPPY_BIRD_W, FLAPPY_BIRD_H, WHITE);
  display.drawPixel(FLAPPY_BIRD_X + 5, y + 2, WHITE); // eye
  display.drawPixel(FLAPPY_BIRD_X + 6, y + 3, WHITE); // beak

  display.display();
}

static void flappyStartOverlay() {
  display.fillRect(14, 22, 100, 30, BLACK);
  display.drawRect(14, 22, 100, 30, WHITE);
  display.setTextColor(WHITE);
  display.setCursor(20, 27);
  display.print("UP/SELECT: FLAP");
  display.setCursor(34, 37);
  display.print("BACK: EXIT");
  display.display();
}

static void flappyGameOver(uint16_t score) {
  display.fillRect(10, 16, 108, 44, BLACK);
  display.drawRect(10, 16, 108, 44, WHITE);
  display.setTextColor(WHITE);
  display.setCursor(37, 20);
  display.print("GAME OVER");
  display.setCursor(37, 30);
  display.print("SCORE ");
  display.print(score);
  display.setCursor(34, 40);
  display.print("SEL: RETRY");
  display.setCursor(34, 49);
  display.print("BACK: EXIT");
  display.display();
}

void MainFlappy() {
  static FlappyPipe pipes[FLAPPY_PIPE_COUNT];
  const float startY = (FLAPPY_TOP_Y + FLAPPY_GROUND_Y - FLAPPY_BIRD_H) * 0.5f;

  for (;;) {
    float birdY = startY;
    float velocity = 0.0f;
    uint16_t score = 0;
    bool flapWasDown = false;
    bool started = false;
    unsigned long nextFrame = millis();

    randomSeed(millis() ^ 0x5A5A5A5AUL);
    for (uint8_t i = 0; i < FLAPPY_PIPE_COUNT; ++i) {
      pipes[i].x = SCREEN_WIDTH + 10 + i * FLAPPY_PIPE_SPACING;
      pipes[i].gapY = flappyGapY();
      pipes[i].scored = false;
    }

    flappyDraw(birdY, pipes, score);
    flappyStartOverlay();

    while (!started) {
      if (digitalRead(BACK_BUTTON) == LOW) {
        while (digitalRead(BACK_BUTTON) == LOW) delay(10);
        return;
      }

      const bool flapDown = digitalRead(UP_BUTTON) == LOW ||
                            digitalRead(SELECT_BUTTON) == LOW;
      if (flapDown && !flapWasDown) {
        started = true;
        velocity = FLAPPY_FLAP_VELOCITY;
        flappyDraw(birdY, pipes, score);
        nextFrame = millis() + FLAPPY_FRAME_MS;
      }
      flapWasDown = flapDown;
      delay(10);
    }

    bool gameOver = false;
    while (!gameOver) {
      if (digitalRead(BACK_BUTTON) == LOW) {
        while (digitalRead(BACK_BUTTON) == LOW) delay(10);
        return;
      }

      const bool flapDown = digitalRead(UP_BUTTON) == LOW ||
                            digitalRead(SELECT_BUTTON) == LOW;
      if (flapDown && !flapWasDown) {
        velocity = FLAPPY_FLAP_VELOCITY;
        beep(700, 12);
      }
      flapWasDown = flapDown;

      if ((long)(millis() - nextFrame) < 0) {
        delay(2);
        continue;
      }
      nextFrame += FLAPPY_FRAME_MS;

      velocity += FLAPPY_GRAVITY;
      if (velocity > FLAPPY_MAX_FALL) velocity = FLAPPY_MAX_FALL;
      birdY += velocity;

      if (birdY < FLAPPY_TOP_Y) {
        birdY = FLAPPY_TOP_Y;
        velocity = 0.0f;
      }
      if (birdY + FLAPPY_BIRD_H >= FLAPPY_GROUND_Y) {
        birdY = FLAPPY_GROUND_Y - FLAPPY_BIRD_H;
        gameOver = true;
      }

      for (uint8_t i = 0; i < FLAPPY_PIPE_COUNT; ++i) {
        pipes[i].x -= FLAPPY_PIPE_SPEED;

        if (!pipes[i].scored &&
            pipes[i].x + FLAPPY_PIPE_W < FLAPPY_BIRD_X) {
          pipes[i].scored = true;
          ++score;
          beep(1000, 25);
        }

        if (pipes[i].x + FLAPPY_PIPE_W < -4) {
          pipes[i].x += FLAPPY_PIPE_COUNT * FLAPPY_PIPE_SPACING;
          pipes[i].gapY = flappyGapY();
          pipes[i].scored = false;
        }

        const bool overlapX =
            FLAPPY_BIRD_X + FLAPPY_BIRD_W > pipes[i].x &&
            FLAPPY_BIRD_X < pipes[i].x + FLAPPY_PIPE_W;
        if (overlapX) {
          const bool insideGap =
              birdY >= (float)pipes[i].gapY &&
              birdY + FLAPPY_BIRD_H <=
                  (float)(pipes[i].gapY + FLAPPY_PIPE_GAP);
          if (!insideGap) gameOver = true;
        }
      }

      flappyDraw(birdY, pipes, score);
      if (gameOver) beep(110, 180);
      yield();
    }

    flappyGameOver(score);

    // Let the player release the button, then offer retry or exit.
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
