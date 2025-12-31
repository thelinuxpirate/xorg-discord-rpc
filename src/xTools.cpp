#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string>
#include <cstring>

using namespace std;

string getWindowManagerName() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return "unknown";

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    // SUPPORTING_WM_CHECK needed for some WMs like 'dwm'
    Atom netSupporting =
        XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", True);
    Atom netWmName =
        XInternAtom(dpy, "_NET_WM_NAME", True);
    Atom utf8 =
        XInternAtom(dpy, "UTF8_STRING", True);

    if (!netSupporting || !netWmName || !utf8) {
        XCloseDisplay(dpy);
        return "unknown";
    }

    Window wmcheck = None;
    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytes;
    unsigned char* prop = nullptr;

    if (XGetWindowProperty(
            dpy, root, netSupporting,
            0, 1, False, XA_WINDOW,
            &actualType, &actualFormat,
            &nitems, &bytes, &prop
        ) != Success || !prop)
    {
        XCloseDisplay(dpy);
        return "unknown";
    }

    wmcheck = *(Window*)prop;
    XFree(prop);

    if (!wmcheck) {
        XCloseDisplay(dpy);
        return "unknown";
    }

    if (XGetWindowProperty(
            dpy, wmcheck, netWmName,
            0, 1024, False, utf8,
            &actualType, &actualFormat,
            &nitems, &bytes, &prop
        ) == Success &&
        prop && actualFormat == 8)
    {
        string result((char*)prop, nitems);
        XFree(prop);
        XCloseDisplay(dpy);
        return result;
    }

    XCloseDisplay(dpy);
    return "unknown";
}

// returns name of Window like: Discord #general, xTools.cpp – Doom Emacs
string getWindowTitle() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return "unknown";

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    Atom netActive =
        XInternAtom(dpy, "_NET_ACTIVE_WINDOW", True);
    Atom netWmName =
        XInternAtom(dpy, "_NET_WM_NAME", True);
    Atom utf8 =
        XInternAtom(dpy, "UTF8_STRING", True);
    Atom wmName =
        XInternAtom(dpy, "WM_NAME", False);

    if (!netActive) {
        XCloseDisplay(dpy);
        return "unknown";
    }

    Window win = None;
    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytes;
    unsigned char* prop = nullptr;

    // get active window
    if (XGetWindowProperty(
            dpy, root, netActive,
            0, 1, False, XA_WINDOW,
            &actualType, &actualFormat,
            &nitems, &bytes, &prop
        ) != Success || !prop)
    {
        XCloseDisplay(dpy);
        return "unknown";
    }

    win = *(Window*)prop;
    XFree(prop);

    if (!win) {
        XCloseDisplay(dpy);
        return "unknown";
    }

    // _NET_WM_NAME (UTF-8)
    if (netWmName && utf8 &&
        XGetWindowProperty(
            dpy, win, netWmName,
            0, 1024, False, utf8,
            &actualType, &actualFormat,
            &nitems, &bytes, &prop
        ) == Success &&
        prop && actualFormat == 8)
    {
        string title((char*)prop, nitems);
        XFree(prop);
        XCloseDisplay(dpy);
        return title;
    }

    // fallback to WM_NAME (older window managers)
    if (XGetWindowProperty(
            dpy, win, wmName,
            0, 1024, False, AnyPropertyType,
            &actualType, &actualFormat,
            &nitems, &bytes, &prop
        ) == Success && prop)
    {
        string title((char*)prop);
        XFree(prop);
        XCloseDisplay(dpy);
        return title;
    }

    XCloseDisplay(dpy);
    return "unknown";
}

// title of Window like - Emacs, Alacritty
string getWindowClass() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return "unknown";

    Window root = DefaultRootWindow(dpy);

    Atom netActiveWindow = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", True);
    if (!netActiveWindow) { XCloseDisplay(dpy); return "unknown"; }

    Window activeWin = None;
    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytes;
    unsigned char* prop = nullptr;

    if (XGetWindowProperty(dpy, root, netActiveWindow, 0, 1, False, XA_WINDOW,
                           &actualType, &actualFormat, &nitems, &bytes, &prop) != Success || !prop) {
        XCloseDisplay(dpy);
        return "unknown";
    }

    activeWin = *(Window*)prop;
    XFree(prop);

    if (!activeWin) { XCloseDisplay(dpy); return "unknown"; }

    // WM_CLASS
    Atom wmClass = XInternAtom(dpy, "WM_CLASS", False);
    if (!wmClass) { XCloseDisplay(dpy); return "unknown"; }

    if (XGetWindowProperty(dpy, activeWin, wmClass, 0, 1024, False, AnyPropertyType,
                           &actualType, &actualFormat, &nitems, &bytes, &prop) == Success && prop) {
        // WM_CLASS is two null-terminated strings: instance\0class\0
        char* instance = (char*)prop;
        char* className = instance + strlen(instance) + 1;

        // Prefer instance if it exists and is non-empty
        string result;
        if (instance && *instance) {
            result = instance;
        } else if (className && *className) {
            result = className;
        } else {
            result = "unknown";
        }

        XFree(prop);
        XCloseDisplay(dpy);
        return result;
    }

    XCloseDisplay(dpy);
    return "unknown";
}
