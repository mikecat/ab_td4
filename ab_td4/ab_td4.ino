#include <Arduboy2.h>
#include <Sprites.h>
#include <Arduboy2Beep.h>
#include <avr/pgmspace.h>

Arduboy2 ab;
Sprites sp;
BeepPin1 beep;

const uint16_t crcTable[256] PROGMEM = {
  0x0000u, 0x1021u, 0x2042u, 0x3063u, 0x4084u, 0x50a5u, 0x60c6u, 0x70e7u, 0x8108u, 0x9129u, 0xa14au, 0xb16bu, 0xc18cu, 0xd1adu, 0xe1ceu, 0xf1efu,
  0x1231u, 0x0210u, 0x3273u, 0x2252u, 0x52b5u, 0x4294u, 0x72f7u, 0x62d6u, 0x9339u, 0x8318u, 0xb37bu, 0xa35au, 0xd3bdu, 0xc39cu, 0xf3ffu, 0xe3deu,
  0x2462u, 0x3443u, 0x0420u, 0x1401u, 0x64e6u, 0x74c7u, 0x44a4u, 0x5485u, 0xa56au, 0xb54bu, 0x8528u, 0x9509u, 0xe5eeu, 0xf5cfu, 0xc5acu, 0xd58du,
  0x3653u, 0x2672u, 0x1611u, 0x0630u, 0x76d7u, 0x66f6u, 0x5695u, 0x46b4u, 0xb75bu, 0xa77au, 0x9719u, 0x8738u, 0xf7dfu, 0xe7feu, 0xd79du, 0xc7bcu,
  0x48c4u, 0x58e5u, 0x6886u, 0x78a7u, 0x0840u, 0x1861u, 0x2802u, 0x3823u, 0xc9ccu, 0xd9edu, 0xe98eu, 0xf9afu, 0x8948u, 0x9969u, 0xa90au, 0xb92bu,
  0x5af5u, 0x4ad4u, 0x7ab7u, 0x6a96u, 0x1a71u, 0x0a50u, 0x3a33u, 0x2a12u, 0xdbfdu, 0xcbdcu, 0xfbbfu, 0xeb9eu, 0x9b79u, 0x8b58u, 0xbb3bu, 0xab1au,
  0x6ca6u, 0x7c87u, 0x4ce4u, 0x5cc5u, 0x2c22u, 0x3c03u, 0x0c60u, 0x1c41u, 0xedaeu, 0xfd8fu, 0xcdecu, 0xddcdu, 0xad2au, 0xbd0bu, 0x8d68u, 0x9d49u,
  0x7e97u, 0x6eb6u, 0x5ed5u, 0x4ef4u, 0x3e13u, 0x2e32u, 0x1e51u, 0x0e70u, 0xff9fu, 0xefbeu, 0xdfddu, 0xcffcu, 0xbf1bu, 0xaf3au, 0x9f59u, 0x8f78u,
  0x9188u, 0x81a9u, 0xb1cau, 0xa1ebu, 0xd10cu, 0xc12du, 0xf14eu, 0xe16fu, 0x1080u, 0x00a1u, 0x30c2u, 0x20e3u, 0x5004u, 0x4025u, 0x7046u, 0x6067u,
  0x83b9u, 0x9398u, 0xa3fbu, 0xb3dau, 0xc33du, 0xd31cu, 0xe37fu, 0xf35eu, 0x02b1u, 0x1290u, 0x22f3u, 0x32d2u, 0x4235u, 0x5214u, 0x6277u, 0x7256u,
  0xb5eau, 0xa5cbu, 0x95a8u, 0x8589u, 0xf56eu, 0xe54fu, 0xd52cu, 0xc50du, 0x34e2u, 0x24c3u, 0x14a0u, 0x0481u, 0x7466u, 0x6447u, 0x5424u, 0x4405u,
  0xa7dbu, 0xb7fau, 0x8799u, 0x97b8u, 0xe75fu, 0xf77eu, 0xc71du, 0xd73cu, 0x26d3u, 0x36f2u, 0x0691u, 0x16b0u, 0x6657u, 0x7676u, 0x4615u, 0x5634u,
  0xd94cu, 0xc96du, 0xf90eu, 0xe92fu, 0x99c8u, 0x89e9u, 0xb98au, 0xa9abu, 0x5844u, 0x4865u, 0x7806u, 0x6827u, 0x18c0u, 0x08e1u, 0x3882u, 0x28a3u,
  0xcb7du, 0xdb5cu, 0xeb3fu, 0xfb1eu, 0x8bf9u, 0x9bd8u, 0xabbbu, 0xbb9au, 0x4a75u, 0x5a54u, 0x6a37u, 0x7a16u, 0x0af1u, 0x1ad0u, 0x2ab3u, 0x3a92u,
  0xfd2eu, 0xed0fu, 0xdd6cu, 0xcd4du, 0xbdaau, 0xad8bu, 0x9de8u, 0x8dc9u, 0x7c26u, 0x6c07u, 0x5c64u, 0x4c45u, 0x3ca2u, 0x2c83u, 0x1ce0u, 0x0cc1u,
  0xef1fu, 0xff3eu, 0xcf5du, 0xdf7cu, 0xaf9bu, 0xbfbau, 0x8fd9u, 0x9ff8u, 0x6e17u, 0x7e36u, 0x4e55u, 0x5e74u, 0x2e93u, 0x3eb2u, 0x0ed1u, 0x1ef0u
};

