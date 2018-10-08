#include "webv.h"

#define WEBVIEW_IMPLEMENTATION
#include "webview.h"

extern "C" {
	void web_view(const char * title, const char * url, int width, int height)
	{
		webview(title, url , width, height, 1);
	}
}