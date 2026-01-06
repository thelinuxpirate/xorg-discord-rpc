#ifndef XTOOLS_H
#define XTOOLS_H

#include <X11/Xlib.h>
#include <string>

struct WindowClass {
    std::string instance;
    std::string className;
};

std::string getWindowManagerName(Display* dpy);
std::string getWindowTitle(Display* dpy);
WindowClass getWindowClass(Display* dpy);

#endif 
