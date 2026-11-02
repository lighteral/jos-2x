/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/Pong_game.ino
  The pong game
	Lighteral (2026)
*/
// Game state
uint8_t       player_score = 0;
uint8_t       mcu_score    = 0;
uint8_t       ball_x       = 53;
uint8_t       ball_y       = 26;
int8_t        ball_dir_x   = 1;
int8_t        ball_dir_y   = 1;
unsigned long ball_update  = 0;
unsigned long paddle_update = 0;
uint8_t       mcu_y        = 16;
uint8_t       player_y     = 16;

void MainGame() {
  mcu_y       = 16;
  player_y    = 16;
  player_score = 0;
  mcu_score    = 0;
  ball_x       = 53;
  ball_y       = 26;
  ball_dir_x   = 1;
  ball_dir_y   = 1;

  display.clearDisplay();
  drawCourt();
  display.display();

  ball_update   = millis();
  paddle_update = ball_update;
  gameloop();
}

void gameloop() {
  bool game_over = false;
  bool win       = false;
  bool up_state  = false;
  bool down_state = false;

  while (true) {
    bool update_needed = false;
    const unsigned long time = millis();

    up_state   |= (digitalRead(UP_BUTTON) == LOW);
    down_state |= (digitalRead(DOWN_BUTTON) == LOW);

    if (time > ball_update) {
      int16_t new_x = (int16_t)ball_x + ball_dir_x;
      int16_t new_y = (int16_t)ball_y + ball_dir_y;

      // Vertical walls (left/right) also score points.
      if (new_x <= 0 || new_x >= SCREEN_WIDTH - 1) {
        ball_dir_x = -ball_dir_x;
        new_x += ball_dir_x + ball_dir_x;

        if (new_x < SCREEN_WIDTH / 2) {
          player_scoreTone();
          ++player_score;
        } else {
          mcu_scoreTone();
          ++mcu_score;
        }

        if (player_score == SCORE_LIMIT || mcu_score == SCORE_LIMIT) {
          win = player_score > mcu_score;
          game_over = true;
        }
      }

      // Horizontal walls.
      if (new_y <= 0 || new_y >= 53) {
        wallTone();
        ball_dir_y = -ball_dir_y;
        new_y += ball_dir_y + ball_dir_y;
      }

      if (new_x == MCU_X && new_y >= mcu_y &&
          new_y < mcu_y + PADDLE_HEIGHT) {
        mcuPaddleTone();
        ball_dir_x = -ball_dir_x;
        new_x += ball_dir_x + ball_dir_x;
      }
      if (new_x == PLAYER_X && new_y >= player_y &&
          new_y < player_y + PADDLE_HEIGHT) {
        playerPaddleTone();
        ball_dir_x = -ball_dir_x;
        new_x += ball_dir_x + ball_dir_x;
      }

      if (new_y < 0) new_y = 0;
      if (new_y > 53) new_y = 53;
      if (new_x < 0) new_x = 0;
      if (new_x > SCREEN_WIDTH - 1) new_x = SCREEN_WIDTH - 1;

      display.drawPixel(ball_x, ball_y, BLACK);
      display.drawPixel(new_x, new_y, WHITE);
      ball_x = (uint8_t)new_x;
      ball_y = (uint8_t)new_y;

      ball_update += BALL_RATE;
      update_needed = true;
    }

    if (time > paddle_update) {
      paddle_update += PADDLE_RATE;

      display.drawFastVLine(MCU_X, mcu_y, PADDLE_HEIGHT, BLACK);
      const uint8_t half_paddle = PADDLE_HEIGHT >> 1;

      if (mcu_y + half_paddle > ball_y) {
        const int8_t dir = ball_x > MCU_X ? -1 : 1;
        mcu_y += dir;
      } else if (mcu_y + half_paddle < ball_y) {
        const int8_t dir = ball_x > MCU_X ? 1 : -1;
        mcu_y += dir;
      }

      if (mcu_y < 1) mcu_y = 1;
      if (mcu_y + PADDLE_HEIGHT > 53) mcu_y = 53 - PADDLE_HEIGHT;
      display.drawFastVLine(MCU_X, mcu_y, PADDLE_HEIGHT, WHITE);

      display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, BLACK);
      if (up_state) --player_y;
      if (down_state) ++player_y;
      up_state = down_state = false;

      if (player_y < 1) player_y = 1;
      if (player_y + PADDLE_HEIGHT > 53) player_y = 53 - PADDLE_HEIGHT;
      display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, WHITE);

      update_needed = true;
    }

    if (update_needed) {
      if (game_over) {
        display.clearDisplay();
        display.setCursor(40, 28);
        display.print(win ? "YOU WIN!!" : "YOU LOSE!");
        display.display();

        const unsigned long waitStart = millis();
        while (millis() - waitStart < 5000) {
          if (digitalRead(BACK_BUTTON) == LOW) {
            return;
          }
          delay(10);
        }

        display.clearDisplay();
        ball_x = 53;
        ball_y = 26;
        ball_dir_x = 1;
        ball_dir_y = 1;
        mcu_y = 16;
        player_y = 16;
        mcu_score = 0;
        player_score = 0;
        game_over = false;
        win = false;
        ball_update = millis();
        paddle_update = ball_update;
        drawCourt();
      }

      display.setTextColor(WHITE, BLACK);
      display.setCursor(0, 56);
      display.print(mcu_score);
      display.setCursor(122, 56);
      display.print(player_score);
      display.display();
    }

    if (digitalRead(BACK_BUTTON) == LOW) {
      return;
    }

    yield();
    delay(1);
  }
}

void beep(uint16_t frequency, uint16_t duration) {
  tone(BUZZER, frequency, duration);
  delay(duration);
  noTone(BUZZER);
}

void playerPaddleTone() { beep(250, 25); }
void mcuPaddleTone()    { beep(225, 25); }
void wallTone()         { beep(200, 25); }

void player_scoreTone() {
  beep(200, 50);
  delay(25);
  beep(250, 25);
}

void mcu_scoreTone() {
  beep(250, 25);
  delay(25);
  beep(200, 25);
}

void drawCourt() {
  display.drawRect(0, 0, SCREEN_WIDTH, 54, WHITE);
}
