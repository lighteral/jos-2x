/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/Audio.ino
  ALL THE AUDIO AND MUSIC CODE HERE
	Lighteral (2026)
*/
/* SECTION I: MUSIC PLAYER */
void MainMusic() {
  display.clearDisplay();
  display.drawBitmap(0, 0, MediaPlayer, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
  musicdisc();
}

void musicdisc() {
  if (!sing(1)) return;
  if (!sing(1)) return;
  sing(2);
}

bool sing(int s) {
  const int16_t *notes;
  const int16_t *tempos;
  size_t count;

  if (s == 2) {
    notes  = underworld_melody;
    tempos = underworld_tempo;
    count  = sizeof(underworld_melody) / sizeof(underworld_melody[0]);
  } else {
    notes  = melody;
    tempos = tempo;
    count  = sizeof(melody) / sizeof(melody[0]);
  }

  for (size_t i = 0; i < count; ++i) {
    if (digitalRead(BACK_BUTTON) == LOW) return false;

    const uint16_t noteDuration = 1000 / tempos[i];
    if (!buzz(BUZZER, notes[i], noteDuration)) return false;

    // 1.3 * noteDuration, without floating point.
    delay(noteDuration + (noteDuration * 3) / 10);

    if (!buzz(BUZZER, 0, noteDuration)) return false;
  }
  return true;
}

bool buzz(uint8_t targetPin, int16_t frequency, uint16_t length) {
  if (digitalRead(BACK_BUTTON) == LOW) return false;

  // A zero-frequency note is a rest. The old code divided by zero here.
  if (frequency <= 0) {
    delay(length);
    return digitalRead(BACK_BUTTON) != LOW;
  }

  const uint32_t halfPeriod = 500000UL / (uint16_t)frequency;
  const uint32_t cycles = ((uint32_t)frequency * length) / 1000UL;

  for (uint32_t i = 0; i < cycles; ++i) {
    digitalWrite(targetPin, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(targetPin, LOW);
    delayMicroseconds(halfPeriod);
    if (digitalRead(BACK_BUTTON) == LOW) return false;
  }
  return true;
}
/* SECTION II: STARTUP SOUND*/
// Short rising jingle played while the splash screen is visible.

void bootTune() {
  const uint8_t count = sizeof(BOOT_TUNE_NOTES) / sizeof(BOOT_TUNE_NOTES[0]);

  for (uint8_t i = 0; i < count; ++i) {
    beep(BOOT_TUNE_NOTES[i], BOOT_TUNE_LENGTHS[i]);
    delay(BOOT_TUNE_GAP_MS);
  }
}
