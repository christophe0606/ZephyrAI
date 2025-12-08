#pragma once

#include <cstdint>

extern "C" {
#include "dbuf_display/display.h"
}

void fillRectangle(uint16_t *renderingFrame, int x, int y, int width, int height, uint16_t color)
{
	if (x < 0) {
		width += x;
		x = 0;
	}
	if (y < 0) {
		height += y;
		y = 0;
	}
	if (width + x > DISPLAY_WIDTH) {
		width = DISPLAY_WIDTH - x;
	}
	if (height + y > DISPLAY_HEIGHT) {
		height = DISPLAY_HEIGHT - y;
	}
	if (width <= 0) {
		return;
	}
	if (height <= 0) {
		return;
	}
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int px = x + j;
			int py = y + i;
			renderingFrame[py * DISPLAY_WIDTH + px] = color;
		}
	}
}

void strokeRectangle(uint16_t *renderingFrame, int x, int y, int width, int height, uint16_t color)
{
	bool drawTop = true;
	bool drawBottom = true;
	bool drawLeft = true;
	bool drawRight = true;
	if (x < 0) {
		width += x;
		x = 0;
		drawLeft = false;
	}
	if (y < 0) {
		height += y;
		y = 0;
		drawTop = false;
	}
	if (width + x > DISPLAY_WIDTH) {
		width = DISPLAY_WIDTH - x;
		drawRight = false;
	}
	if (height + y > DISPLAY_HEIGHT) {
		height = DISPLAY_HEIGHT - y;
		drawBottom = false;
	}

	if (width <= 0) {
		return;
	}
	if (height <= 0) {
		return;
	}
	if (drawTop) {
		for (int j = 0; j < width; j++) {
			int px = x + j;
			int py = y;
			renderingFrame[py * DISPLAY_WIDTH + px] = color;
		}
	}
	if (drawBottom) {
		for (int j = 0; j < width; j++) {
			int px = x + j;
			int py = y + height - 1;
			renderingFrame[py * DISPLAY_WIDTH + px] = color;
		}
	}
	if (drawLeft) {
		for (int i = 0; i < height; i++) {
			int px = x;
			int py = y + i;
			renderingFrame[py * DISPLAY_WIDTH + px] = color;
		}
	}
	if (drawRight) {
		for (int i = 0; i < height; i++) {
			int px = x + width - 1;
			int py = y + i;
			renderingFrame[py * DISPLAY_WIDTH + px] = color;
		}
	}
}

