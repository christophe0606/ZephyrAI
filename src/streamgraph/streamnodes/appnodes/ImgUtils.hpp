#pragma once

#include <cstdint>



extern void fillRectangle(uint16_t *renderingFrame, int x, int y, int width, int height, uint16_t color);

extern void strokeRectangle(uint16_t *renderingFrame, int x, int y, int width, int height, uint16_t color);

extern void displayImage(uint16_t *renderingFrame, const uint16_t *buf,  int width, int height);