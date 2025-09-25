#ifndef MATRIX32_H
#define MATRIX32_H

#include <cstdint>

struct RGB
{
    uint8_t r, g, b;
};
inline constexpr RGB rgb(uint8_t r, uint8_t g, uint8_t b) { return RGB{r, g, b}; }

struct Matrix32
{
  virtual void begin() = 0;
  virtual void clear() = 0;
  virtual void set(int x, int y, RGB c) = 0;
  virtual void show() = 0;
  virtual ~Matrix32() = default;
};

#endif
