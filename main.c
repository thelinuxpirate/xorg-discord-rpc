#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

int main() {
    Display *display;
    Window root, active_window;
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *data = NULL;
    char *window_title = NULL;

    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Unable to open X display\n");
        return EXIT_FAILURE;
    }

    root = DefaultRootWindow(display);

    Atom net_active_window = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    XGetWindowProperty(display, root, net_active_window, 0, 1, False, XA_WINDOW,
                       &actual_type, &actual_format, &nitems, &bytes_after, &data);

    if (nitems == 1) {
        active_window = *(Window *)data;
        printf("Active Window ID: 0x%lx\n", active_window);

        Atom net_wm_name = XInternAtom(display, "_NET_WM_NAME", False);
        XGetWindowProperty(display, active_window, net_wm_name, 0, 1024, False, XA_STRING,
                           &actual_type, &actual_format, &nitems, &bytes_after, &data);

        if (nitems > 0) {
            window_title = strdup((char *)data);
            printf("Active Window Title: %s\n", window_title);
            free(window_title);
        } else {
            printf("No title found for the active window.\n");
        }
    } else {
        printf("No active window found.\n");
    }

    if (data) XFree(data);
    XCloseDisplay(display);

    return EXIT_SUCCESS;
}
