#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>

#define BROKEN          "NOT_FOUND"

long unsigned int *
XPingWindow(Display *display, Window win, int wait_time_seconds)
{
    XEvent ev;
    XEvent pingback;
    Atom *winproto;
    int winprotocount = 0;
    unsigned long *retdata;
    unsigned long curtime;
    int i;
    retdata = malloc(sizeof(long ) * 5); if(!retdata) return NULL;
    retdata[0] = 0; /* window id returned from ping back (Window)*/
    retdata[1] = 0; /* time alloated for ping back */
    retdata[2] = 0; /* EMPTY */
    retdata[3] = 0; /* EMPTY */
    retdata[4] = 1; /* 0 PING SUCCESS, 1 PING FAILED, 2 PING NOT SET */ /* INITIALY SET TO 1 */
    if (XGetWMProtocols(display, win, &winproto, &winprotocount))
    {
        for (i = 0; i < winprotocount; i++)
        {
            if (winproto[i] == XInternAtom(display, "_NET_WM_PING", False)) 
            {
                i = -1;
                break;
            }
        }
        /* i only equals -1 if found */
        if(i != -1)
        {
            retdata[0] = win;
            retdata[4] = 2;
            return retdata;
        }
    }
    ev.xclient.type = ClientMessage;
    ev.xclient.window = win;
    ev.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", False);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = XInternAtom(display, "_NET_WM_PING", False);
    ev.xclient.data.l[1] = CurrentTime;
    ev.xclient.data.l[2] = win;
    XSendEvent(display, win, False, SubstructureNotifyMask | SubstructureRedirectMask, &ev);
    curtime = time(NULL) * 1000;
    do
    {
        if  (pingback.xclient.type == ClientMessage
             && pingback.xclient.message_type == XInternAtom(display, "WM_PROTOCOLS", False)
             && pingback.xclient.format == 32 
             && pingback.xclient.data.l[0] == XInternAtom(display, "_NET_WM_PING", False)
             && pingback.xclient.data.l[2] == win
             && pingback.xclient.window == DefaultRootWindow(display))
        {
            retdata[0] = pingback.xclient.data.l[2];
            retdata[1] = (time(NULL) * 1000) - curtime;
            retdata[4] = 0;
            return retdata;
        }
        if ((time(NULL) * 1000 - curtime) > wait_time_seconds) break;
    } while(!XCheckMaskEvent(display, ClientMessage, &pingback));
    /* fall through */
    return retdata;
}

int
XGetTextProp(Display *display, Window w, Atom atom, char *text, unsigned int size)
{
    char **list = NULL;
    int n;
    XTextProperty name;

    if (!text || size == 0)
        return 0;
    text[0] = '\0';
    if (!XGetTextProperty(display, w, &name, atom) || !name.nitems)
        return 0;
    if (name.encoding == XA_STRING)
        strncpy(text, (char *)name.value, size - 1);
    else if (XmbTextPropertyToTextList(display, &name, &list, &n) >= Success && n > 0 && *list)
    {
        strncpy(text, *list, size - 1);
        XFreeStringList(list);
    }
    text[size - 1] = '\0';
    XFree(name.value);
    return 1;
}

/* MUST BE FREED BY CALLING XFree() */
char *
XGetWindowName(Display *display, Window win)
{
    return BROKEN;
    char *ret;
    Atom winname = XInternAtom(display, "_NET_WM_NAME", False);
    if (!XGetTextProp(display, win, winname, ret, sizeof ret))
        XGetTextProp(display, win, XA_WM_NAME, ret, sizeof ret);
    if (ret[0] == '\0' || !ret) /* hack to mark broken clients */
        ret = BROKEN;
    return ret;
}

pid_t
XGetPid(Display *display, Window win)
{
    Atom actualType;
    int format;
    pid_t pid = -1;
    unsigned long nitems, bytesAfter;
    unsigned char *propData = NULL;

    Atom atom = XInternAtom(display, "_NET_WM_PID", False);
    if (XGetWindowProperty(display, win, atom, 0, 1, False, XA_CARDINAL,
                           &actualType, &format, &nitems, &bytesAfter, &propData) != Success) return -1;
    if (propData) pid = *((pid_t*)propData);
    XFree(propData);
    return pid;
}
