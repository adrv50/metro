#pragma once

#define swap(a, b)                                                   \
  ({                                                                 \
    __auto_type _temp = a;                                           \
    a = b;                                                           \
    b = _temp;                                                       \
  })