uint16_t crcUpdate(uint8_t data, uint16_t crc) {
  return pgm_read_word_near(crcTable + ((crc >> 8) ^ data)) ^ (crc << 8);
}

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

void eepromInitialize() {
}

bool eepromIsBusy() {
  return !!(EECR & 2); // EEPE == 1 -> busy
}

void eepromErase(int address) {
  while (eepromIsBusy());
  if (EEPROM.read(address) == 0xff) return;
  EEAR = address;
  // set EEPM = 0b01 (erase only)
  EECR = (EECR & 0xcf) | 0x10;
  __asm__ __volatile__(
    // EECR I/O address = 0x1f
    "in r18, 0x1f\n\t"
    "andi r18, 0xf9\n\t"
    "ori r18, 0x04\n\t"
    "cli\n\t"
    "out 0x1f, r18\n\t" // EEMPE = 1, EEPE = 0
    "ori r18, 0x02\n\t"
    "out 0x1f, r18\n\t" // EEPE = 1
    "sei\n\t"
  : : : "r18");
}

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
uint8_t romImportBuffer[16];
uint8_t romImportPos = 0;
uint8_t romExportPos = 0;
uint8_t prevSerialStatus = 0;

// menu draw status
uint8_t menuRedraw = 0;

// EEPROM erase/import/export
enum {
  // done
  EEPROM_COMM_SUCCEEDED,
  EEPROM_COMM_FAILED,
  EEPROM_COMM_CANCELED,
  // not running
  EEPROM_COMM_NONE,
  // ready to run
  EEPROM_COMM_RECV_CONNECT,
  EEPROM_COMM_SEND_CONNECT,
  // running (disable idle)
  EEPROM_COMM_DISABLE_IDLE_BORDER,
  EEPROM_COMM_ERASING = EEPROM_COMM_DISABLE_IDLE_BORDER,
  EEPROM_COMM_RECV_INIT,
  EEPROM_COMM_RECV_READING,
  EEPROM_COMM_RECV_WRITING,
  EEPROM_COMM_SEND_INIT,
  EEPROM_COMM_SEND_SENDING,
  EEPROM_COMM_SEND_WAITING_ACK
};
uint8_t eepromCommStatus = EEPROM_COMM_NONE;
uint8_t eepromCommUseCrc;
unsigned long eepromCommStartTime;
uint8_t eepromCommRetryCount;
uint8_t eepromCommBuffer[1 + 2 + 128 + 2];
uint16_t eepromCommCrc;
uint8_t eepromCommSeqId;
uint8_t eepromCommBlockPos;
int eepromCommPointer;
uint8_t eepromCommCancelRequest;
uint8_t eepromCommCanCount;

const uint8_t SOH = 0x01, EOT = 0x04, ACK = 0x06, NAK = 0x15, CAN = 0x18;

