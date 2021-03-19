#include "webview_c.h"

#define WEBVIEW_IMPLEMENTATION
#include "webview/webview.h"

extern "C"
{
	void web_view(const char *title, const char *url, int width, int height, int fullscreen, bool alwaysOnTop,
				bool borderless, bool maximize, void *icon)
	{
		struct webview webview;
		memset(&webview, 0, sizeof(webview));
		webview.title = title;
		webview.url = url;
		webview.width = width;
		webview.height = height;
		webview.resizable = true;
		webview.alwaysOnTop = alwaysOnTop;
		webview.borderless = borderless;
		webview.maximize = maximize;
    	webview.icon = icon;
		int r = webview_init(&webview);
		webview_set_fullscreen(&webview, fullscreen);
		if (r != 0)
		{
			return;
		}
		while (webview_loop(&webview, 1) == 0)
		{
		}
		webview_exit(&webview);
	}
}