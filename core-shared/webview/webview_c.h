#pragma once

extern "C" {
	void web_view(const char* title, const char* url, int width, int height, int fullScreen, bool alwaysOnTop, bool borderless, bool maximize, void *icon);
}