uint8_t prevEepromCommStatusKind;
int prevEepromCommPointer;
uint8_t menuFramePrescalerWhileComm;

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
      } else if (menuPage == 2) {
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
      } else if (menuPage == 2) {
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
      } else if (menuPage == 2) {
        if (menuSelect == 0) {
          // EEPROM erase
          if (menuConfirmSelect == 1) {
            menuConfirmSelect = 2;
            eepromCommStatus = EEPROM_COMM_ERASING;
            eepromCommPointer = EEPROM_STORAGE_SPACE_START;
          } else if (menuConfirmSelect == 0) {
            menuSelected = 0;
            menuDialogAutoClose = 0;
          }
        } else {
          // EEPROM import/export
          if (eepromCommStatus < EEPROM_COMM_NONE) {
            eepromCommStatus = EEPROM_COMM_NONE;
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
      } else if (menuPage == 1) {
        if (menuSelect == 0) {
          menuConfirmSelect = 0;
        } else if (menuSelect == 3) {
          romImportPos = 0;
          if (Serial) {
            while (Serial.available() > 0) Serial.read();
          }
        } else if (menuSelect == 4) {
          romExportPos = 0;
        }
      } else if (menuPage == 2) {
        if (menuSelect == 0) {
          menuConfirmSelect = 0;
        } else if (menuSelect == 1 || menuSelect == 2) {
          if (Serial) {
            while (Serial.available() > 0) Serial.read();
          }
          if (menuSelect == 1) {
            eepromCommStatus = EEPROM_COMM_RECV_CONNECT;
          } else {
            eepromCommStatus = EEPROM_COMM_SEND_CONNECT;
          }
          eepromCommCancelRequest = 0;
          eepromCommCanCount = 0;
          eepromCommPointer = EEPROM_STORAGE_SPACE_START;
        }
      }
      if (menuPage != 3) {
        menuSelected = 1;
      }
    }
    menuRedraw = 1;
  }
  if (ab.justReleased(B_BUTTON)) {
    if (menuSelected) {
      if (menuPage == 2) {
        if (menuSelect == 0) {
          // EEPROM erase
          if (menuConfirmSelect != 2) {
            menuSelected = 0;
            menuDialogAutoClose = 0;
            menuRedraw = 1;
          }
        } else {
          // EEPROM import/export
          if (eepromCommStatus >= EEPROM_COMM_DISABLE_IDLE_BORDER) {
            eepromCommCancelRequest = 1;
          } else {
            eepromCommStatus = EEPROM_COMM_NONE;
            menuSelected = 0;
            menuDialogAutoClose = 0;
            menuRedraw = 1;
          }
        }
      } else {
        menuSelected = 0;
        menuDialogAutoClose = 0;
        menuRedraw = 1;
      }
    } else {
      prescaler = 100 / cpuSpeedIdToSpeed(cpuSpeed);
      screen = SCREEN_MAIN;
      statusFullRedraw = 1;
    }
  }
  if (menuPage == 1 && menuSelected) {
    if (menuSelect == 3) {
      if (romImportPos < 32 && Serial) {
        while (Serial.available() > 0) {
          int c = Serial.read();
          int d = -1;
          if ('0' <= c && c <= '9') d = c - '0';
          else if ('A' <= c && c <= 'F') d = c - 'A' + 10;
          else if ('a' <= c && c <= 'f') d = c - 'a' + 10;
          if (d >= 0) {
            if (romImportPos % 2 == 0) {
              romImportBuffer[romImportPos / 2] = d << 4;
            } else {
              romImportBuffer[romImportPos / 2] |= d;
            }
            romImportPos++;
            if (romImportPos >= 32) {
              for (int i = 0; i < 16; i++) rom[i] = romImportBuffer[i];
              menuDialogAutoClose = 50;
              menuRedraw = 1;
              break;
            }
          }
        }
      }
    } else if (menuSelect == 4) {
      if (romExportPos < 33 && Serial) {
        while (Serial.availableForWrite() > 0) {
          char c;
          if (romExportPos >= 32) {
            c = '\n';
          } else {
            uint8_t cu = rom[romExportPos / 2];
            if (romExportPos % 2 == 0) cu >>= 4; else cu &= 0xf;
            cu += cu > 9 ? 'a' - 10 : '0';
            c = cu;
          }
          Serial.print(c);
          romExportPos++;
          if (romExportPos > 32) {
            menuDialogAutoClose = 50;
            menuRedraw = 1;
            break;
          }
        }
      }
    }
  }
  if (menuPage == 2 && menuSelected && menuSelect == 0) {
    if (eepromCommStatus == EEPROM_COMM_SUCCEEDED && menuDialogAutoClose == 0) {
      eepromInitialize();
      eepromCommStatus = EEPROM_COMM_NONE;
      menuDialogAutoClose = 50;
    }
  }
  if (eepromCommStatus == EEPROM_COMM_RECV_CONNECT && Serial) {
    eepromCommUseCrc = 1;
    eepromCommRetryCount = 0;
    eepromCommStartTime = millis();
    Serial.write('C');
    eepromCommStatus = EEPROM_COMM_RECV_INIT;
  }
  if (eepromCommStatus == EEPROM_COMM_SEND_CONNECT && Serial) {
    eepromCommStatus = EEPROM_COMM_SEND_INIT;
  }
  if (menuDialogAutoClose > 0) {
    menuDialogAutoClose--;
    if (menuDialogAutoClose == 0) {
      menuSelected = 0;
      menuRedraw = 1;
    }
  }
  uint8_t currentSerialStatus = !!Serial;
  if (currentSerialStatus != prevSerialStatus) {
    menuRedraw = 1;
    prevSerialStatus = currentSerialStatus;
  }
  uint8_t eepromCommStatusKind =
    (eepromCommStatus >= EEPROM_COMM_NONE) +
    (eepromCommStatus > EEPROM_COMM_NONE) +
    (eepromCommStatus >= EEPROM_COMM_DISABLE_IDLE_BORDER);
  if (eepromCommStatusKind != prevEepromCommStatusKind) {
    menuRedraw = 1;
    prevEepromCommStatusKind = eepromCommStatusKind;
  }
}

