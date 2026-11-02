/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/Menu.ino
  Shared menu renderer/navigation. All menu screens use this one function.
	Lighteral (2026)
*/
static constexpr uint8_t MENU_VISIBLE_ROWS = 5;
static constexpr uint8_t MENU_ROW_HEIGHT   = 10;
static constexpr uint8_t MENU_FIRST_ROW    = HEADER_HEIGHT + 2;
static constexpr uint8_t MENU_ROW_WIDTH    = 120;
static constexpr uint8_t MENU_BAR_X        = 122;
static constexpr uint8_t MENU_BAR_Y        = MENU_FIRST_ROW - 1;
static constexpr uint8_t MENU_BAR_WIDTH    = 6;
static constexpr uint8_t MENU_BAR_HEIGHT   = MENU_VISIBLE_ROWS * MENU_ROW_HEIGHT;

int8_t runMenu(const char *const *items, uint8_t count,
               const char *title, int8_t start) {
  if (items == nullptr || count == 0) return -1;

  int8_t selected = start;
  if (selected < 0) selected = 0;
  if (selected >= count) selected = (int8_t)(count - 1);

  uint8_t top = 0;
  if (selected >= MENU_VISIBLE_ROWS) {
    top = (uint8_t)(selected - MENU_VISIBLE_ROWS + 1);
  }

  bool upHeld = false;
  bool downHeld = false;
  unsigned long lastUp = 0;
  unsigned long lastDown = 0;
  bool dirty = true;
  int lastHeaderTime = -1;

  // A button held on the previous screen must not trigger an item here.
  while (digitalRead(SELECT_BUTTON) == LOW ||
         digitalRead(BACK_BUTTON) == LOW) {
    delay(10);
  }

  while (true) {
    updateRTC();

    const int headerTime = hours * 60 + minutes;
    if (headerTime != lastHeaderTime) {
      lastHeaderTime = headerTime;
      dirty = true;
    }

    const bool up    = digitalRead(UP_BUTTON) == LOW;
    const bool down  = digitalRead(DOWN_BUTTON) == LOW;
    const bool enter = digitalRead(SELECT_BUTTON) == LOW;
    const bool back  = digitalRead(BACK_BUTTON) == LOW;
    const unsigned long now = millis();

    if (up) {
      if (!upHeld) {
        selected = (selected == 0) ? (int8_t)(count - 1) : selected - 1;
        upHeld = true;
        lastUp = now;
        dirty = true;
      } else if (now - lastUp >= 300) {
        selected = (selected == 0) ? (int8_t)(count - 1) : selected - 1;
        lastUp = now;
        dirty = true;
      }
    } else {
      upHeld = false;
    }

    if (down) {
      if (!downHeld) {
        selected = (selected == count - 1) ? 0 : selected + 1;
        downHeld = true;
        lastDown = now;
        dirty = true;
      } else if (now - lastDown >= 300) {
        selected = (selected == count - 1) ? 0 : selected + 1;
        lastDown = now;
        dirty = true;
      }
    } else {
      downHeld = false;
    }

    if (selected < top) {
      top = (uint8_t)selected;
    } else if (selected >= top + MENU_VISIBLE_ROWS) {
      top = (uint8_t)(selected - MENU_VISIBLE_ROWS + 1);
    }

    if (dirty) {
      staticHeader(title);

      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);

      for (uint8_t row = 0; row < MENU_VISIBLE_ROWS; ++row) {
        const uint8_t index = top + row;
        if (index >= count) break;

        const uint8_t y = MENU_FIRST_ROW + row * MENU_ROW_HEIGHT;
        if (index == selected) {
          display.fillRect(0, y - 1, MENU_ROW_WIDTH, MENU_ROW_HEIGHT,
                           SSD1306_WHITE);
          display.setTextColor(SSD1306_BLACK);
        } else {
          display.setTextColor(SSD1306_WHITE);
        }
        display.setCursor(3, y);
        display.print(items[index]);
      }

      // Scrollbar: white frame with a white thumb on a black track.
      const uint8_t maxTop = (count > MENU_VISIBLE_ROWS)
                                 ? (uint8_t)(count - MENU_VISIBLE_ROWS)
                                 : 0;
      const uint8_t innerY      = MENU_BAR_Y + 1;
      const uint8_t innerHeight = MENU_BAR_HEIGHT - 2;
      uint8_t thumbHeight = innerHeight;
      if (maxTop > 0) {
        thumbHeight = (uint8_t)(((uint16_t)MENU_VISIBLE_ROWS *
                                 innerHeight) / count);
        if (thumbHeight < 4) thumbHeight = 4;
      }
      const uint8_t travel = innerHeight - thumbHeight;
      const uint8_t thumbY = innerY +
          (maxTop ? (uint8_t)(((uint16_t)top * travel) / maxTop) : 0);

      display.drawRect(MENU_BAR_X, MENU_BAR_Y, MENU_BAR_WIDTH,
                       MENU_BAR_HEIGHT, SSD1306_WHITE);
      display.fillRect(MENU_BAR_X + 1, thumbY, MENU_BAR_WIDTH - 2,
                       thumbHeight, SSD1306_WHITE);

      display.display();
      dirty = false;
    }

    if (enter) {
      const int8_t result = selected;
      while (digitalRead(SELECT_BUTTON) == LOW) delay(10);
      return result;
    }

    if (back) {
      while (digitalRead(BACK_BUTTON) == LOW) delay(10);
      return -1;
    }

    delay(5);
    yield();
  }
}

