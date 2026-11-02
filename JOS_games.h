/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/JOS_games.h
  Shared game types live in a header so the Arduino auto-prototype generator
  sees them before it emits prototypes for the game functions.
	Lighteral (2026)
*/
#ifndef GAME_H

#include <stdint.h>

struct FlappyPipe {
  int16_t x;
  uint8_t gapY;
  bool scored;
};

struct PfPlatform {
  int32_t x;
  uint8_t y;
  uint8_t w;
};
#define GAME_H
#endif
