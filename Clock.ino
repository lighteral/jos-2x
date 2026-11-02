/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/Clock.ino
  The Clock application
	Lighteral (2026)
*/
void MainClock() {
  short lastSecond = -1;

  while (true) {
    updateRTC();
    if (seconds != lastSecond) {
      cluck(); // Chickens can tell the time, so the clock application is called cluck.
      lastSecond = seconds;
    }
    if (digitalRead(BACK_BUTTON) == LOW) break;
    delay(50);
  }
}

void cluck() {
  staticHeader("CLOCK");

  display.setTextSize(2);
  display.setCursor(16, 20);
  printPad2((uint8_t)hours);
  display.print(':');
  printPad2((uint8_t)minutes);
  display.print(':');
  printPad2((uint8_t)seconds);

  display.setCursor(34, 40);
  display.setTextSize(1);
  display.print(date);
  display.print('/');
  display.print(month);
  display.print("/20");
  display.print(year);
  display.display();
}
