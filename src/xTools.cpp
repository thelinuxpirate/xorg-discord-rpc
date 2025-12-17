// all Xorg-related functions go here
// TODO: add Wayland support (make wayland files?)

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string>

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
