#pragma once

extern "C" {
	#define CINTERFACE
	#include <windows.h>
	void web_view(const char* title, const char* url, int width, int height, int fullscreen, bool always_on_top, bool borderless, bool maximize, HICON icon);
}