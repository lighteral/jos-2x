/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/Weather.ino
  The DHT11/22 Weather application
	Lighteral (2026)
*/
void MainTemp() {
  while (true) {
    dl();
    if (digitalRead(BACK_BUTTON) == LOW) break;
  }
}

void dl() {
  const float temperature = dht.readTemperature();
  const float humidity    = dht.readHumidity();

  updateRTC();
  staticHeader("WEATHER");

  drawMetricBox(1, "TEMP", temperature, "C");
  drawMetricBox(65, "HUMIDITY", humidity, "%");
  display.display();

  // DHT11 needs about a second between samples. Wait in small slices so the
  // BACK button stays responsive.
  for (unsigned long waited = 0; waited < DHT_READ_INTERVAL; waited += 50) {
    if (digitalRead(BACK_BUTTON) == LOW) return;
    delay(50);
  }
}

void drawMetricBox(uint8_t x, const char *label, float value, const char *unit) {
  static constexpr uint8_t BOX_Y      = HEADER_HEIGHT + 2;
  static constexpr uint8_t BOX_WIDTH  = 62;
  static constexpr uint8_t BOX_HEIGHT = 50;

  display.drawRoundRect(x, BOX_Y, BOX_WIDTH, BOX_HEIGHT, 4, WHITE);

  uint8_t labelLength = 0;
  while (label[labelLength]) ++labelLength;

  // Inverted label strip makes each reading look like a card, not console text.
  display.fillRect(x + 2, BOX_Y + 2, BOX_WIDTH - 4, 9, WHITE);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(x + (BOX_WIDTH - labelLength * 6) / 2, BOX_Y + 3);
  display.print(label);
  display.setTextColor(WHITE);

  const bool missing = isnan(value);
  uint8_t valueChars = 2; // "--"
  if (!missing) {
    float magnitude = value;
    const bool negative = magnitude < 0;
    if (negative) magnitude = -magnitude;

    // Add half of the displayed precision so 99.96 counts as 100.0.
    magnitude += 0.05f;
    valueChars = (magnitude >= 100.0f) ? 3 : (magnitude >= 10.0f ? 2 : 1);
    valueChars += 2; // decimal point plus one decimal digit
    if (negative) ++valueChars;
  }

  display.setTextSize(2);
  display.setCursor(x + (BOX_WIDTH - valueChars * 12) / 2, BOX_Y + 17);
  if (missing) {
    display.print("--");
  } else {
    display.print(value, 1);
  }

  uint8_t unitLength = 0;
  while (unit[unitLength]) ++unitLength;

  display.setTextSize(1);
  display.setCursor(x + (BOX_WIDTH - unitLength * 6) / 2, BOX_Y + 36);
  display.print(unit);
}
