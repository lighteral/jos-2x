/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/JOS_Rewritten_2.0.0-a.ino
	Lighteral (2026)
*/
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <math.h>
#include "JOS_bitmaps.h"
#include "JOS_config.h"
#include "JOS_games.h"

// Hardware
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, JOS_OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);

// Shared application state
short hours   = 0;
short minutes = 0;
short seconds = 0;
short date    = 0;
short month   = 0;
int   year    = 0;

struct EStructure {
  char fileA[128];
  char fileB[128];
  char fileC[128];
  char fileD[128];
  char fileE[128];
  char fileF[128];
  char nameA[16];
  char nameB[16];
  char nameC[16];
  char nameD[16];
  char nameE[16];
  char nameF[16];
  char chatMsgA1[32];
  char chatMsgA2[32];
  char chatMsgB1[32];
  char chatMsgB2[32];
  char chatMsgC1[32];
  char chatMsgC2[32];
  char chatMsgD1[32];
  char chatMsgD2[32];
  char chatMsgE1[32];
  char chatMsgE2[32];
  char chatMsgF1[32];
  char chatMsgF2[32];
  char chatNameA[16];
  char chatNameB[16];
  char chatNameC[16];
  char chatNameD[16];
  char chatNameE[16];
  char chatNameF[16];
};
EStructure mainDirectory;

// Forward declarations (the Arduino preprocessor also generates these)
void splash();
void bootTune();
void staticHeader(const char *title);
void updateRTC();

void MainMenu();
void MainHome();
void UtilitiesMenu();
void GamesMenu();
void aboutScreen();
int8_t runMenu(const char *const *items, uint8_t count, const char *title, int8_t start);

void MainClock();
void cluck();

void MainTemp();
void dl();
void drawMetricBox(uint8_t x, const char *label, float value, const char *unit);

void MainMusic();
void musicdisc();
bool sing(int s);
bool buzz(uint8_t targetPin, int16_t frequency, uint16_t length);

void MainGame();
void gameloop();
void playerPaddleTone();
void mcuPaddleTone();
void wallTone();
void player_scoreTone();
void mcu_scoreTone();
void drawCourt();
void beep(uint16_t frequency, uint16_t duration);

void MainFlappy();
void MainPlatformer();
void MainTicTacToe();
void MainCalculator();

void fs();
void showFile(const char *file, const char *name);

// Helpers
void printPad2(uint8_t value) {
  if (value < 10) display.print('0');
  display.print((int)value);
}

uint8_t bcdToDec(uint8_t value) {
  return (uint8_t)(((value >> 4) * 10) + (value & 0x0F));
}

void updateRTC() {
  static unsigned long lastUpdate = 0;
  const unsigned long now = millis();
  const unsigned long elapsed = now - lastUpdate;
  if (elapsed < 1000) return;

  uint32_t ticks = elapsed / 1000;
  lastUpdate += ticks * 1000UL;

  if (USE_RTC) {
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write((uint8_t)0x00);
    if (Wire.endTransmission() == 0 &&
        Wire.requestFrom((uint8_t)RTC_ADDRESS, (uint8_t)7) == 7) {
      seconds = bcdToDec((uint8_t)Wire.read() & 0x7F);
      minutes = bcdToDec((uint8_t)Wire.read() & 0x7F);

      const uint8_t rawHour = (uint8_t)Wire.read();
      if (rawHour & 0x40) {                       // 12-hour mode
        const uint8_t hour12 = bcdToDec(rawHour & 0x1F);
        hours = (rawHour & 0x20) ? (hour12 % 12) + 12 : (hour12 % 12);
      } else {                                    // 24-hour mode
        hours = bcdToDec(rawHour & 0x3F);
      }

      Wire.read();                                // day of week (unused)
      date  = bcdToDec((uint8_t)Wire.read() & 0x3F);
      month = bcdToDec((uint8_t)Wire.read() & 0x1F);
      year  = bcdToDec((uint8_t)Wire.read());     // 0-99, rendered as 20xx
      return;
    }
    // Fall through to the software clock if the RTC does not respond.
  }

  // Software clock fallback so the clock still advances without an RTC.
  ticks %= 86400UL;
  uint32_t total = (uint32_t)hours * 3600UL +
                   (uint32_t)minutes * 60UL +
                   (uint32_t)seconds + ticks;
  total %= 86400UL;
  hours   = (short)(total / 3600UL);
  total  %= 3600UL;
  minutes = (short)(total / 60UL);
  seconds = (short)(total % 60UL);
}

// Setup / main loop
void setup() {
  Wire.setSDA(4); // Physical Pin 6
  Wire.setSCL(5); // Physical Pin 7
  Wire.begin();
  Wire.setClock(400000); // Faster updates keep the games responsive.
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for (;;) delay(100);
  }
  display.clearDisplay();

  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  pinMode(SELECT_BUTTON, INPUT_PULLUP);
  pinMode(BACK_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  if (SPLASHSCREEN) splash();
}

void loop() {
  MainHome();
}

void splash() {
  display.clearDisplay();
  display.drawBitmap(0, 0, TitlePage, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();

  const unsigned long start = millis();
  if (BOOT_TUNE_ENABLED) bootTune();

  // Keep the splash visible for its full length even when the tune is short.
  const unsigned long elapsed = millis() - start;
  if (elapsed < SPLASH_LENGTH_MS) delay(SPLASH_LENGTH_MS - elapsed);

  display.clearDisplay();
  display.display();
}

void staticHeader(const char *title) {
  display.clearDisplay();
  display.fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, 1);
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(1);
  display.setFont(NULL);
  // Center the 8 px tall text inside the taller bar.
  const int textY = (HEADER_HEIGHT - 8) / 2;
  display.setCursor(1, textY);
  if (title != nullptr) display.print(title);
  display.setCursor(98, textY);
  printPad2((uint8_t)hours);
  display.print(':');
  printPad2((uint8_t)minutes);
  display.setTextColor(WHITE);
}
