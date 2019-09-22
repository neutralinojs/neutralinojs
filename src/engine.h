#ifndef ENGINE_H
#define ENGINE_H

#include <serge/webview.h>

namespace neut
{
    class Engine
    {
    private:
        struct webview _webview = {};

    public:
        Engine();
        ~Engine();
    };
} // namespace neut
#endif