#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string>
#include <cstring>

using namespace std;

/*
 * returns WMs TITLE (e.g. "dwm", "herbstluftwm", "xmonad")
 * Caller owns Display*
 */
string getWindowManagerName(Display* dpy) {
    if (!dpy) {
        return "unknown";
    }

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    Atom netSupporting = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", True);
    Atom netWmName = XInternAtom(dpy, "_NET_WM_NAME", True);
    Atom utf8 = XInternAtom(dpy, "UTF8_STRING", True);

    if (!netSupporting || !netWmName || !utf8) {
        return "unknown";
    }

    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytes;
    unsigned char* prop = nullptr;

    // Get the WM check window
    if (XGetWindowProperty(
            dpy, root, netSupporting,
            0, 1, False, XA_WINDOW,
            &actualType, &actualFormat,
            &nitems, &bytes, &prop
        ) != Success || !prop)
    {
        return "unknown";
    }

    Window wmcheck = *(Window*)prop;
    XFree(prop);

    if (!wmcheck) {
        return "unknown";
    }

    // Get WM name from that window
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
        return result;
    }

    if (prop) {
        XFree(prop);
    }

    return "unknown";
}

/*
 * Returns the active window title
 */
string getWindowTitle(Display* dpy) {
    if (!dpy) {
        return "unknown";
    }

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    Atom netActive = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", True);
    Atom netWmName = XInternAtom(dpy, "_NET_WM_NAME", True);
    Atom utf8 = XInternAtom(dpy, "UTF8_STRING", True);
    Atom wmName = XInternAtom(dpy, "WM_NAME", False);

    if (!netActive) {
        return "unknown";
    }

    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytes;
    unsigned char* prop = nullptr;

    // Get active window
    if (XGetWindowProperty(
            dpy, root, netActive,
            0, 1, False, XA_WINDOW,
            &actualType, &actualFormat,
            &nitems, &bytes, &prop
        ) != Success || !prop)
    {
        return "unknown";
    }

    Window win = *(Window*)prop;
    XFree(prop);

    if (!win) {
        return "unknown";
    }

    // UTF-8 title
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
        return title;
    }

    if (prop) {
        XFree(prop);
    }

    // Fallback to WM_NAME
    if (XGetWindowProperty(
            dpy, win, wmName,
            0, 1024, False, AnyPropertyType,
            &actualType, &actualFormat,
            &nitems, &bytes, &prop
        ) == Success && prop)
    {
        string title((char*)prop);
        XFree(prop);
        return title;
    }

    if (prop) {
        XFree(prop);
    }

    return "unknown";
}

/*
 * returns WM_CLASS of the active window
 * instance name -> class name
 */
string getWindowClass(Display* dpy) {
    if (!dpy) return "unknown";

    Window root = DefaultRootWindow(dpy);

    Atom netActiveWindow = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", True);
    if (!netActiveWindow) {
        return "unknown";
    }

    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytes;
    unsigned char* prop = nullptr;

    if (XGetWindowProperty(
            dpy, root, netActiveWindow,
            0, 1, False, XA_WINDOW,
            &actualType, &actualFormat,
            &nitems, &bytes, &prop
        ) != Success || !prop)
    {
        return "unknown";
    }

    Window activeWin = *(Window*)prop;
    XFree(prop);

    if (!activeWin) {
        return "unknown";
    }

    Atom wmClass = XInternAtom(dpy, "WM_CLASS", False);
    if (!wmClass) {
        return "unknown";
    }

    if (XGetWindowProperty(
            dpy, activeWin, wmClass,
            0, 1024, False, AnyPropertyType,
            &actualType, &actualFormat,
            &nitems, &bytes, &prop
        ) == Success && prop)
    {
        char* instance = (char*)prop;
        char* className = instance + strlen(instance) + 1;

        string result;
        if (instance && *instance)
            result = instance;
        else if (className && *className)
            result = className;
        else
            result = "unknown";

        XFree(prop);
        return result;
    }

    if (prop) {
        XFree(prop);
    }

    return "unknown";
}
