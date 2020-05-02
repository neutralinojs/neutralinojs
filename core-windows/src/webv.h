#pragma once

extern "C" {
	void web_view(const char* title, const char* url, int width, int height, int fullscreen, bool always_on_top, bool borderless, const char* iconfile);
}