#include "webv.h"

#define WEBVIEW_IMPLEMENTATION
#include "webview.h"

extern "C"
{
	void web_view(const char *title, const char *url, int width, int height, int fullscreen)
	{
		struct webview webview;
		memset(&webview, 0, sizeof(webview));
		webview.title = title;
		webview.url = url;
		webview.width = width;
		webview.height = height;
		webview.resizable = 1;
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