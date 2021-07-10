#ifndef OS_H
#define OS_H

namespace os {
    struct MessageBoxOptions {
        string type;
        string title;
        string content;
    };
    
    struct MessageBoxResult {
        bool hasError = false;
        string error;
        bool yesButtonClicked = false;
    };
    string execCommand(string command);
    MessageBoxResult showMessageBox(MessageBoxOptions options);

namespace controllers {
    json execCommand(json input);
    json getEnvar(json input);
    json dialogOpen(json input);
    json dialogSave(json input);
    json showNotification(json input);
    json showMessageBox(json input);
    json setTray(json input);
} // namespace controllers
} // namespace os

#endif
