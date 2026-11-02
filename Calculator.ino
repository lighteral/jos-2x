/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/Calculator.ino
  The calculator application
	Lighteral (2026)
*/
#include <stdio.h>
#include <string.h>

static constexpr uint8_t CALC_KEY_COUNT = 18;
static constexpr uint8_t CALC_EXPR_MAX  = 40;
static constexpr uint8_t CALC_NUM_MAX   = 20;
static constexpr char    CALC_KEY_00    = 'z';

// 5 columns x 4 rows. The bottom row has wide 0 and 00 keys plus =.
// '\b' is the backspace key and is labelled "DEL" on screen.
struct CalcKey {
  char key;
  uint8_t x;
  uint8_t y;
  uint8_t w;
};

// The keypad is navigated by UP/DOWN in reading order, SELECT presses a key.
static const CalcKey CALC_KEYS[CALC_KEY_COUNT] = {
    {'7',   3, 29, 23}, {'8',  28, 29, 23}, {'9',  53, 29, 23},
    {'/',  78, 29, 23}, {'\b', 103, 29, 23},
    {'4',   3, 38, 23}, {'5',  28, 38, 23}, {'6',  53, 38, 23},
    {'*',  78, 38, 23}, {'C', 103, 38, 23},
    {'1',   3, 47, 23}, {'2',  28, 47, 23}, {'3',  53, 47, 23},
    {'-',  78, 47, 23}, {'+', 103, 47, 23},
    {'0',   3, 56, 48}, {CALC_KEY_00, 53, 56, 48}, {'=', 103, 56, 23}
};

static char    calcExpr[CALC_EXPR_MAX];
static uint8_t calcLen           = 0;
static bool    calcJustEvaluated = false;
static bool    calcError         = false;

static bool calcIsOperator(char c) {
  return c == '+' || c == '-' || c == '*' || c == '/';
}

static void calcReset() {
  calcLen           = 0;
  calcExpr[0]       = '\0';
  calcJustEvaluated = false;
  calcError         = false;
}

// --- Recursive-descent evaluator (handles * and / before + and -). ----------
static float calcParseExpr(const char **p);

static float calcParseFactor(const char **p) {
  while (**p == ' ') ++*p;

  float value = 0.0f;
  while (**p >= '0' && **p <= '9') {
    value = value * 10.0f + (float)(**p - '0');
    ++*p;
  }
  return value;
}

static float calcParseTerm(const char **p) {
  float value = calcParseFactor(p);

  for (;;) {
    while (**p == ' ') ++*p;
    const char op = **p;
    if (op != '*' && op != '/') break;
    ++*p;

    const float rhs = calcParseFactor(p);
    if (op == '*') {
      value *= rhs;
    } else {
      if (rhs == 0.0f) {
        calcError = true;
        return 0.0f;
      }
      value /= rhs;
    }
  }
  return value;
}

static float calcParseExpr(const char **p) {
  float value = calcParseTerm(p);

  for (;;) {
    while (**p == ' ') ++*p;
    const char op = **p;
    if (op != '+' && op != '-') break;
    ++*p;

    const float rhs = calcParseTerm(p);
    if (op == '+') value += rhs;
    else           value -= rhs;
  }
  return value;
}

// Format with at most three decimal places, trimming trailing zeros.
static void calcFormat(float value, char *out, size_t size) {
  if (calcError || value != value || value > 999999999.0f ||
      value < -999999999.0f) {
    strncpy(out, "ERROR", size);
    out[size - 1] = '\0';
    return;
  }

  const bool negative = value < 0.0f;
  if (negative) value = -value;

  unsigned long whole = (unsigned long)value;
  unsigned long frac =
      (unsigned long)((value - (float)whole) * 1000.0f + 0.5f);
  if (frac >= 1000UL) {
    ++whole;
    frac -= 1000UL;
  }

  char temp[24];
  if (frac == 0UL) {
    if (negative) snprintf(temp, sizeof(temp), "-%lu", whole);
    else          snprintf(temp, sizeof(temp), "%lu", whole);
  } else {
    if (negative) {
      snprintf(temp, sizeof(temp), "-%lu.%03lu", whole, frac);
    } else {
      snprintf(temp, sizeof(temp), "%lu.%03lu", whole, frac);
    }

    size_t n = strlen(temp);
    while (n > 0 && temp[n - 1] == '0') temp[--n] = '\0';
    if (n > 0 && temp[n - 1] == '.') temp[--n] = '\0';
  }

  strncpy(out, temp, size);
  out[size - 1] = '\0';
}

