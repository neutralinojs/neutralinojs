#ifndef OS_H
#define OS_H

namespace os {
    string execCommand(json input);
    string getEnvar(json input);
    string dialogOpen(json input);
    string dialogSave(json input);
    string showNotification(json input);
    string showMessageBox(json input);
}

#endif
