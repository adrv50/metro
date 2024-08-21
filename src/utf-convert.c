//
//  based libctru.
//

#include <string.h>
#include <stdlib.h>
#include "types.h"

i64 decode_utf8(u32 *out, const u8 *in) {
  uint8_t code1, code2, code3, code4;

  code1 = *in++;

  if (code1 < 0x80) {
    /* 1-byte sequence */
    *out = code1;
    return 1;
  } else if (code1 < 0xC2) {
    return -1;
  } else if (code1 < 0xE0) {
    /* 2-byte sequence */
    code2 = *in++;
    if ((code2 & 0xC0) != 0x80) {
      return -1;
    }

    *out = (code1 << 6) + code2 - 0x3080;
    return 2;
  } else if (code1 < 0xF0) {
    /* 3-byte sequence */
    code2 = *in++;
    if ((code2 & 0xC0) != 0x80) {
      return -1;
    }
    if (code1 == 0xE0 && code2 < 0xA0) {
      return -1;
    }

    code3 = *in++;
    if ((code3 & 0xC0) != 0x80) {
      return -1;
    }

    *out = (code1 << 12) + (code2 << 6) + code3 - 0xE2080;
    return 3;
  } else if (code1 < 0xF5) {
    /* 4-byte sequence */
    code2 = *in++;
    if ((code2 & 0xC0) != 0x80) {
      return -1;
    }
    if (code1 == 0xF0 && code2 < 0x90) {
      return -1;
    }
    if (code1 == 0xF4 && code2 >= 0x90) {
      return -1;
    }

    code3 = *in++;
    if ((code3 & 0xC0) != 0x80) {
      return -1;
    }

    code4 = *in++;
    if ((code4 & 0xC0) != 0x80) {
      return -1;
    }

    *out = (code1 << 18) + (code2 << 12) + (code3 << 6) + code4 -
           0x3C82080;
    return 4;
  }

  return -1;
}