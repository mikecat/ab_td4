#include <Arduboy2.h>
#include <Sprites.h>
#include <Arduboy2Beep.h>

Arduboy2 ab;
Sprites sp;
BeepPin1 beep;

const uint8_t ledGraph[] PROGMEM = {
  6, 8,
  0x1e, 0x21, 0x21, 0x21, 0x21, 0x1e,
  0x1e, 0x3f, 0x3f, 0x3d, 0x33, 0x1e
};

const uint8_t outGraph[] PROGMEM = {
  11, 8,
  0x3e, 0x41, 0x3e, 0x00,
  0x3f, 0x40, 0x3f, 0x00,
  0x01, 0x7f, 0x01
};

const uint8_t romFont[] PROGMEM = {
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
const uint8_t inverter[] PROGMEM = { 0x7f, 0x7f, 0x7f, 0x7f, 0x7f};

const uint8_t triangle[] PROGMEM = {
  3, 8,
  0x10, 0x38, 0x7c,
  0x7c, 0x38, 0x10
};

// UP+DOWN reset
const uint8_t UD_RESET_WAIT = 150;
uint8_t udResetCount = 0;

// configuration
uint8_t cpuSpeed = 3;
uint8_t inputIsAlternate = 1;
uint8_t soundPort = 4;
const uint8_t CPU_TD4 = 0;
const uint8_t CPU_KAGEKI = 1;
const uint8_t CPU_TD4_STRICT = 2;
uint8_t cpuType = CPU_TD4;

uint8_t cpuSpeedIdToSpeed(uint8_t id) {
  uint8_t res = 1;
  while (id >= 3) {
    id -= 3;
    res *= 10;
  }
  if (id == 2) res *= 5;
  else if (id == 1) res *= 2;
  return res;
}

// screen select
const uint8_t SCREEN_MAIN = 0;
const uint8_t SCREEN_MENU = 1;
uint8_t screen = SCREEN_MAIN;

// main UI status
uint8_t running = 0;
uint8_t prescaler = 10, prescaleCount = 0;
uint8_t editing = 0;
uint8_t editx = 0, edity = 0;
const uint8_t EDIT_CURSOR_ANIM_PERIOD = 70;
uint8_t editCursorAnim = 0;
uint8_t prevIsAdd = 0;
const uint8_t UNDI_OP = 1;
const uint8_t UNDI_JNC = 2;
uint8_t undefinedInstruction = 0;

const uint8_t BUTTON_PRESS_NONE = 0;
const uint8_t BUTTON_PRESS_A = 1;
const uint8_t BUTTON_PRESS_AB = 2;
const uint8_t BUTTON_PRESS_B = 4;
const uint8_t BUTTON_PRESS_BA = 8;
uint8_t buttonStatus = BUTTON_PRESS_NONE;

// CPU status
uint8_t a = 0, b = 0, c = 0, pc = 0, out = 0, in = 0;
uint8_t rom[16];

// main UI draw status
const uint8_t ROM_REDRAW_NONE = 0xff;
uint8_t statusFullRedraw = 1;
uint8_t romIdxRedraw = ROM_REDRAW_NONE, romBitRedraw = ROM_REDRAW_NONE;
// drawn values
uint8_t ad = 0, bd = 0, cd = 0, pcd = 0, outd = 0, ind = 0;
uint8_t runningd = 0, editingd = 0, buttonStatusd = 0, uid = 0;
uint8_t editxd = 0, edityd = 0, editscd = 0;

const uint16_t beepCount = BeepPin1::freq(500);

// menu status
const uint8_t MENU_CPU = 0;
uint8_t menuPage = 0;
uint8_t menuSelect = 0;
uint8_t helpPage = 0;
uint8_t menuSelected = 0;
uint8_t menuDialogAutoClose = 0;
uint8_t menuConfirmSelect = 0;

// menu draw status
uint8_t menuRedraw = 0;

void resetCPU() {
  a = 0;
  b = 0;
  c = 0;
  pc = 0;
  out = 0;
  prevIsAdd = 0;
}

void emulateOneCycle() {
  if (cpuType == CPU_TD4) {
    uint8_t inst = rom[pc];
    uint8_t inputSel = ((inst >> 4) & 3) | (inst >> 7);
    uint8_t loadSel = inst >> 6;
    uint8_t input, aluRes;
    if (loadSel == 3 && !((inst & 0x10) || !c)) loadSel = 4;
    switch (inputSel) {
      case 0: input = a; break;
      case 1: input = b; break;
      case 2: input = in; break;
      default: input = 0; break;
    }
    aluRes = input + (inst & 0xf);
    c = aluRes >> 4;
    aluRes &= 0xf;
    pc = (pc + 1) & 0xf;
    switch (loadSel) {
      case 0: a = aluRes; break;
      case 1: b = aluRes; break;
      case 2: out = aluRes; break;
      case 3: pc = aluRes; break;
    }
    prevIsAdd = (inst >> 4) == 0x0 || (inst >> 4) == 0x5;
  } else if (cpuType == CPU_KAGEKI) {
    uint8_t inst = rom[pc];
    uint8_t imm = inst & 0xf;
    uint8_t srcSel = (inst >> 4) & 3;
    uint8_t wbSel = inst >> 6;
    uint8_t notCJump = srcSel & 1;
    uint8_t src, wb, newc;
    switch (srcSel) {
      case 0: src = a; break;
      case 1: src = b; break;
      case 2: src = in; break;
      default: src = 0; break;
    }
    if (wbSel == 3) src = 0;
    wb = src + imm;
    newc = wb >> 4;
    wb &= 0xf;
    pc = (pc + 1) & 0xf;
    switch (wbSel) {
      case 0: a = wb; break;
      case 1: b = wb; break;
      case 2: out = wb; break;
      case 3:
        if (!(!notCJump && c)) pc = wb;
        break;
    }
    c = newc;
    prevIsAdd = (inst >> 4) == 0x0 || (inst >> 4) == 0x5;
  } else if (cpuType == CPU_TD4_STRICT) {
    uint8_t inst = rom[pc];
    uint8_t op = inst >> 4;
    uint8_t Im = inst & 0xf;
    uint8_t newPC = (pc + 1) & 0xf, newC = 0, nextIsAdd = 0;
    if (op == 0x0) { // ADD A, Im
      a += Im;
      newC = a >> 4;
      a &= 0xf;
      nextIsAdd = 1;
    } else if (op == 0x5) { // ADD B, Im
      b += Im;
      newC = b >> 4;
      b &= 0xf;
      nextIsAdd = 1;
    } else if (op == 0x3) { // MOV A, Im
      a = Im;
    } else if (op == 0x7) { // MOV B, Im
      b = Im;
    } else if (op == 0x1 && Im == 0) { // MOV A, B
      a = b;
    } else if (op == 0x4 && Im == 0) { // MOV B, A
      b = a;
    } else if (op == 0xf) { // JMP Im
      newPC = Im;
    } else if (op == 0xe) { // JNC Im
      if (prevIsAdd) {
        if (!c) newPC = Im;
      } else {
        undefinedInstruction = UNDI_JNC;
      }
    } else if (op == 0x2 && Im == 0) { // IN A
      a = in;
    } else if (op == 0x6 && Im == 0) { // IN B
      b = in;
    } else if (op == 0x9 && Im == 0) { // OUT B
      out = b;
    } else if (op == 0xb) { // OUT Im
      out = Im;
    } else {
      undefinedInstruction = UNDI_OP;
    }
    if (!undefinedInstruction) {
      pc = newPC;
      c = newC;
      prevIsAdd = nextIsAdd;
    } else {
      running = 0;
    }
  }
}

void updateMainUI(uint8_t releasedButton) {
  if (undefinedInstruction) {
    if ((ab.justReleased(A_BUTTON) || ab.justReleased(B_BUTTON)) && ab.notPressed(A_BUTTON | B_BUTTON)) {
      undefinedInstruction = 0;
    }
  } else {
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
        beep.noTone();
        screen = SCREEN_MENU;
        menuRedraw = 1;
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
      if (inputIsAlternate) {
        if (ab.justReleased(LEFT_BUTTON)) in ^= 8;
        if (ab.justReleased(UP_BUTTON)) in ^= 4;
        if (ab.justReleased(DOWN_BUTTON)) in ^= 2;
        if (ab.justReleased(RIGHT_BUTTON)) in ^= 1;
      } else {
        in = 0;
        if (ab.pressed(LEFT_BUTTON)) in |= 8;
        if (ab.pressed(UP_BUTTON)) in |= 4;
        if (ab.pressed(DOWN_BUTTON)) in |= 2;
        if (ab.pressed(RIGHT_BUTTON)) in |= 1;
      }
    }
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

  if (statusFullRedraw || (uid && !undefinedInstruction)) {
    ab.clear();

    // draw status
    for (int i = 0; i < 4; i++) {
      sp.drawSelfMasked(LED_POS + 7 * i, PC_Y + 1, ledGraph, (pc >> (3 - i)) & 1);
      sp.drawSelfMasked(LED_POS + 7 * i, A_Y + 1, ledGraph, (a >> (3 - i)) & 1);
      sp.drawSelfMasked(LED_POS + 7 * i, B_Y + 1, ledGraph, (b >> (3 - i)) & 1);
      sp.drawSelfMasked(LED_POS + 7 * i, OUT_Y + 1, ledGraph, (out >> (3 - i)) & 1);
      sp.drawSelfMasked(LED_POS + 7 * i, IN_Y + 1, ledGraph, (in >> (3 - i)) & 1);
    }
    sp.drawSelfMasked(LED_POS, C_Y + 1, ledGraph, c & 1);
    ab.setCursor(0, PC_Y); ab.print(F("PC"));
    ab.setCursor(3, A_Y); ab.write('A');
    ab.setCursor(3, B_Y); ab.write('B');
    ab.setCursor(3, C_Y); ab.write('C');
    sp.drawSelfMasked(0, OUT_Y, outGraph, 0);
    ab.setCursor(0, IN_Y); ab.print(F("IN"));

    // set beep status
    if (out & (1 << soundPort)) {
      beep.tone(beepCount);
    } else {
      beep.noTone();
    }

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
      if (out & (1 << soundPort)) {
        if (!(outd & (1 << soundPort))) beep.tone(beepCount);
      } else {
        beep.noTone();
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
  if (buttonStatus != buttonStatusd || running != runningd || editingChanged || undefinedInstruction != uid) {
    ab.fillRect(0, 57, 128, 7, BLACK);
    if (undefinedInstruction) {
      ab.fillRect(16 - 5 - 2, 22 - 5 - 2, 2 + 5 + 6 * 16 + 4 + 2, 2 + 5 + 7 + 5 + 7 + 5 + 2, BLACK);
      ab.drawRect(16 - 5 - 1, 22 - 5 - 1, 1 + 5 + 6 * 16 + 4 + 1, 1 + 5 + 7 + 5 + 7 + 5 + 1, WHITE);
      if (undefinedInstruction == UNDI_OP) {
        ab.setCursor(16, 22);
        ab.print(F("UNDEFINED OPCODE"));
      } else if (undefinedInstruction == UNDI_JNC) {
        ab.setCursor(16 + 3, 22);
        ab.print(F("JNC without ADD"));
      }
      ab.setCursor(43, 22 + 7 + 5);
      ab.print(F("PRESS A"));
    } else {
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
    }
    runningd = running;
    editingd = editing;
    buttonStatusd = buttonStatus;
    uid = undefinedInstruction;
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

void updateMenu() {
  if (ab.justReleased(RIGHT_BUTTON)) {
    if (menuSelected) {
      if (menuPage == 0) {
        switch (menuSelect) {
          case 1: cpuSpeed = cpuSpeed == 6 ? 0 : cpuSpeed + 1; break;
          case 2: inputIsAlternate = 1 - inputIsAlternate; break;
          case 3: soundPort = soundPort == 4 ? 0 : soundPort + 1; break;
          case 4: cpuType = cpuType == CPU_TD4_STRICT ? CPU_TD4 : cpuType + 1; break;
        }
      } else if (menuPage == 1) {
        if (menuSelect == 0) {
          if (menuConfirmSelect == 0) menuConfirmSelect = 1;
          else if (menuConfirmSelect == 1) menuConfirmSelect = 0;
        }
      }
    } else {
      menuPage++;
      if (menuPage > 3) menuPage = 0;
      if (menuPage == 2 && menuSelect > 2) menuSelect = 2;
    }
    menuRedraw = 1;
  }
  if (ab.justReleased(LEFT_BUTTON)) {
    if (menuSelected) {
      if (menuPage == 0) {
        switch (menuSelect) {
          case 1: cpuSpeed = cpuSpeed == 0 ? 6 : cpuSpeed - 1; break;
          case 2: inputIsAlternate = 1 - inputIsAlternate; break;
          case 3: soundPort = soundPort == 0 ? 4 : soundPort - 1; break;
          case 4: cpuType = cpuType == CPU_TD4 ? CPU_TD4_STRICT : cpuType - 1; break;
        }
      } else if (menuPage == 1) {
        if (menuSelect == 0) {
          if (menuConfirmSelect == 0) menuConfirmSelect = 1;
          else if (menuConfirmSelect == 1) menuConfirmSelect = 0;
        }
      }
    } else {
      if (menuPage == 0) menuPage = 3; else menuPage--;
      if (menuPage == 2 && menuSelect > 2) menuSelect = 2;
    }
    menuRedraw = 1;
  }
  if (ab.justReleased(DOWN_BUTTON)) {
    if (menuSelected) {
      if (menuPage == 0) {
        if (menuSelect == 4) menuSelect = 1;
        else if (menuSelect > 0) menuSelect++;
      }
    } else {
      if (menuPage == 3) {
        helpPage++;
        if (helpPage > 1) helpPage = 0;
      } else {
        menuSelect++;
        if (menuSelect > (menuPage == 2 ? 2 : 4)) menuSelect = 0;
      }
    }
    menuRedraw = 1;
  }
  if (ab.justReleased(UP_BUTTON)) {
    if (menuSelected) {
      if (menuPage == 0) {
        if (menuSelect == 1) menuSelect = 4;
        else if (menuSelect > 0) menuSelect--;
      }
    } else {
      if (menuPage == 3) {
        if (helpPage == 0) helpPage = 1; else helpPage--;
      } else {
        if (menuSelect == 0) menuSelect = (menuPage == 2 ? 2 : 4);
        else menuSelect--;
      }
    }
    menuRedraw = 1;
  }
  if (ab.justReleased(A_BUTTON)) {
    if (menuSelected) {
      if (menuPage == 1) {
        if (menuSelect == 0) {
          // ROM clear
          if (menuConfirmSelect == 1) {
            for (int i = 0; i < 16; i++) rom[i] = 0;
            menuConfirmSelect = 2;
            menuDialogAutoClose = 50;
          } else {
            menuSelected = 0;
            menuDialogAutoClose = 0;
          }
        }
      } else {
        menuSelected = 0;
        menuDialogAutoClose = 0;
      }
    } else {
      if (menuPage == 0 && menuSelect == 0) {
        resetCPU();
        menuDialogAutoClose = 50;
      }
      if (menuPage == 1 && menuSelect == 0) {
        menuConfirmSelect = 0;
      }
      if (menuPage != 3) {
        menuSelected = 1;
      }
    }
    menuRedraw = 1;
  }
  if (ab.justReleased(B_BUTTON)) {
    if (menuSelected) {
      menuSelected = 0;
      menuDialogAutoClose = 0;
      menuRedraw = 1;
    } else {
      prescaler = 100 / cpuSpeedIdToSpeed(cpuSpeed);
      screen = SCREEN_MAIN;
      statusFullRedraw = 1;
    }
  }
  if (menuDialogAutoClose > 0) {
    menuDialogAutoClose--;
    if (menuDialogAutoClose == 0) {
      menuSelected = 0;
      menuRedraw = 1;
    }
  }
}

void drawMenu() {
  if (!menuRedraw) return;
  ab.clear();

  // draw menu tabs
  const int MENU_OFFSET = 6;
  int subMenuTextNum = 0;
  if (menuPage == 0) {
    subMenuTextNum = 5;
    ab.fillRect(MENU_OFFSET + 0, 0, 6 * 3 + 1, 9, WHITE);
    ab.setTextColor(BLACK);
  } else {
    ab.setTextColor(WHITE);
  }
  ab.setCursor(MENU_OFFSET + 1, 1);
  ab.print(F("CPU"));

  if (menuPage == 1) {
    subMenuTextNum = 6;
    ab.fillRect(MENU_OFFSET + 24, 0, 6 * 3 + 1, 9, WHITE);
    ab.setTextColor(BLACK);
  } else {
    ab.setTextColor(WHITE);
  }
  ab.setCursor(MENU_OFFSET + 25, 1);
  ab.print(F("ROM"));

  if (menuPage == 2) {
    subMenuTextNum = 10;
    ab.fillRect(MENU_OFFSET + 48, 0, 6 * 6 + 1, 9, WHITE);
    ab.setTextColor(BLACK);
  } else {
    ab.setTextColor(WHITE);
  }
  ab.setCursor(MENU_OFFSET + 49, 1);
  ab.print(F("EEPROM"));

  if (menuPage == 3) {
    ab.fillRect(MENU_OFFSET + 90, 0, 6 * 4 + 1, 9, WHITE);
    ab.setTextColor(BLACK);
  } else {
    ab.setTextColor(WHITE);
  }
  ab.setCursor(MENU_OFFSET + 91, 1);
  ab.print(F("HELP"));

  ab.drawFastHLine(0, 9, 128, WHITE);

  // draw sub menus
  const int SUBMENU_Y = 11;
  if (menuPage != 3) {
    if (menuSelect == 0) {
      ab.fillRect(0, SUBMENU_Y, 6 * subMenuTextNum + 1, 9, WHITE);
      ab.setTextColor(BLACK);
    } else {
      ab.setTextColor(WHITE);
    }
    ab.setCursor(1, SUBMENU_Y + 1);
    if (menuPage == 0) ab.print(F("RESET"));
    else if (menuPage == 1) ab.print(F("CLEAR"));
    else if (menuPage == 2) ab.print(F("ERASE  ALL"));

    if (menuSelect == 1) {
      ab.fillRect(0, SUBMENU_Y + 9 * 1, 6 * subMenuTextNum + 1, 9, WHITE);
      ab.setTextColor(BLACK);
    } else {
      ab.setTextColor(WHITE);
    }
    ab.setCursor(1, SUBMENU_Y + 1 + 9 * 1);
    if (menuPage == 0) ab.print(F("SPEED"));
    else if (menuPage == 1) ab.print(F("LOAD"));
    else if (menuPage == 2) ab.print(F("IMPORT ALL"));

    if (menuSelect == 2) {
      ab.fillRect(0, SUBMENU_Y + 9 * 2, 6 * subMenuTextNum + 1, 9, WHITE);
      ab.setTextColor(BLACK);
    } else {
      ab.setTextColor(WHITE);
    }
    ab.setCursor(1, SUBMENU_Y + 1 + 9 * 2);
    if (menuPage == 0) ab.print(F("INPUT"));
    else if (menuPage == 1) ab.print(F("SAVE"));
    else if (menuPage == 2) ab.print(F("EXPORT ALL"));

    if (menuSelect == 3) {
      ab.fillRect(0, SUBMENU_Y + 9 * 3, 6 * subMenuTextNum + 1, 9, WHITE);
      ab.setTextColor(BLACK);
    } else {
      ab.setTextColor(WHITE);
    }
    ab.setCursor(1, SUBMENU_Y + 1 + 9 * 3);
    if (menuPage == 0) ab.print(F("SOUND"));
    else if (menuPage == 1) ab.print(F("IMPORT"));

    if (menuSelect == 4) {
      ab.fillRect(0, SUBMENU_Y + 9 * 4, 6 * subMenuTextNum + 1, 9, WHITE);
      ab.setTextColor(BLACK);
    } else {
      ab.setTextColor(WHITE);
    }
    ab.setCursor(1, SUBMENU_Y + 1 + 9 * 4);
    if (menuPage == 0) ab.print(F("TYPE"));
    else if (menuPage == 1) ab.print(F("EXPORT"));
  }

  ab.setTextColor(WHITE);
  if (menuPage == 2) {
    ab.setCursor(1, SUBMENU_Y + 1 + 9 * 3);
    ab.print(F("FREE"));
    ab.setCursor(1, SUBMENU_Y + 1 + 9 * 4);
    ab.print(F("USED"));
    ab.setCursor(1, SUBMENU_Y + 1 + 9 * 5);
    ab.print(F("RESERVED"));
  } else if (menuPage == 3) {
    if (helpPage == 0) {
      ab.setCursor(1, SUBMENU_Y + 9 * 0);
      ab.print(F("0011 mmmm  MOV A, Im"));
      ab.setCursor(1, SUBMENU_Y + 9 * 1);
      ab.print(F("0111 mmmm  MOV B, Im"));
      ab.setCursor(1, SUBMENU_Y + 9 * 2);
      ab.print(F("0001 0000  MOV A, B"));
      ab.setCursor(1, SUBMENU_Y + 9 * 3);
      ab.print(F("0100 0000  MOV B, A"));
      ab.setCursor(1, SUBMENU_Y + 9 * 4);
      ab.print(F("0000 mmmm  ADD A, Im"));
      ab.setCursor(1, SUBMENU_Y + 9 * 5);
      ab.print(F("0101 mmmm  ADD B, Im"));
    } else if (helpPage == 1) {
      ab.setCursor(1, SUBMENU_Y + 9 * 0);
      ab.print(F("0010 0000  IN  A"));
      ab.setCursor(1, SUBMENU_Y + 9 * 1);
      ab.print(F("0110 0000  IN  B"));
      ab.setCursor(1, SUBMENU_Y + 9 * 2);
      ab.print(F("1011 mmmm  OUT Im"));
      ab.setCursor(1, SUBMENU_Y + 9 * 3);
      ab.print(F("1001 0000  OUT B"));
      ab.setCursor(1, SUBMENU_Y + 9 * 4);
      ab.print(F("1111 mmmm  JMP Im"));
      ab.setCursor(1, SUBMENU_Y + 9 * 5);
      ab.print(F("1110 mmmm  JNC Im"));
    }
  }

  // draw menu data
  if (menuPage == 0) {
    const int MENU_DATA_X = 6 * 8;
    ab.setCursor(MENU_DATA_X, SUBMENU_Y + 1 + 9 * 1);
    ab.print(cpuSpeedIdToSpeed(cpuSpeed));
    ab.print(F("Hz"));
    ab.setCursor(MENU_DATA_X, SUBMENU_Y + 1 + 9 * 2);
    if (inputIsAlternate) {
      ab.print(F("ON/OFF"));
    } else {
      ab.print(F("PUSH ON"));
    }
    ab.setCursor(MENU_DATA_X, SUBMENU_Y + 1 + 9 * 3);
    if (soundPort < 4) {
      ab.print(F("OUT "));
      ab.print(soundPort);
    } else {
      ab.print(F("OFF"));
    }
    ab.setCursor(MENU_DATA_X, SUBMENU_Y + 1 + 9 * 4);
    switch (cpuType) {
      case CPU_TD4: ab.print(F("TD4")); break;
      case CPU_KAGEKI: ab.print(F("Kageki")); break;
      case CPU_TD4_STRICT: ab.print(F("TD4 STRICT")); break;
    }
    if (menuSelected) {
      switch (menuSelect) {
        case 0:
          {
            const int X = 37, Y = 28;
            ab.fillRect(X - 5 - 2, Y - 5 - 2, 2 + 5 + 6 * 9 + 4 + 2, 2 + 5 + 8 + 4 + 2, BLACK);
            ab.drawRect(X - 5 - 1, Y - 5 - 1, 1 + 5 + 6 * 9 + 4 + 1, 1 + 5 + 8 + 4 + 1, WHITE);
            ab.setCursor(X, Y);
            ab.print(F("CPU RESET"));
          }
          break;
        case 1:
          sp.drawSelfMasked(MENU_DATA_X - 7, SUBMENU_Y + 9 * 1, triangle, 0);
          sp.drawSelfMasked(MENU_DATA_X + 3 + 6 * 5, SUBMENU_Y + 9 * 1, triangle, 1);
          break;
        case 2:
          sp.drawSelfMasked(MENU_DATA_X - 7, SUBMENU_Y + 9 * 2, triangle, 0);
          sp.drawSelfMasked(MENU_DATA_X + 3 + 6 * 7, SUBMENU_Y + 9 * 2, triangle, 1);
          break;
        case 3:
          sp.drawSelfMasked(MENU_DATA_X - 7, SUBMENU_Y + 9 * 3, triangle, 0);
          sp.drawSelfMasked(MENU_DATA_X + 3 + 6 * 5, SUBMENU_Y + 9 * 3, triangle, 1);
          break;
        case 4:
          sp.drawSelfMasked(MENU_DATA_X - 7, SUBMENU_Y + 9 * 4, triangle, 0);
          sp.drawSelfMasked(MENU_DATA_X + 3 + 6 * 10, SUBMENU_Y + 9 * 4, triangle, 1);
          break;
      }
    }
  } else if (menuPage == 1) {
    if (menuSelect == 1 || menuSelect == 2) {
      const int MENU_DATA_X = 6 * 7 + 4;
      for (int i = 0; i < 5; i++) {
        ab.setCursor(MENU_DATA_X, SUBMENU_Y + 1 + 9 * i);
        ab.print(F("999. "));
        ab.print(F("ABCDEFGH"));
      }
      ab.setCursor(MENU_DATA_X + 12, SUBMENU_Y + 1 + 9 * 5);
      ab.print(F("999 / 999"));
    }
    if (menuSelect == 0 && menuSelected) {
      if (menuConfirmSelect == 2) {
        ab.fillRect(31 - 5 - 2, 28 - 5 - 2, 2 + 5 + 6 * 11 + 4 + 2, 2 + 5 + 7 + 5 + 2, BLACK);
        ab.drawRect(31 - 5 - 1, 28 - 5 - 1, 1 + 5 + 6 * 11 + 4 + 1, 1 + 5 + 7 + 5 + 1, WHITE);
        ab.setCursor(31, 28);
        ab.print(F("ROM CLEARED"));
      } else {
        ab.fillRect(34 - 5 - 2, 22 - 5 - 2, 2 + 5 + 6 * 10 + 4 + 2, 2 + 5 + 7 + 5 + 7 + 5 + 2, BLACK);
        ab.drawRect(34 - 5 - 1, 22 - 5 - 1, 1 + 5 + 6 * 10 + 4 + 1, 1 + 5 + 7 + 5 + 7 + 5 + 1, WHITE);
        ab.setCursor(34, 22);
        ab.print(F("CLEAR ROM?"));
        if (menuConfirmSelect == 1) {
          ab.fillRect(34 + 6 - 1, 22 + 7 + 5 - 1, 19, 9, WHITE);
          ab.setTextColor(BLACK);
        }
        ab.setCursor(34 + 6, 22 + 7 + 5);
        ab.print(F("YES"));
        if (menuConfirmSelect == 0) {
          ab.fillRect(34 + 6 * 6 - 1, 22 + 7 + 5 - 1, 19, 9, WHITE);
          ab.setTextColor(BLACK);
        } else {
          ab.setTextColor(WHITE);
        }
        ab.setCursor(34 + 6 * 6 + 3, 22 + 7 + 5);
        ab.print(F("NO"));
        ab.setTextColor(WHITE);
      }
    }
  } else if (menuPage == 2) {
    const int MENU_DATA_X = 6 * 10;
    ab.setCursor(MENU_DATA_X, SUBMENU_Y + 1 + 9 * 3);
    ab.print(F("999 BLOCKS"));
    ab.setCursor(MENU_DATA_X, SUBMENU_Y + 1 + 9 * 4);
    ab.print(F("999 BLOCKS"));
    ab.setCursor(MENU_DATA_X, SUBMENU_Y + 1 + 9 * 5);
    ab.print(F("999 BLOCKS"));
  }

  menuRedraw = 0;
}

void setup() {
  ab.begin();
  ab.setTextSize(1);
  ab.setFrameRate(100);
  beep.begin();
}

void loop() {
  if (!ab.nextFrame()) {
    ab.idle();
    return;
  }
  ab.pollButtons();
  beep.timer();
  if (ab.pressed(UP_BUTTON | DOWN_BUTTON) && !(screen == SCREEN_MAIN && !editing && !inputIsAlternate)) {
    udResetCount++;
    if (udResetCount >= UD_RESET_WAIT) ab.exitToBootloader();
  } else {
    udResetCount = 0;
  }
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

  switch (screen) {
    case SCREEN_MAIN:
      updateMainUI(releasedButtonStatus);
      drawMainUI();
      break;
    case SCREEN_MENU:
      updateMenu();
      drawMenu();
      break;
  }

  ab.display();
}
