#include <Arduboy2.h>
#include <Sprites.h>

Arduboy2 ab;
Sprites sp;

const PROGMEM uint8_t ledGraph[] = {
  6, 8,
  0x1e, 0x21, 0x21, 0x21, 0x21, 0x1e,
  0x1e, 0x3f, 0x3f, 0x3d, 0x33, 0x1e
};

const PROGMEM uint8_t outGraph[] = {
  11, 8,
  0x3e, 0x41, 0x3e, 0x00,
  0x3f, 0x40, 0x3f, 0x00,
  0x01, 0x7f, 0x01
};

const PROGMEM uint8_t romFont[] = {
  3, 8,
  0x0e, 0x11, 0x0e, // 0
  0x12, 0x1f, 0x10, // 1
  0x1d, 0x15, 0x17, // 2
  0x15, 0x15, 0x1f, // 3
  0x07, 0x04, 0x1f, // 4
  0x17, 0x15, 0x1d, // 5
  0x1f, 0x15, 0x1d, // 6
  0x01, 0x01, 0x1f, // 7
  0x1f, 0x15, 0x1f, // 8
  0x17, 0x15, 0x1f, // 9
  0x1f, 0x05, 0x1f, // A
  0x1f, 0x15, 0x0a, // B
  0x1f, 0x11, 0x0a, // C
  0x1f, 0x11, 0x0e, // D
  0x1f, 0x15, 0x15, // E
  0x1f, 0x05, 0x05, // F
  0x00, 0x0a, 0x00  // :
};
const PROGMEM uint8_t inverter[] = { 0x7f, 0x7f, 0x7f, 0x7f, 0x7f};

uint8_t running = 0;
uint8_t prescaler = 10, prescaleCount = 0;
uint8_t editing = 0;
uint8_t editx = 0, edity = 0;
const uint8_t EDIT_CURSOR_ANIM_PERIOD = 70;
uint8_t editCursorAnim = 0;

const uint8_t BUTTON_PRESS_NONE = 0;
const uint8_t BUTTON_PRESS_A = 1;
const uint8_t BUTTON_PRESS_AB = 2;
const uint8_t BUTTON_PRESS_B = 4;
const uint8_t BUTTON_PRESS_BA = 8;
uint8_t buttonStatus = BUTTON_PRESS_NONE;

uint8_t a = 0, b = 0, c = 0, pc = 0, out = 0, in = 0;
uint8_t rom[16];

const uint8_t ROM_REDRAW_NONE = 0xff;
uint8_t statusFullRedraw = 0;
uint8_t romIdxRedraw = ROM_REDRAW_NONE, romBitRedraw = ROM_REDRAW_NONE;
// drawn values
uint8_t ad = 0, bd = 0, cd = 0, pcd = 0, outd = 0, ind = 0;
uint8_t runningd = 0, editingd = 0, buttonStatusd = 0;
uint8_t editxd = 0, edityd = 0, editscd = 0;

void emulateOneCycle() {
  pc = (pc + 1) & 0xf;
}

void updateMainUI(uint8_t releasedButton) {
  if ((editing || running) && releasedButton == BUTTON_PRESS_AB) {
    releasedButton = BUTTON_PRESS_A;
  }
  switch (releasedButton) {
    case BUTTON_PRESS_A:
      if (editing) {
        romIdxRedraw = edity + 8 * (editx >= 8);
        romBitRedraw = editx % 8;
        rom[romIdxRedraw] ^= 1 << (7 - romBitRedraw);
      } else {
        if (running) {
          running = 0;
        } else {
          emulateOneCycle();
        }
      }
      break;
    case BUTTON_PRESS_AB:
      running = 1;
      prescaleCount = prescaler;
      break;
    case BUTTON_PRESS_B:
      if (editing) {
        editing = 0;
      } else {
        editing = 1;
        editCursorAnim = 0;
      }
      break;
    case BUTTON_PRESS_BA:
      running = 0;
      break;
  }
  if (editing) {
    editCursorAnim++;
    if (editCursorAnim >= EDIT_CURSOR_ANIM_PERIOD) editCursorAnim = 0;
    
    if (ab.justReleased(LEFT_BUTTON)) {
      if (editx == 0) editx = 15; else editx--;
    }
    if (ab.justReleased(UP_BUTTON)) {
      if (edity == 0) edity = 7; else edity--;
    }
    if (ab.justReleased(DOWN_BUTTON)) {
      edity++;
      if (edity > 7) edity = 0;
    }
    if (ab.justReleased(RIGHT_BUTTON)) {
      editx++;
      if (editx > 15) editx = 0;
    }
  } else {
    if (ab.justReleased(LEFT_BUTTON)) in ^= 8;
    if (ab.justReleased(UP_BUTTON)) in ^= 4;
    if (ab.justReleased(DOWN_BUTTON)) in ^= 2;
    if (ab.justReleased(RIGHT_BUTTON)) in ^= 1;
  }

  if (running) {
    prescaleCount--;
    if (prescaleCount == 0) {
      prescaleCount = prescaler;
      emulateOneCycle();
    }
  }
}

