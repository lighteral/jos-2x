/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/XO.ino
  NOUGHTS AND CROSSES
	Lighteral (2026)
*/
static constexpr uint8_t TTT_NONE  = 0;
static constexpr uint8_t TTT_X     = 1;
static constexpr uint8_t TTT_O     = 2;
static constexpr uint8_t TTT_DRAW  = 3;
static constexpr uint8_t TTT_CELLS = 9;

static constexpr uint8_t TTT_BOARD_X = 22;
static constexpr uint8_t TTT_BOARD_Y = 14;
static constexpr uint8_t TTT_CELL_W  = 28;
static constexpr uint8_t TTT_CELL_H  = 16;

// 0 = no winner yet, 1 = X, 2 = O, 3 = draw.
static uint8_t ticTacToeWinner(const uint8_t *board) {
  static const uint8_t lines[8][3] = {
      {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
      {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
      {0, 4, 8}, {2, 4, 6}
  };

  for (uint8_t i = 0; i < 8; ++i) {
    const uint8_t a = board[lines[i][0]];
    if (a != TTT_NONE && a == board[lines[i][1]] &&
        a == board[lines[i][2]]) {
      return a;
    }
  }

  for (uint8_t i = 0; i < TTT_CELLS; ++i) {
    if (board[i] == TTT_NONE) return TTT_NONE;
  }
  return TTT_DRAW;
}

static int ticTacToeScore(uint8_t *board, uint8_t depth,
                          int alpha, int beta, bool maximizing) {
  const uint8_t winner = ticTacToeWinner(board);
  if (winner == TTT_O) return 10 - depth;
  if (winner == TTT_X) return depth - 10;
  if (winner == TTT_DRAW) return 0;

  if (maximizing) {
    int best = -100;
    for (uint8_t i = 0; i < TTT_CELLS; ++i) {
      if (board[i] != TTT_NONE) continue;
      board[i] = TTT_O;
      const int score = ticTacToeScore(board, depth + 1, alpha, beta, false);
      board[i] = TTT_NONE;
      if (score > best) best = score;
      if (best > alpha) alpha = best;
      if (beta <= alpha) break;
    }
    return best;
  }

  int best = 100;
  for (uint8_t i = 0; i < TTT_CELLS; ++i) {
    if (board[i] != TTT_NONE) continue;
    board[i] = TTT_X;
    const int score = ticTacToeScore(board, depth + 1, alpha, beta, true);
    board[i] = TTT_NONE;
    if (score < best) best = score;
    if (best < beta) beta = best;
    if (beta <= alpha) break;
  }
  return best;
}

// AI plays O. Equal-scoring moves are picked at random so games vary.
static uint8_t ticTacToeAIMove(uint8_t *board) {
  int bestScore = -100;
  uint8_t bestMoves[TTT_CELLS];
  uint8_t bestCount = 0;

  for (uint8_t i = 0; i < TTT_CELLS; ++i) {
    if (board[i] != TTT_NONE) continue;

    board[i] = TTT_O;
    const int score = ticTacToeScore(board, 0, -100, 100, false);
    board[i] = TTT_NONE;

    if (score > bestScore) {
      bestScore = score;
      bestCount = 0;
      bestMoves[bestCount++] = i;
    } else if (score == bestScore) {
      bestMoves[bestCount++] = i;
    }
  }

  return bestCount ? bestMoves[random(bestCount)] : 0xFF;
}

static void ticTacToeDraw(const uint8_t *board, uint8_t cursor,
                          uint8_t turn) {
  updateRTC();
  staticHeader("TIC TAC TOE");

  display.setTextColor(BLACK, WHITE);
  display.setCursor(74, (HEADER_HEIGHT - 8) / 2);
  display.print(turn == TTT_X ? "X" : "O");
  display.setTextColor(WHITE);

  // Selected cell is inverted. Symbols are drawn afterwards so they stay
  // readable in both the normal and selected cells.
  for (uint8_t i = 0; i < TTT_CELLS; ++i) {
    if (i != cursor) continue;
    const uint8_t col = i % 3;
    const uint8_t row = i / 3;
    display.fillRect(TTT_BOARD_X + col * TTT_CELL_W + 1,
                     TTT_BOARD_Y + row * TTT_CELL_H + 1,
                     TTT_CELL_W - 2, TTT_CELL_H - 2, WHITE);
  }

  display.drawRect(TTT_BOARD_X - 1, TTT_BOARD_Y - 1,
                   TTT_CELL_W * 3 + 2, TTT_CELL_H * 3 + 2, WHITE);
  display.drawFastVLine(TTT_BOARD_X + TTT_CELL_W, TTT_BOARD_Y,
                        TTT_CELL_H * 3, WHITE);
  display.drawFastVLine(TTT_BOARD_X + TTT_CELL_W * 2, TTT_BOARD_Y,
                        TTT_CELL_H * 3, WHITE);
  display.drawFastHLine(TTT_BOARD_X, TTT_BOARD_Y + TTT_CELL_H,
                        TTT_CELL_W * 3, WHITE);
  display.drawFastHLine(TTT_BOARD_X, TTT_BOARD_Y + TTT_CELL_H * 2,
                        TTT_CELL_W * 3, WHITE);

  for (uint8_t i = 0; i < TTT_CELLS; ++i) {
    if (board[i] == TTT_NONE) continue;

    const uint8_t col = i % 3;
    const uint8_t row = i / 3;
    const int16_t cx = TTT_BOARD_X + col * TTT_CELL_W + TTT_CELL_W / 2;
    const int16_t cy = TTT_BOARD_Y + row * TTT_CELL_H + TTT_CELL_H / 2;
    const uint16_t color = (i == cursor) ? BLACK : WHITE;

    if (board[i] == TTT_X) {
      display.drawLine(cx - 4, cy - 4, cx + 4, cy + 4, color);
      display.drawLine(cx - 4, cy + 4, cx + 4, cy - 4, color);
    } else {
      display.drawCircle(cx, cy, 5, color);
    }
  }

  display.display();
}

static void ticTacToeHelp() {
  display.fillRect(6, 14, 116, 44, BLACK);
  display.drawRect(6, 14, 116, 44, WHITE);
  display.setTextColor(WHITE);
  display.setCursor(25, 18);
  display.print("UP/DOWN: MOVE");
  display.setCursor(25, 28);
  display.print("SELECT: PLACE");
  display.setCursor(34, 38);
  display.print("BACK: EXIT");
  display.setCursor(19, 48);
  display.print("SELECT TO START");
  display.display();
}

static void ticTacToeResult(uint8_t result) {
  display.fillRect(10, 18, 108, 38, BLACK);
  display.drawRect(10, 18, 108, 38, WHITE);
  display.setTextColor(WHITE);

  const char *text = (result == TTT_X) ? "X WINS!"
                     : (result == TTT_O) ? "O WINS!"
                                         : "DRAW!";
  uint8_t length = 0;
  while (text[length]) ++length;
  display.setCursor((SCREEN_WIDTH - length * 6) / 2, 22);
  display.print(text);

  display.setCursor(34, 34);
  display.print("SEL: AGAIN");
  display.setCursor(34, 44);
  display.print("BACK: EXIT");
  display.display();
}

static bool ticTacToeRound(bool ai, bool showHelp) {
  uint8_t board[TTT_CELLS] = {0};
  uint8_t cursor = 4;
  uint8_t turn = TTT_X;
  uint8_t result = TTT_NONE;
  bool gameOver = false;
  bool dirty = true;
  bool upHeld = false;
  bool downHeld = false;
  bool selectHeld = false;

  if (showHelp) {
    ticTacToeDraw(board, cursor, turn);
    ticTacToeHelp();
    for (;;) {
      if (digitalRead(BACK_BUTTON) == LOW) {
        while (digitalRead(BACK_BUTTON) == LOW) delay(10);
        return false;
      }
      if (digitalRead(SELECT_BUTTON) == LOW) {
        while (digitalRead(SELECT_BUTTON) == LOW) delay(10);
        break;
      }
      delay(10);
    }
  }

  for (;;) {
    if (dirty) {
      ticTacToeDraw(board, cursor, turn);
      if (gameOver) ticTacToeResult(result);
      dirty = false;
    }

    if (digitalRead(BACK_BUTTON) == LOW) {
      while (digitalRead(BACK_BUTTON) == LOW) delay(10);
      return false;
    }

    if (gameOver) {
      if (digitalRead(SELECT_BUTTON) == LOW) {
        while (digitalRead(SELECT_BUTTON) == LOW) delay(10);
        return true;
      }
      delay(10);
      continue;
    }

    if (ai && turn == TTT_O) {
      for (uint8_t i = 0; i < 30; ++i) {
        if (digitalRead(BACK_BUTTON) == LOW) {
          while (digitalRead(BACK_BUTTON) == LOW) delay(10);
          return false;
        }
        delay(10);
      }

      const uint8_t move = ticTacToeAIMove(board);
      if (move < TTT_CELLS) {
        board[move] = TTT_O;
        beep(500, 15);
        result = ticTacToeWinner(board);
        if (result != TTT_NONE) {
          gameOver = true;
          beep(900, 80);
        } else {
          turn = TTT_X;
        }
        dirty = true;
      }
      continue;
    }

    const bool up = digitalRead(UP_BUTTON) == LOW;
    if (up && !upHeld) {
      cursor = (cursor == 0) ? TTT_CELLS - 1 : cursor - 1;
      dirty = true;
    }
    upHeld = up;

    const bool down = digitalRead(DOWN_BUTTON) == LOW;
    if (down && !downHeld) {
      cursor = (cursor == TTT_CELLS - 1) ? 0 : cursor + 1;
      dirty = true;
    }
    downHeld = down;

    const bool select = digitalRead(SELECT_BUTTON) == LOW;
    if (select && !selectHeld) {
      if (board[cursor] == TTT_NONE) {
        board[cursor] = turn;
        beep(turn == TTT_X ? 800 : 500, 15);
        result = ticTacToeWinner(board);
        if (result != TTT_NONE) {
          gameOver = true;
          beep(900, 80);
        } else {
          turn = (turn == TTT_X) ? TTT_O : TTT_X;
        }
        dirty = true;
      } else {
        beep(150, 30);
      }
    }
    selectHeld = select;

    delay(5);
  }
}

void MainTicTacToe() {
  static const char *const modes[] = {
      "1 Player vs AI",
      "2 Players"
  };

  const int8_t mode = runMenu(modes, 2, "TIC TAC TOE", 0);
  if (mode < 0) return;

  bool firstRound = true;
  for (;;) {
    if (!ticTacToeRound(mode == 0, firstRound)) return;
    firstRound = false;
  }
}
