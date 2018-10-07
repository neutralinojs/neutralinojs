#include "webv.h"

#define WEBVIEW_IMPLEMENTATION
#include "webview.h"

extern "C" {
	void web_view(const char * title, const char * url)
	{
		webview(title, url , 800, 600, 1);
	}
}