static bool calcHasOperator() {
  for (uint8_t i = 0; i < calcLen; ++i) {
    if (calcIsOperator(calcExpr[i])) return true;
  }
  return false;
}

// Builds the live result preview. Returns false when there is nothing to show.
static bool calcPreview(char *out, size_t size) {
  if (calcLen == 0) return false;
  if (calcIsOperator(calcExpr[calcLen - 1])) return false;
  if (!calcHasOperator()) return false;

  calcError = false;
  const char *p = calcExpr;
  const float value = calcParseExpr(&p);
  calcFormat(value, out, size);
  calcError = false;
  return true;
}

// Appends the "00" key, keeping leading zeros out of the expression.
static void calcAppendZeros() {
  if (calcJustEvaluated) {
    calcLen           = 0;
    calcExpr[0]       = '\0';
    calcJustEvaluated = false;
  }

  if (calcLen == 0) {
    calcExpr[0] = '0';
    calcExpr[1] = '\0';
    calcLen     = 1;
    return;
  }

  const char last = calcExpr[calcLen - 1];
  const bool afterOperator = calcIsOperator(last);

  if (calcLen == 1 && calcExpr[0] == '0' && !afterOperator) return;

  const uint8_t count = afterOperator ? 1 : 2;
  for (uint8_t i = 0; i < count; ++i) {
    if (calcLen >= CALC_EXPR_MAX - 1) break;
    calcExpr[calcLen++] = '0';
    calcExpr[calcLen]   = '\0';
  }
}

static void calcPress(char key) {
  // A previous error is cleared by the next key press.
  if (calcError) calcReset();

  if (key == '\b') {
    if (calcLen > 0) {
      calcExpr[--calcLen] = '\0';
      calcJustEvaluated   = false;
      beep(420, 12);
    }
    return;
  }

  if (key == 'C') {
    calcReset();
    beep(320, 20);
    return;
  }

  if (key == '=') {
    if (calcLen > 0 && !calcIsOperator(calcExpr[calcLen - 1])) {
      char valueText[CALC_NUM_MAX];
      calcError = false;
      const char *p = calcExpr;
      const float value = calcParseExpr(&p);
      calcFormat(value, valueText, sizeof(valueText));

      if (calcError || strcmp(valueText, "ERROR") == 0) {
        calcReset();
        calcError = true;
        beep(180, 60);
      } else {
        const size_t n = strlen(valueText);
        if (n < CALC_EXPR_MAX) {
          strcpy(calcExpr, valueText);
          calcLen           = (uint8_t)n;
          calcJustEvaluated = true;
          beep(900, 30);
        }
      }
    }
    return;
  }

  if (calcIsOperator(key)) {
    if (calcLen == 0) return;
    calcJustEvaluated = false;

    if (calcIsOperator(calcExpr[calcLen - 1])) {
      calcExpr[calcLen - 1] = key;       // replace a pending operator
    } else if (calcLen < CALC_EXPR_MAX - 1) {
      calcExpr[calcLen++] = key;
      calcExpr[calcLen]   = '\0';
    }
    beep(620, 12);
    return;
  }

  if (key == CALC_KEY_00) {
    calcAppendZeros();
    beep(800, 10);
    return;
  }

  // Digit key.
  if (calcJustEvaluated) {
    calcLen           = 0;
    calcExpr[0]       = '\0';
    calcJustEvaluated = false;
  }

  if (calcLen == 0) {
    calcExpr[0] = key;
    calcExpr[1] = '\0';
    calcLen     = 1;
  } else if (calcLen == 1 && calcExpr[0] == '0') {
    calcExpr[0] = key;                   // do not keep leading zeros
  } else if (calcLen < CALC_EXPR_MAX - 1) {
    calcExpr[calcLen++] = key;
    calcExpr[calcLen]   = '\0';
  }
  beep(800, 10);
}

