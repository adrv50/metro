//
//  based libctru.
//

#include <string.h>
#include <stdlib.h>
#include "types.h"

size_t decode_utf8(u32* out, const u8* in);
size_t encode_utf16(u16* out, u32 in);

size_t utf8_to_utf16(u16* out, const u8* in, size_t len);

size_t decode_utf8(u32* out, const u8* in) {
  uint8_t code[4];

  code[0] = *in++;

  if (code[0] < 0x80) {
    /* 1-byte sequence */
    *out = code[0];
    return 1;
  }
  else if (code[0] < 0xC2) {
    return -1;
  }
  else if (code[0] < 0xE0) {
    /* 2-byte sequence */
    code[1] = *in++;
    if ((code[1] & 0xC0) != 0x80) {
      return -1;
    }

    *out = (code[0] << 6) + code[1] - 0x3080;
    return 2;
  }
  else if (code[0] < 0xF0) {
    /* 3-byte sequence */
    code[1] = *in++;
    if ((code[1] & 0xC0) != 0x80) {
      return -1;
    }
    if (code[0] == 0xE0 && code[1] < 0xA0) {
      return -1;
    }

    code[2] = *in++;
    if ((code[2] & 0xC0) != 0x80) {
      return -1;
    }

    *out = (code[0] << 12) + (code[1] << 6) + code[2] - 0xE2080;
    return 3;
  }
  else if (code[0] < 0xF5) {
    /* 4-byte sequence */
    code[1] = *in++;
    if ((code[1] & 0xC0) != 0x80) {
      return -1;
    }
    if (code[0] == 0xF0 && code[1] < 0x90) {
      return -1;
    }
    if (code[0] == 0xF4 && code[1] >= 0x90) {
      return -1;
    }

    code[2] = *in++;
    if ((code[2] & 0xC0) != 0x80) {
      return -1;
    }

    code[3] = *in++;
    if ((code[3] & 0xC0) != 0x80) {
      return -1;
    }

    *out = (code[0] << 18) + (code[1] << 12) + (code[2] << 6) + code[3] -
           0x3C82080;

    return 4;
  }

  return -1;
}

size_t encode_utf16(u16* out, u32 in) {
  if (in < 0x10000) {
    if (out != NULL)
      *out++ = in;

    return 1;
  }
  else if (in < 0x110000) {
    if (out != NULL) {
      *out++ = (in >> 10) + 0xD7C0;
      *out++ = (in & 0x3FF) + 0xDC00;
    }

    return 2;
  }

  return -1;
}

size_t utf8_to_utf16(u16* out, const u8* in, size_t len) {
  size_t rc = 0;
  size_t units;
  u32 code;
  u16 encoded[2];

  do {
    units = decode_utf8(&code, in);

    if (units == -1)
      return -1;

    if (code > 0) {
      in += units;

      units = encode_utf16(encoded, code);

      if (units == -1)
        return -1;

      if (out != NULL) {
        if (rc + units <= len) {
          *out++ = encoded[0];
          if (units > 1)
            *out++ = encoded[1];
        }
      }

      if (SIZE_MAX - units >= rc)
        rc += units;
      else
        return -1;
    }
  } while (code > 0);

  return rc;
}