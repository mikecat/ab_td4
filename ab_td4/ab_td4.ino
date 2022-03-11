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

uint8_t a = 0, b = 0, c = 0, pc = 0, out = 0, in = 0;
uint8_t rom[16];

uint8_t statusFullRedraw = 0;
// drawn values
uint8_t ad = 0, bd = 0, cd = 0, pcd = 0, outd = 0, ind = 0;

void drawStatus() {
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
    ab.setCursor(0, PC_Y); ab.print("PC");
    ab.setCursor(3, A_Y); ab.write('A');
    ab.setCursor(3, B_Y); ab.write('B');
    ab.setCursor(3, C_Y); ab.write('C');
    sp.drawSelfMasked(0, OUT_Y, outGraph, 0);
    ab.setCursor(0, IN_Y); ab.print("IN");
  
    // draw ROM contents
    for (int i = 0; i < 8; i++) {
      int y = 1 + 7 * i;
      sp.drawSelfMasked(ROM_C1 + 1, y, romFont, i);
      sp.drawSelfMasked(ROM_C1 + 5, y, romFont, 16);
      for (int j = 0; j < 8; j++) {
        sp.drawSelfMasked(ROM_C1 + 8 + 4 * j + 2 * (j >= 4), y, romFont, (rom[i] >> (7 - i)) & 1);
      }
      sp.drawSelfMasked(ROM_C2 + 1, y, romFont, 8 + i);
      sp.drawSelfMasked(ROM_C2 + 5, y, romFont, 16);
      for (int j = 0; j < 8; j++) {
        sp.drawSelfMasked(ROM_C2 + 8 + 4 * j + 2 * (j >= 4), y, romFont, (rom[8 + i] >> (7 - i)) & 1);
      }
    }
    // draw PC marker on ROM
    ab.drawBitmap(pc & 8 ? ROM_C2 : ROM_C1, 7 * (pc % 8), inverter, 5, 7, INVERT);

    // save what is drawn
    statusFullRedraw = 0;
    ad = a; bd = b; cd = c; pcd = pc; outd = out; ind = in;
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
  if (ab.justPressed(A_BUTTON)) a++;
  if (ab.justPressed(B_BUTTON)) b++;
  if (ab.justPressed(LEFT_BUTTON)) pc++;
  if (ab.justPressed(RIGHT_BUTTON)) c++;
  if (ab.justPressed(UP_BUTTON)) out++;
  if (ab.justPressed(DOWN_BUTTON)) in++;

  drawStatus();
  ab.display();
}
