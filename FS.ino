/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/FS.ino
  Filesystem Manager
	Lighteral (2026)
*/
uint8_t printFixed(const char *text, uint8_t maxLen) {
  uint8_t i = 0;
  for (; i < maxLen && text[i]; ++i) display.print(text[i]);
  return i;
}

void fs() {
  static const char *const names[MENU_ITEMS] = {
      mainDirectory.nameA,
      mainDirectory.nameB,
      mainDirectory.nameC,
      mainDirectory.nameD,
      mainDirectory.nameE,
      mainDirectory.nameF
  };

  // Build display labels once. The file names live in fixed 16-byte fields,
  // so bound the copy instead of assuming null termination.
  char labels[MENU_ITEMS][21];
  const char *items[MENU_ITEMS];
  for (uint8_t i = 0; i < MENU_ITEMS; ++i) {
    uint8_t n = 0;
    // The name field is 16 bytes, so at most 15 characters plus a terminator.
    while (n < 15 && names[i][n]) {
      labels[i][n] = names[i][n];
      ++n;
    }
    labels[i][n++] = '.';
    labels[i][n++] = 't';
    labels[i][n++] = 'x';
    labels[i][n++] = 't';
    labels[i][n] = '\0';
    items[i] = labels[i];
  }

  int8_t selection = 0;
  for (;;) {
    const int8_t choice = runMenu(items, MENU_ITEMS, "FILE MANAGER", selection);
    if (choice < 0) return; // BACK leaves the file manager.

    selection = choice;
    switch (choice) {
      case 0: showFile(mainDirectory.fileA, mainDirectory.nameA); break;
      case 1: showFile(mainDirectory.fileB, mainDirectory.nameB); break;
      case 2: showFile(mainDirectory.fileC, mainDirectory.nameC); break;
      case 3: showFile(mainDirectory.fileD, mainDirectory.nameD); break;
      case 4: showFile(mainDirectory.fileE, mainDirectory.nameE); break;
      case 5: showFile(mainDirectory.fileF, mainDirectory.nameF); break;
      default: continue;
    }

    display.display();
    while (digitalRead(BACK_BUTTON) == HIGH) delay(20);
    // runMenu ignores the still-held BACK button when it starts again.
  }
}

void showFile(const char *file, const char *name) {
  staticHeader("FILE MANAGER");
  display.setCursor(0, HEADER_HEIGHT + 2);
  display.print(' ');
  printFixed(name, 16);
  display.println(".txt");
  printFixed(file, 128);
}