static void calcDraw(uint8_t cursor) {
  updateRTC();
  staticHeader("CALCULATOR");

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  const char *top = calcError ? "ERROR" : (calcExpr[0] ? calcExpr : "0");
  const int topLen   = (int)strlen(top);
  const int topStart = (topLen > 20) ? (topLen - 20) : 0;
  display.setCursor(2, 12);
  display.print(top + topStart);

  char preview[CALC_NUM_MAX + 4];
  if (!calcError && calcPreview(preview, sizeof(preview))) {
    char line[CALC_NUM_MAX + 6];
    snprintf(line, sizeof(line), "= %s", preview);
    const int lineLen = (int)strlen(line);
    display.setCursor(126 - lineLen * 6, 21);
    display.print(line);
  }

  for (uint8_t i = 0; i < CALC_KEY_COUNT; ++i) {
    const CalcKey &k = CALC_KEYS[i];

    if (i == cursor) {
      display.fillRect(k.x, k.y, k.w, 8, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.drawRect(k.x, k.y, k.w, 8, SSD1306_WHITE);
      display.setTextColor(SSD1306_WHITE);
    }

    char label[4];
    if (k.key == '\b') {
      strcpy(label, "DEL");
    } else if (k.key == CALC_KEY_00) {
      strcpy(label, "00");
    } else {
      label[0] = k.key;
      label[1] = '\0';
    }

    const uint8_t labelLen = (uint8_t)strlen(label);
    display.setCursor(k.x + (k.w - labelLen * 6) / 2, k.y);
    display.print(label);
  }

  display.setTextColor(SSD1306_WHITE);
  display.display();
}

void MainCalculator() {
  calcReset();

  uint8_t cursor = 0;
  bool upHeld    = false;
  bool downHeld  = false;
  bool selectHeld = false;
  unsigned long lastUp   = 0;
  unsigned long lastDown = 0;
  bool dirty = true;
  int lastHeaderTime = -1;

  // A button held on the previous screen must not press a key here.
  while (digitalRead(SELECT_BUTTON) == LOW ||
         digitalRead(BACK_BUTTON) == LOW) {
    delay(10);
  }

  for (;;) {
    updateRTC();

    const int headerTime = hours * 60 + minutes;
    if (headerTime != lastHeaderTime) {
      lastHeaderTime = headerTime;
      dirty = true;
    }

    const bool up     = digitalRead(UP_BUTTON) == LOW;
    const bool down   = digitalRead(DOWN_BUTTON) == LOW;
    const bool select = digitalRead(SELECT_BUTTON) == LOW;
    const bool back   = digitalRead(BACK_BUTTON) == LOW;
    const unsigned long now = millis();

    if (up) {
      if (!upHeld) {
        cursor  = (cursor == 0) ? CALC_KEY_COUNT - 1 : cursor - 1;
        upHeld  = true;
        lastUp  = now;
        dirty   = true;
      } else if (now - lastUp >= 280) {
        cursor = (cursor == 0) ? CALC_KEY_COUNT - 1 : cursor - 1;
        lastUp = now;
        dirty  = true;
      }
    } else {
      upHeld = false;
    }

    if (down) {
      if (!downHeld) {
        cursor   = (cursor == CALC_KEY_COUNT - 1) ? 0 : cursor + 1;
        downHeld = true;
        lastDown = now;
        dirty    = true;
      } else if (now - lastDown >= 280) {
        cursor   = (cursor == CALC_KEY_COUNT - 1) ? 0 : cursor + 1;
        lastDown = now;
        dirty    = true;
      }
    } else {
      downHeld = false;
    }

    if (select) {
      if (!selectHeld) {
        selectHeld = true;
        calcPress(CALC_KEYS[cursor].key);
        dirty = true;
      }
    } else {
      selectHeld = false;
    }

    if (back) {
      while (digitalRead(BACK_BUTTON) == LOW) delay(10);
      return;
    }

    if (dirty) {
      calcDraw(cursor);
      dirty = false;
    }

    delay(5);
    yield();
  }
}
