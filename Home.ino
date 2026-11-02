/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/Home.ino
  The home screen
	Lighteral (2026)
*/
#include <string.h>

static constexpr uint8_t HOME_BUTTON_COUNT = 3;
static constexpr uint8_t HOME_BUTTON_Y = 42;
static constexpr uint8_t HOME_BUTTON_W = 38;
static constexpr uint8_t HOME_BUTTON_H = 18;
static constexpr uint8_t HOME_BUTTON_RADIUS = 3;
static const uint8_t HOME_BUTTON_X[HOME_BUTTON_COUNT] = { 3, 45, 87 };

static void homeDraw(uint8_t selection) {
  display.clearDisplay();

  // Large clock: HH:MM:SS.
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(16, 6);
  printPad2((uint8_t)hours);
  display.print(':');
  printPad2((uint8_t)minutes);
  display.print(':');
  printPad2((uint8_t)seconds);

  // A real RTC has a date; the software clock only keeps time.
  display.setTextSize(1);
  if (date > 0 && month > 0) {
    display.setCursor(34, 26);
    printPad2((uint8_t)date);
    display.print('/');
    printPad2((uint8_t)month);
    display.print("/20");
    printPad2((uint8_t)year);
  } else {
    const char *label = "JOS " JOS_VERSION;
    display.setCursor((SCREEN_WIDTH - (int)strlen(label) * 6) / 2, 26);
    display.print(label);
  }

  display.drawFastHLine(4, 38, 120, SSD1306_WHITE);

  const char *labels[HOME_BUTTON_COUNT] = { "MENU", "FILES", "SLEEP" };

  for (uint8_t i = 0; i < HOME_BUTTON_COUNT; ++i) {
    const uint8_t x = HOME_BUTTON_X[i];

    if (i == selection) {
      display.fillRoundRect(x, HOME_BUTTON_Y, HOME_BUTTON_W,
                            HOME_BUTTON_H, HOME_BUTTON_RADIUS,
                            SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.drawRoundRect(x, HOME_BUTTON_Y, HOME_BUTTON_W,
                            HOME_BUTTON_H, HOME_BUTTON_RADIUS,
                            SSD1306_WHITE);
      display.setTextColor(SSD1306_WHITE);
    }

    const uint8_t labelLen = (uint8_t)strlen(labels[i]);
    display.setCursor(x + (HOME_BUTTON_W - labelLen * 6) / 2,
                      HOME_BUTTON_Y + 5);
    display.print(labels[i]);
  }

  display.setTextColor(SSD1306_WHITE);
  display.display();
}

static void powerOff() {
  // Do not let the button that opened this screen wake the display again.
  while (digitalRead(SELECT_BUTTON) == LOW) delay(10);

  beep(392, 70);
  beep(294, 70);
  beep(196, 140);

  noTone(BUZZER);
  digitalWrite(BUZZER, LOW);

  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);

  // The RP2040 has no onboard power latch, so this is a screen-off sleep.
  // Any of the four buttons wakes the device and returns to the home screen.
  for (;;) {
    if (digitalRead(UP_BUTTON) == LOW ||
        digitalRead(DOWN_BUTTON) == LOW ||
        digitalRead(SELECT_BUTTON) == LOW ||
        digitalRead(BACK_BUTTON) == LOW) {
      break;
    }
    delay(50);
  }

  display.ssd1306_command(SSD1306_DISPLAYON);

  // Wait for the wake button to be released.
  while (digitalRead(UP_BUTTON) == LOW ||
         digitalRead(DOWN_BUTTON) == LOW ||
         digitalRead(SELECT_BUTTON) == LOW ||
         digitalRead(BACK_BUTTON) == LOW) {
    delay(10);
  }
  delay(100);
}

void MainHome() {
  uint8_t selection = 0;
  bool upHeld    = false;
  bool downHeld  = false;
  bool selectHeld = false;
  bool dirty = true;
  short lastSecond = -1;

  // Ignore a button still held from the previous screen.
  while (digitalRead(SELECT_BUTTON) == LOW ||
         digitalRead(BACK_BUTTON) == LOW) {
    delay(10);
  }

  for (;;) {
    updateRTC();

    if (seconds != lastSecond) {
      lastSecond = seconds;
      dirty = true;
    }

    const bool up     = digitalRead(UP_BUTTON) == LOW;
    const bool down   = digitalRead(DOWN_BUTTON) == LOW;
    const bool select = digitalRead(SELECT_BUTTON) == LOW;

    if (up && !upHeld) {
      upHeld = true;
      selection = (selection + HOME_BUTTON_COUNT - 1) % HOME_BUTTON_COUNT;
      dirty = true;
    } else if (!up) {
      upHeld = false;
    }

    if (down && !downHeld) {
      downHeld = true;
      selection = (selection + 1) % HOME_BUTTON_COUNT;
      dirty = true;
    } else if (!down) {
      downHeld = false;
    }

    if (select) {
      if (!selectHeld) {
        selectHeld = true;

        if (selection == 0) {
          while (digitalRead(SELECT_BUTTON) == LOW) delay(10);
          MainMenu();
          dirty = true;
        } else if (selection == 1) {
          while (digitalRead(SELECT_BUTTON) == LOW) delay(10);
          fs();
          dirty = true;
        } else {
          powerOff();
          dirty = true;
        }
      }
    } else {
      selectHeld = false;
    }

    if (dirty) {
      homeDraw(selection);
      dirty = false;
    }

    delay(10);
    yield();
  }
}
