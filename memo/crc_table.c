#include <stdio.h>
#include <inttypes.h>

/* reference: https://web.mit.edu/6.115/www/amulet/xmodem.htm */

int main(void) {
	int i;
	for (i = 0; i < 256; i++) {
		uint16_t crc = i << 8;
		int j;
		for (j = 0; j < 8; j++) {
			uint16_t x = crc & UINT16_C(0x8000) ? UINT16_C(0x1021) : 0;
			crc = (crc << 1) ^ x;
		}
		printf("0x%04" PRIx16 "u", crc);
		if (i + 1 < 256) putchar(',');
		putchar((i + 1) % 16 == 0 ? '\n' : ' ');
	}
	return 0;
}