void drawMenu() {
  if (!menuRedraw) {
    if (eepromCommPointer != prevEepromCommPointer) {
      if (menuPage == 2 && menuSelected) {
        if (menuSelect == 0 && menuConfirmSelect == 2 && eepromCommStatus == EEPROM_COMM_ERASING) {
          ab.fillRect(26, 22 + 7 + 5 + 1, 6 * 13 - 1 - 2, BLACK);
          ab.fillRect(26, 22 + 7 + 5 + 1, (long)(6 * 13 - 1 - 2) * (eepromCommPointer - EEPROM_STORAGE_SPACE_START) / (EEPROM.length() - EEPROM_STORAGE_SPACE_START), 5, WHITE);
        } else if (eepromCommStatus >= EEPROM_COMM_DISABLE_IDLE_BORDER) {
          int graphPos = eepromCommPointer;
          if (graphPos > EEPROM.length()) graphPos = EEPROM.length();
          ab.fillRect(23, 21 + 7 + 5 + 1, 6 * 14 - 1 - 2, BLACK);
          ab.fillRect(23, 21 + 7 + 5 + 1, (long)(6 * 14 - 1 - 2) * (graphPos - EEPROM_STORAGE_SPACE_START) / (EEPROM.length() - EEPROM_STORAGE_SPACE_START), 5, WHITE);
        }
      }
      prevEepromCommPointer = eepromCommPointer;
    }
    return;
  }
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
    if (menuSelect == 0) {
      if (menuSelected) {
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
    } else if (menuSelect == 1 || menuSelect == 2) {
      const int MENU_DATA_X = 6 * 7 + 4;
      for (int i = 0; i < 5; i++) {
        ab.setCursor(MENU_DATA_X, SUBMENU_Y + 1 + 9 * i);
        ab.print(F("999. "));
        ab.print(F("ABCDEFGH"));
      }
      ab.setCursor(MENU_DATA_X + 12, SUBMENU_Y + 1 + 9 * 5);
      ab.print(F("999 / 999"));
    } else if (menuSelect == 3) {
      if (menuSelected) {
        if (romImportPos == 32) {
          ab.fillRect(28 - 5 - 2, 28 - 5 - 2, 2 + 5 + 6 * 12 + 4 + 2, 2 + 5 + 7 + 5 + 2, BLACK);
          ab.drawRect(28 - 5 - 1, 28 - 5 - 1, 1 + 5 + 6 * 12 + 4 + 1, 1 + 5 + 7 + 5 + 1, WHITE);
          ab.setCursor(28, 28);
          ab.print(F("ROM IMPORTED"));
        } else {
          ab.fillRect(22 - 5 - 2, 22 - 5 - 2, 2 + 5 + 6 * 14 + 4 + 2, 2 + 5 + 7 + 5 + 7 + 5 + 2, BLACK);
          ab.drawRect(22 - 5 - 1, 22 - 5 - 1, 1 + 5 + 6 * 14 + 4 + 1, 1 + 5 + 7 + 5 + 7 + 5 + 1, WHITE);
          if (Serial) {
            ab.setCursor(22 + 6, 22);
            ab.print(F("READING DATA"));
          } else {
            ab.setCursor(22, 22);
            ab.print(F("CONNECT SERIAL"));
          }
          ab.setCursor(22 + 6 * 3, 22 + 7 + 5);
          ab.print(F("B:CANCEL"));
        }
      }
    } else if (menuSelect == 4) {
      if (menuSelected) {
        if (romExportPos == 33) {
          ab.fillRect(28 - 5 - 2, 28 - 5 - 2, 2 + 5 + 6 * 12 + 4 + 2, 2 + 5 + 7 + 5 + 2, BLACK);
          ab.drawRect(28 - 5 - 1, 28 - 5 - 1, 1 + 5 + 6 * 12 + 4 + 1, 1 + 5 + 7 + 5 + 1, WHITE);
          ab.setCursor(28, 28);
          ab.print(F("ROM EXPORTED"));
        } else {
          ab.fillRect(22 - 5 - 2, 22 - 5 - 2, 2 + 5 + 6 * 14 + 4 + 2, 2 + 5 + 7 + 5 + 7 + 5 + 2, BLACK);
          ab.drawRect(22 - 5 - 1, 22 - 5 - 1, 1 + 5 + 6 * 14 + 4 + 1, 1 + 5 + 7 + 5 + 7 + 5 + 1, WHITE);
          if (Serial) {
            ab.setCursor(22 + 6, 22);
            ab.print(F("SENDING DATA"));
          } else {
            ab.setCursor(22, 22);
            ab.print(F("CONNECT SERIAL"));
          }
          ab.setCursor(22 + 6 * 3, 22 + 7 + 5);
          ab.print(F("B:CANCEL"));
        }
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
    if (menuSelected) {
      if (menuSelect == 0) {
        ab.fillRect(25 - 5 - 2, 22 - 5 - 2, 2 + 5 + 6 * 13 + 4 + 2, 2 + 5 + 7 + 5 + 7 + 5 + 2, BLACK);
        ab.drawRect(25 - 5 - 1, 22 - 5 - 1, 1 + 5 + 6 * 13 + 4 + 1, 1 + 5 + 7 + 5 + 7 + 5 + 1, WHITE);
        ab.setCursor(25, 22);
        if (menuConfirmSelect == 2) {
          ab.print(F("ERASE EEPROM"));
          if (eepromCommStatus == EEPROM_COMM_ERASING) {
            ab.drawRect(25, 22 + 7 + 5, 6 * 13 - 1, 7, WHITE);
            ab.fillRect(26, 22 + 7 + 5 + 1, (long)(6 * 13 - 1 - 2) * (eepromCommPointer - EEPROM_STORAGE_SPACE_START) / (EEPROM.length() - EEPROM_STORAGE_SPACE_START), 5, WHITE);
          } else {
            ab.setCursor(49, 22 + 7 + 5);
            ab.print(F("DONE!"));
          }
        } else {
          ab.print(F("ERASE EEPROM?"));
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
      } else {
        ab.fillRect(22 - 5 - 2, 21 - 5 - 2, 2 + 5 + 6 * 14 + 4 + 2, 2 + 5 + 7 + 5 + 7 + 5 + 7 + 5 + 2, BLACK);
        ab.drawRect(22 - 5 - 1, 21 - 5 - 1, 1 + 5 + 6 * 14 + 4 + 1, 1 + 5 + 7 + 5 + 7 + 5 + 7 + 5 + 1, WHITE);
        ab.setCursor(25, 21);
        if (menuSelect == 1) {
          ab.print(F("IMPORT EEPROM"));
        } else {
          ab.print(F("EXPORT EEPROM"));
        }
        if (eepromCommStatus == EEPROM_COMM_SUCCEEDED) {
          ab.setCursor(49, 21 + 7 + 5);
          ab.print(F("DONE!"));
        } else if (eepromCommStatus == EEPROM_COMM_FAILED) {
          ab.setCursor(46, 21 + 7 + 5);
          ab.print(F("FAILED"));
        } else if (eepromCommStatus == EEPROM_COMM_CANCELED) {
          ab.setCursor(40, 21 + 7 + 5);
          ab.print(F("CANCELED"));
        } else if (eepromCommStatus == EEPROM_COMM_RECV_CONNECT || eepromCommStatus == EEPROM_COMM_SEND_CONNECT) {
          ab.setCursor(22, 21 + 7 + 5);
          ab.print(F("CONNECT SERIAL"));
        } else {
          int graphPos = eepromCommPointer;
          if (graphPos > EEPROM.length()) graphPos = EEPROM.length();
          ab.drawRect(22, 21 + 7 + 5, 6 * 14 - 1, 7, WHITE);
          ab.fillRect(23, 21 + 7 + 5 + 1, (long)(6 * 14 - 1 - 2) * (graphPos - EEPROM_STORAGE_SPACE_START) / (EEPROM.length() - EEPROM_STORAGE_SPACE_START), 5, WHITE);
        }
        if (eepromCommStatus > EEPROM_COMM_NONE) {
          ab.setCursor(40, 21 + 7 + 5 + 7 + 5);
          ab.print(F("B:CANCEL"));
        } else {
          ab.setCursor(43, 21 + 7 + 5 + 7 + 5);
          ab.print(F("PRESS A"));
        }
      }
    }
  }

  menuRedraw = 0;
  prevEepromCommPointer = eepromCommPointer;
}

void setup() {
  ab.begin();
  ab.setTextSize(1);
  ab.setFrameRate(100);
  beep.begin();
  Serial.begin(9600);
}

void loop() {
  if (eepromCommStatus >= EEPROM_COMM_DISABLE_IDLE_BORDER) {
    switch (eepromCommStatus) {
      case EEPROM_COMM_ERASING:
        if (!eepromIsBusy()) {
          if (eepromCommPointer < EEPROM.length()) {
            eepromErase(eepromCommPointer);
            eepromCommPointer++;
          } else {
            eepromCommStatus = EEPROM_COMM_SUCCEEDED;
          }
        }
        break;
      case EEPROM_COMM_RECV_INIT:
        if (!Serial) {
          eepromCommStatus = EEPROM_COMM_FAILED;
        } else if (eepromCommCancelRequest) {
          Serial.write(CAN);
          Serial.write(CAN);
          eepromCommStatus = EEPROM_COMM_NONE;
          menuSelected = 0;
        } else {
          int inChar = Serial.read();
          if (inChar == SOH) {
            eepromCommStatus = EEPROM_COMM_RECV_READING;
            eepromCommRetryCount = 0;
            eepromCommCanCount = 0;
            eepromCommSeqId = 1;
            eepromCommBlockPos = 1;
            eepromCommCrc = 0;
            eepromCommBuffer[0] = inChar;
            eepromCommStartTime = millis();
          } else if (inChar == CAN) {
            eepromCommCanCount++;
            eepromCommStartTime = millis();
            if (eepromCommCanCount >= 2) {
              eepromCommStatus = EEPROM_COMM_CANCELED;
            }
          } else if (inChar >= 0) {
            eepromCommCanCount = 0;
          }
          if (millis() - eepromCommStartTime >= 3000) {
            eepromCommRetryCount++;
            if (eepromCommUseCrc && eepromCommRetryCount >= 4) {
              // switch to checksum after sending 4 'C's
              eepromCommRetryCount = 0;
              eepromCommUseCrc = 0;
            }
            if (eepromCommRetryCount >= 10) {
              eepromCommStatus = EEPROM_COMM_FAILED;
            } else {
              Serial.write(eepromCommUseCrc ? 'C' : NAK);
              eepromCommStartTime = millis();
            }
          }
        }
        break;
      case EEPROM_COMM_RECV_READING:
        if (eepromCommCancelRequest) {
          Serial.write(CAN);
          Serial.write(CAN);
          eepromCommStatus = EEPROM_COMM_NONE;
          menuSelected = 0;
        } else {
          bool sendNak = false, ignoreFrame = false;
          if (Serial.available() > 0) {
            while (Serial.available() > 0) {
              int inChar = Serial.read();
              if (eepromCommBlockPos == 0 && inChar != SOH) {
                if (inChar == CAN) {
                  eepromCommCanCount++;
                  if (eepromCommCanCount >= 2) {
                    eepromCommStatus = EEPROM_COMM_CANCELED;
                    break;
                  }
                } else if (inChar == EOT) {
                  Serial.write(ACK);
                  eepromCommStatus = EEPROM_COMM_SUCCEEDED;
                  break;
                }
              } else {
                eepromCommCanCount = 0;
                if (3 <= eepromCommBlockPos && eepromCommBlockPos < 3 + 128) {
                  if (eepromCommUseCrc) {
                    eepromCommCrc = crcUpdate(inChar, eepromCommCrc);
                  } else {
                    eepromCommCrc += inChar;
                  }
                }
                eepromCommBuffer[eepromCommBlockPos++] = inChar;
                if (eepromCommBlockPos >= 1 + 2 + 128 + (eepromCommUseCrc ? 2 : 1)) {
                  if (eepromCommBuffer[2] != (eepromCommBuffer[1] ^ 0xff)) {
                    // broken frame ID
                    sendNak = true;
                  } else if (eepromCommBuffer[1] == ((eepromCommSeqId - 1) & 0xff)) {
                    // duplicate frame -> ignore
                    sendNak = true;
                    ignoreFrame = true;
                  } else if (eepromCommBuffer[1] != eepromCommSeqId) {
                    // invalid frame -> abort
                    Serial.write(CAN);
                    Serial.write(CAN);
                    eepromCommStatus = EEPROM_COMM_FAILED;
                  } else if (eepromCommUseCrc &&
                             ((eepromCommBuffer[3 + 128] != (eepromCommCrc >> 8)) || (eepromCommBuffer[3 + 128 + 1] != (eepromCommCrc & 0xff)))) {
                    // CRC mismatch
                    sendNak = true;
                  } else if (!eepromCommUseCrc && eepromCommBuffer[3 + 128] != (eepromCommCrc & 0xff)) {
                    // checksum mismatch
                    sendNak = true;
                  } else {
                    // valid frame
                    if (eepromCommPointer >= EEPROM.length()) {
                      // too much data
                      Serial.write(CAN);
                      Serial.write(CAN);
                      eepromCommStatus = EEPROM_COMM_SUCCEEDED;
                    } else {
                      // begin to write
                      eepromCommBlockPos = 0;
                      eepromCommStatus = EEPROM_COMM_RECV_WRITING;
                    }
                  }
                  break;
                }
              }
            }
            eepromCommStartTime = millis();
          }
          if (sendNak || millis() - eepromCommStartTime >= 10000) {
            if (eepromCommRetryCount >= 10) {
              // give up after sending 10 NAKs
              Serial.write(CAN);
              Serial.write(CAN);
              eepromCommStatus = EEPROM_COMM_FAILED;
            } else {
              Serial.write(ignoreFrame ? ACK : NAK);
              eepromCommBlockPos = 0;
              eepromCommCrc = 0;
              eepromCommRetryCount++;
              eepromCommStartTime = millis();
            }
          }
        }
        break;
      case EEPROM_COMM_RECV_WRITING:
        if (eepromCommCancelRequest) {
          Serial.write(CAN);
          Serial.write(CAN);
          eepromCommStatus = EEPROM_COMM_NONE;
          menuSelected = 0;
        } else if (!eepromIsBusy()) {
          if (eepromCommBlockPos < 128) {
            if (eepromCommPointer < EEPROM.length()) {
              EEPROM.update(eepromCommPointer, eepromCommBuffer[3 + eepromCommBlockPos]);
              eepromCommPointer++;
              eepromCommBlockPos++;
            } else {
              eepromCommBlockPos = 128;
            }
          } else {
            Serial.write(ACK);
            eepromCommSeqId++;
            eepromCommBlockPos = 0;
            eepromCommCrc = 0;
            eepromCommStartTime = millis();
            eepromCommRetryCount = 0;
            eepromCommStatus = EEPROM_COMM_RECV_READING;
          }
        }
        break;
      case EEPROM_COMM_SEND_INIT:
        if (eepromCommCancelRequest) {
          Serial.write(CAN);
          Serial.write(CAN);
          eepromCommStatus = EEPROM_COMM_NONE;
          menuSelected = 0;
        } else {
          int inChar = Serial.read();
          if (inChar == 'C' || inChar == NAK) {
            eepromCommStatus = EEPROM_COMM_SEND_SENDING;
            eepromCommUseCrc = (inChar == 'C' ? 1 : 0);
            eepromCommRetryCount = 0;
            eepromCommCrc = 0;
            eepromCommSeqId = 1;
            eepromCommBlockPos = 0;
            eepromCommCanCount = 0;
          } else if (inChar == CAN) {
            eepromCommCanCount++;
            if (eepromCommCanCount >= 2) {
              eepromCommStatus = EEPROM_COMM_CANCELED;
            }
          } else if (inChar >= 0) {
            eepromCommCanCount = 0;
          }
        }
        break;
      case EEPROM_COMM_SEND_SENDING:
        while (Serial.availableForWrite() > 0) {
          if (eepromCommBlockPos == 0) {
            Serial.write(SOH);
          } else if (eepromCommBlockPos == 1) {
            Serial.write(eepromCommSeqId);
          } else if (eepromCommBlockPos == 2) {
            Serial.write(eepromCommSeqId ^ 0xff);
          } else if (eepromCommBlockPos < 3 + 128) {
            int outChar;
            if (eepromCommPointer + eepromCommBlockPos - 3 < EEPROM.length()) {
              outChar = EEPROM.read(eepromCommPointer + eepromCommBlockPos - 3);
            } else {
              outChar = 0x1a;
            }
            Serial.write(outChar);
            if (eepromCommUseCrc) {
              eepromCommCrc = crcUpdate(outChar, eepromCommCrc);
            } else {
              eepromCommCrc += outChar;
            }
          } else if (eepromCommBlockPos == 3 + 128) {
            Serial.write((eepromCommCrc >> (eepromCommUseCrc ? 8 : 0)) & 0xff);
          } else if(eepromCommBlockPos == 3 + 128 + 1) {
            if(eepromCommUseCrc) {
              Serial.write(eepromCommCrc & 0xff);
            } else {
              eepromCommStartTime = millis();
              eepromCommStatus = EEPROM_COMM_SEND_WAITING_ACK;
              break;
            }
          } else {
            eepromCommStartTime = millis();
            eepromCommStatus = EEPROM_COMM_SEND_WAITING_ACK;
            break;
          }
          eepromCommBlockPos++;
        }
        break;
      case EEPROM_COMM_SEND_WAITING_ACK:
        if (eepromCommCancelRequest) {
          Serial.write(CAN);
          Serial.write(CAN);
          eepromCommStatus = EEPROM_COMM_NONE;
          menuSelected = 0;
        } else {
          int inChar = Serial.read();
          if (inChar == ACK) {
            if (eepromCommPointer < EEPROM.length()) {
              eepromCommPointer += 128;
              if (eepromCommPointer < EEPROM.length()) {
                eepromCommStatus = EEPROM_COMM_SEND_SENDING;
                eepromCommCrc = 0;
                eepromCommSeqId++;
                eepromCommBlockPos = 0;
              } else{
                eepromCommStartTime = millis();
                Serial.write(EOT);
              }
              eepromCommRetryCount = 0;
              eepromCommCanCount = 0;
            } else {
              eepromCommStatus = EEPROM_COMM_SUCCEEDED;
            }
          } else if (inChar == NAK || (inChar < 0 && (millis() - eepromCommStartTime) >= 10000)) {
            eepromCommCanCount = 0;
            eepromCommRetryCount++;
            if (eepromCommRetryCount < 10) {
              if (eepromCommPointer < EEPROM.length()) {
                eepromCommStatus = EEPROM_COMM_SEND_SENDING;
                eepromCommCrc = 0;
                eepromCommBlockPos = 0;
              } else {
                eepromCommStartTime = millis();
                Serial.write(EOT);
              }
            } else {
              // too many retries -> abort
              Serial.write(CAN);
              Serial.write(CAN);
              eepromCommStatus = EEPROM_COMM_FAILED;
            }
          } else if (inChar == CAN) {
            eepromCommCanCount++;
            if (eepromCommCanCount >= 2) {
              eepromCommStatus = EEPROM_COMM_CANCELED;
            }
          } else if (inChar >= 0) {
            eepromCommCanCount = 0;
          }
        }
        break;
    }
  }
  if (!ab.nextFrame()) {
    if (eepromCommStatus < EEPROM_COMM_DISABLE_IDLE_BORDER) {
      ab.idle();
    }
    return;
  }
  beep.timer();
  if (ab.pressed(UP_BUTTON | DOWN_BUTTON) && !(screen == SCREEN_MAIN && !editing && !inputIsAlternate)) {
    udResetCount++;
    if (udResetCount >= UD_RESET_WAIT) ab.exitToBootloader();
  } else {
    udResetCount = 0;
  }
  if (eepromCommStatus >= EEPROM_COMM_DISABLE_IDLE_BORDER) {
    menuFramePrescalerWhileComm++;
    if (menuFramePrescalerWhileComm < 5) {
      return;
    } else {
      menuFramePrescalerWhileComm = 0;
    }
  } else {
    menuFramePrescalerWhileComm = 0;
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