void drawMainUI() {
  const int LED_POS = 13;
  const int PC_Y = 0, A_Y = 10 + 8 * 0, B_Y = 10 + 8 * 1;
  const int C_Y = 10 + 8 * 2, OUT_Y = 10 + 8 * 3, IN_Y = 10 + 8 * 4 + 2;

  const int ROM_C1 = 45, ROM_C2 = 87;
  
  if (statusFullRedraw) {
    ab.clear();
  
    // draw status
    for (int i = 0; i < 4; i++) {
      sp.drawSelfMasked(LED_POS + 7 * i, PC_Y + 1, ledGraph, (pc >> (3 - i)) & 1);
      sp.drawSelfMasked(LED_POS + 7 * i, A_Y + 1, ledGraph, (a >> (3 - i)) & 1);
      sp.drawSelfMasked(LED_POS + 7 * i, B_Y + 1, ledGraph, (b >> (3 - i)) & 1);
      sp.drawSelfMasked(LED_POS + 7 * i, OUT_Y + 1, ledGraph, (b >> (3 - i)) & 1);
      sp.drawSelfMasked(LED_POS + 7 * i, IN_Y + 1, ledGraph, (b >> (3 - i)) & 1);
    }
    sp.drawSelfMasked(LED_POS, C_Y + 1, ledGraph, c & 1);
    ab.setCursor(0, PC_Y); ab.print(F("PC"));
    ab.setCursor(3, A_Y); ab.write('A');
    ab.setCursor(3, B_Y); ab.write('B');
    ab.setCursor(3, C_Y); ab.write('C');
    sp.drawSelfMasked(0, OUT_Y, outGraph, 0);
    ab.setCursor(0, IN_Y); ab.print(F("IN"));
  
    // draw ROM contents
    for (int i = 0; i < 8; i++) {
      int y = 1 + 7 * i;
      sp.drawSelfMasked(ROM_C1 + 1, y, romFont, i);
      sp.drawSelfMasked(ROM_C1 + 5, y, romFont, 16);
      for (int j = 0; j < 8; j++) {
        sp.drawSelfMasked(ROM_C1 + 8 + 4 * j + 2 * (j >= 4), y, romFont, (rom[i] >> (7 - j)) & 1);
      }
      sp.drawSelfMasked(ROM_C2 + 1, y, romFont, 8 + i);
      sp.drawSelfMasked(ROM_C2 + 5, y, romFont, 16);
      for (int j = 0; j < 8; j++) {
        sp.drawSelfMasked(ROM_C2 + 8 + 4 * j + 2 * (j >= 4), y, romFont, (rom[8 + i] >> (7 - j)) & 1);
      }
    }
    // draw PC marker on ROM
    ab.drawBitmap(pc & 8 ? ROM_C2 : ROM_C1, 7 * (pc % 8), inverter, 5, 7, INVERT);

    // save what is drawn
    statusFullRedraw = 0;
    ad = a; bd = b; cd = c; pcd = pc; outd = out; ind = in;
    // request button status redraw
    buttonStatusd = ~buttonStatus;
  } else {
    // redraw what is updated
    if (pc != pcd) {
      // redraw PC LED
      ab.fillRect(LED_POS, PC_Y + 1, 6 * 4 + 3, 6, BLACK);
      for (int i = 0; i < 4; i++) {
        sp.drawSelfMasked(LED_POS + 7 * i, PC_Y + 1, ledGraph, (pc >> (3 - i)) & 1);
      }
      // redraw PC on ROM
      ab.fillRect(pcd & 8 ? ROM_C2 : ROM_C1, 7 * (pcd % 8), 5, 7, BLACK);
      sp.drawSelfMasked((pcd & 8 ? ROM_C2 : ROM_C1) + 1, 7 * (pcd % 8) + 1, romFont, pcd & 0xf);
      ab.drawBitmap(pc & 8 ? ROM_C2 : ROM_C1, 7 * (pc % 8), inverter, 5, 7, INVERT);
      // save new PC
      pcd = pc;
    }
    if (a != ad) {
      ab.fillRect(LED_POS, A_Y + 1, 6 * 4 + 3, 6, BLACK);
      for (int i = 0; i < 4; i++) {
        sp.drawSelfMasked(LED_POS + 7 * i, A_Y + 1, ledGraph, (a >> (3 - i)) & 1);
      }
      ad = a;
    }
    if (b != bd) {
      ab.fillRect(LED_POS, B_Y + 1, 6 * 4 + 3, 6, BLACK);
      for (int i = 0; i < 4; i++) {
        sp.drawSelfMasked(LED_POS + 7 * i, B_Y + 1, ledGraph, (b >> (3 - i)) & 1);
      }
      bd = b;
    }
    if (c != cd) {
      ab.fillRect(LED_POS, C_Y + 1, 6, 6, BLACK);
      sp.drawSelfMasked(LED_POS, C_Y + 1, ledGraph, c & 1);
      cd = c;
    }
    if (out != outd) {
      ab.fillRect(LED_POS, OUT_Y + 1, 6 * 4 + 3, 6, BLACK);
      for (int i = 0; i < 4; i++) {
        sp.drawSelfMasked(LED_POS + 7 * i, OUT_Y + 1, ledGraph, (out >> (3 - i)) & 1);
      }
      outd = out;
    }
    if (in != ind) {
      ab.fillRect(LED_POS, IN_Y + 1, 6 * 4 + 3, 6, BLACK);
      for (int i = 0; i < 4; i++) {
        sp.drawSelfMasked(LED_POS + 7 * i, IN_Y + 1, ledGraph, (in >> (3 - i)) & 1);
      }
      ind = in;
    }
    if (romIdxRedraw != ROM_REDRAW_NONE) {
      int x = (romIdxRedraw & 8 ? ROM_C2 : ROM_C1) + 8 + 4 * romBitRedraw + 2 * (romBitRedraw >= 4);
      int y = 1 + 7 * (romIdxRedraw % 8);
      ab.fillRect(x, y, 3, 5, BLACK);
      sp.drawSelfMasked(x, y, romFont, (rom[romIdxRedraw] >> (7 - romBitRedraw)) & 1);
      romIdxRedraw = ROM_REDRAW_NONE;
    }
  }
  bool editingChanged = editing != editingd;
  if (buttonStatus != buttonStatusd || running != runningd || editingChanged) {
    ab.fillRect(0, 57, 128, 7, BLACK);
    if (buttonStatus & (BUTTON_PRESS_B | BUTTON_PRESS_BA)) {
      ab.setCursor(0, 57);
      ab.print(F("B+A:MENU"));
    } else {
      ab.setCursor(12, 57);
      if (editing) {
        ab.print(F("A:TOGGLE"));
      } else {
        if (running) {
          ab.print(F("A:STOP"));
        } else {
          ab.print(F("A:STEP"));
        }
      }
    }
    if (buttonStatus & (BUTTON_PRESS_A | BUTTON_PRESS_AB)) {
      ab.setCursor(64, 57);
      if (!editing && !running) {
        ab.print(F("A+B:RUN"));
      }
    } else {
      ab.setCursor(64 + 12, 57);
      if (editing) {
        ab.print(F("B:BACK"));
      } else {
        ab.print(F("B:EDIT"));
      }
    }
    runningd = running;
    editingd = editing;
    buttonStatusd = buttonStatus;
  }
  uint8_t editsc = editCursorAnim < EDIT_CURSOR_ANIM_PERIOD / 2;
  if (editingChanged || editx != editxd || edity != edityd || editsc != editscd) {
    int xd = (editxd & 8 ? ROM_C2 : ROM_C1) + 8 + 4 * (editxd % 8) + 2 * (editxd % 8 >= 4);
    int yd = 1 + 7 * edityd + 5;
    int x = (editx & 8 ? ROM_C2 : ROM_C1) + 8 + 4 * (editx % 8) + 2 * (editx % 8 >= 4);
    int y = 1 + 7 * edity + 5;
    ab.drawFastHLine(xd, yd, 3, BLACK);
    if (editing && editsc) {
      ab.drawFastHLine(x, y, 3, WHITE);
    }
    editingd = editing;
    editxd = editx;
    edityd = edity;
    editscd = editsc;
  }
}

