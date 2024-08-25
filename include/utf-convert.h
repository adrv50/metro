#pragma once

#include "types.h"

size_t decode_utf8(u32* out, const u8* in);
size_t encode_utf8(u8* out, u32 in);

size_t encode_utf16(u16* out, u32 in);
size_t utf8_to_utf16(u16* out, const u8* in, size_t len);

size_t utf8_to_utf16(u16* out, const u8* in, size_t len);
size_t utf16_to_utf8(u8* out, const u16* in, size_t len);

size_t get_count_of_utf8(const char* str);
