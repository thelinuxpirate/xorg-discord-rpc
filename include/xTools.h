#ifndef XTOOLS_H
#define XTOOLS_H

#include <X11/Xlib.h>
#include <string>

std::string getWindowManagerName(Display* dpy);
std::string getWindowTitle(Display* dpy);
std::string getWindowClass(Display* dpy);

#endif 