void setup() {
  ab.begin();
  ab.setTextSize(1);
  ab.setFrameRate(100);
  statusFullRedraw = 1;
}

void loop() {
  if (!ab.nextFrame()) {
    ab.idle();
    return;
  }
  ab.pollButtons();
  uint8_t releasedButtonStatus = BUTTON_PRESS_NONE;
  switch (buttonStatus) {
    case BUTTON_PRESS_NONE:
      if (ab.pressed(A_BUTTON)) {
        buttonStatus = BUTTON_PRESS_A;
      } else if (ab.pressed(B_BUTTON)) {
        buttonStatus = BUTTON_PRESS_B;
      }
      break;
    case BUTTON_PRESS_A:
      if (ab.pressed(B_BUTTON)) {
        buttonStatus = BUTTON_PRESS_AB;
      } else if (ab.notPressed(A_BUTTON)) {
        releasedButtonStatus = BUTTON_PRESS_A;
        buttonStatus = BUTTON_PRESS_NONE;
      }
      break;
    case BUTTON_PRESS_AB:
      if (ab.notPressed(A_BUTTON | B_BUTTON)) {
        releasedButtonStatus = BUTTON_PRESS_AB;
        buttonStatus = BUTTON_PRESS_NONE;
      }
      break;
    case BUTTON_PRESS_B:
      if (ab.pressed(A_BUTTON)) {
        buttonStatus = BUTTON_PRESS_BA;
      } else if (ab.notPressed(B_BUTTON)) {
        releasedButtonStatus = BUTTON_PRESS_B;
        buttonStatus = BUTTON_PRESS_NONE;
      }
      break;
    case BUTTON_PRESS_BA:
      if (ab.notPressed(A_BUTTON | B_BUTTON)) {
        releasedButtonStatus = BUTTON_PRESS_BA;
        buttonStatus = BUTTON_PRESS_NONE;
      }
      break;
  }

  updateMainUI(releasedButtonStatus);

  drawMainUI();
  ab.display();
}