void MainMenu() {
  static const char *const items[] = {
      "Utilities/",
      "Games/",
      "Filesystem/",
      "SuperMario Song"
  };
  const uint8_t itemCount = sizeof(items) / sizeof(items[0]);

  int8_t selection = 0;
  for (;;) {
    const int8_t choice = runMenu(items, itemCount, "MENU", selection);
    if (choice < 0) return; // BACK returns to the home screen.

    selection = choice;
    switch (choice) {
      case 0: UtilitiesMenu(); break;
      case 1: GamesMenu(); break;
      case 2: fs(); break;
      case 3: MainMusic(); break;
      default: break;
    }
  }
}

void UtilitiesMenu() {
  static const char *const items[] = {
      "Clock",
      "Weather",
      "Calculator",
      "About This MCU"
  };
  const uint8_t itemCount = sizeof(items) / sizeof(items[0]);

  int8_t selection = 0;
  for (;;) {
    const int8_t choice = runMenu(items, itemCount, "UTILITIES", selection);
    if (choice < 0) return; // BACK returns to the root menu.

    selection = choice;
    switch (choice) {
      case 0: MainClock(); break;
      case 1: MainTemp(); break;
      case 2: MainCalculator(); break;
      case 3: aboutScreen(); break;
      default: break;
    }
  }
}

void GamesMenu() {
  static const char *const items[] = {
      "Flappy Bird",
      GAME ? "The Pong Game" : "Pong unavailable",
      "Platformer",
      "Noughts & Crosses"
  };
  const uint8_t itemCount = sizeof(items) / sizeof(items[0]);

  int8_t selection = 0;
  for (;;) {
    const int8_t choice = runMenu(items, itemCount, "GAMES", selection);
    if (choice < 0) return; // BACK returns to the root menu.

    selection = choice;
    switch (choice) {
      case 0: MainFlappy(); break;
      case 1: if (GAME) MainGame(); break;
      case 2: MainPlatformer(); break;
      case 3: MainTicTacToe(); break;
      default: break;
    }
  }
}

void aboutScreen() {
  static const char *const lines[] = {
      "JOS " JOS_VERSION,
      "by Lighteral",
      "Device: " DEVICE_NAME,
      "Board: " JOS_BOARD_NAME,
      "CPU: " JOS_PROCESSOR,
      "EEPROM FS " JOS_EEPROM_FS_VERSION,
      "Display: SSD1306",
      "Screen: 128 x 64",
      "Input: 4 buttons",
      "UP/DOWN to scroll",
      "BACK to return"
  };
  const uint8_t lineCount = sizeof(lines) / sizeof(lines[0]);
  const uint8_t maxTop = (lineCount > MENU_VISIBLE_ROWS)
                             ? (uint8_t)(lineCount - MENU_VISIBLE_ROWS)
                             : 0;

  uint8_t top = 0;
  bool upHeld = false;
  bool downHeld = false;
  unsigned long lastUp = 0;
  unsigned long lastDown = 0;
  bool dirty = true;
  int lastHeaderTime = -1;

  while (digitalRead(SELECT_BUTTON) == LOW ||
         digitalRead(BACK_BUTTON) == LOW) {
    delay(10);
  }

  while (true) {
    updateRTC();

    const int headerTime = hours * 60 + minutes;
    if (headerTime != lastHeaderTime) {
      lastHeaderTime = headerTime;
      dirty = true;
    }

    const bool up   = digitalRead(UP_BUTTON) == LOW;
    const bool down = digitalRead(DOWN_BUTTON) == LOW;
    const bool back = digitalRead(BACK_BUTTON) == LOW;
    const unsigned long now = millis();

    if (up) {
      if (!upHeld) {
        if (top > 0) {
          --top;
          dirty = true;
        }
        upHeld = true;
        lastUp = now;
      } else if (now - lastUp >= 300) {
        if (top > 0) {
          --top;
          dirty = true;
        }
        lastUp = now;
      }
    } else {
      upHeld = false;
    }

    if (down) {
      if (!downHeld) {
        if (top < maxTop) {
          ++top;
          dirty = true;
        }
        downHeld = true;
        lastDown = now;
      } else if (now - lastDown >= 300) {
        if (top < maxTop) {
          ++top;
          dirty = true;
        }
        lastDown = now;
      }
    } else {
      downHeld = false;
    }

    if (back) {
      while (digitalRead(BACK_BUTTON) == LOW) delay(10);
      return;
    }

    if (dirty) {
      staticHeader("ABOUT");

      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);

      for (uint8_t row = 0; row < MENU_VISIBLE_ROWS; ++row) {
        const uint8_t index = top + row;
        if (index >= lineCount) break;
        display.setCursor(3, MENU_FIRST_ROW + row * MENU_ROW_HEIGHT);
        display.print(lines[index]);
      }

      const uint8_t innerY      = MENU_BAR_Y + 1;
      const uint8_t innerHeight = MENU_BAR_HEIGHT - 2;
      uint8_t thumbHeight = innerHeight;
      if (maxTop > 0) {
        thumbHeight = (uint8_t)(((uint16_t)MENU_VISIBLE_ROWS *
                                 innerHeight) / lineCount);
        if (thumbHeight < 4) thumbHeight = 4;
      }
      const uint8_t travel = innerHeight - thumbHeight;
      const uint8_t thumbY = innerY +
          (maxTop ? (uint8_t)(((uint16_t)top * travel) / maxTop) : 0);

      // Scrollbar: white frame with a white thumb on a black track.
      display.drawRect(MENU_BAR_X, MENU_BAR_Y, MENU_BAR_WIDTH,
                       MENU_BAR_HEIGHT, SSD1306_WHITE);
      display.fillRect(MENU_BAR_X + 1, thumbY, MENU_BAR_WIDTH - 2,
                       thumbHeight, SSD1306_WHITE);

      display.display();
      dirty = false;
    }

    delay(5);
    yield();
  }
}
