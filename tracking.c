#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <X11/cursorfont.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>

#include "util.h"
#include "drw.h"
#include "winutil.h"

void (*handler[LASTEvent]) (XEvent *) =
{
};
/*
gcc -o tracker tracking.c util.c drw.c winutil.c
*/

#define MAX_COLOR_DEPTH             3

/* colours */
#define COL_BLACK           "#000000"
#define COL_WHITE           "#ffffff"
#define COL_GREY            "#C0C0C0"
#define COL_RED             "#ff0000"
#define COL_PINK            "#FF00FF"
#define COL_BLUE            "#0000FF"
#define COL_YELLOW          "#FFFF00"

char *trckgetwinattr(Window win);
void trckdrwnode(int x, int y, int w, int h, char *text);
void trckdrwnodes(int x, int y);
char *trckgetstatus(Window win);
Window trckwin(Display *display, Window parent, int x, int y, int w, int h);
void trackersetup();
void trackerstop();
void trackerrun();

void arghandler();
void cleanup();
void run();
void setup();

enum
{
    SchemeNorm, 
};

char *colors[][MAX_COLOR_DEPTH] = 
{
    /*				fg         bg          border   */
    [SchemeNorm] = { COL_WHITE, COL_BLACK, COL_BLACK}, /* Scheme normal */
};

static const char *fonts[]      =   {"monospace:size=12" };

Display *dpy;
Window root;
int screen;
Drw *drw;
Clr **scheme;
int sw, sh;
Window mainwin;
int running = 1;
void
arghandler()
{

}

void
cleanup()
{
    XCloseDisplay(dpy);
}

void
setup()
{
    int i; /* holder */
    /* checks */
    if(!XInitThreads()) die("FATAL: NO_MULTI_THREAD_SUPPORT");
    if(!(dpy = XOpenDisplay(NULL))) die("FATAL: CANNOT_OPEN_DISPLAY");


    /* vars */
    screen = DefaultScreen(dpy);
    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);
    root = RootWindow(dpy, screen);
    drw = drw_create(dpy, screen, root, sw, sh);
    root = DefaultRootWindow(dpy);

    /* other checks */
    if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
        die("FATAL ERROR: NO FONTS LOADED.");

    /* scheme */
    scheme = ecalloc(LENGTH(colors), sizeof(Clr *));
    for (i = 0; i < LENGTH(colors); ++i) scheme[i] = drw_scm_create(drw, colors[i], MAX_COLOR_DEPTH);
    trackersetup();
}

void
run()
{
    XEvent ev;
    while(running && !XNextEvent(dpy, &ev))
    {
    }
}
Window
trckwin(Display *display, Window parent, int x, int y, int w, int h)
{
    /* window stuff */
    Atom wt = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom wtnormal= XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    Window win;
    XSizeHints hints;
    XSetWindowAttributes xwa = 
    {
        .background_pixmap = None,
    };
    hints.min_width = 50;
    hints.min_height = 50;
    hints.max_width = sw;
    hints.max_height = sh;
    hints.flags = PMinSize | PMaxSize;

    /* setting tracker */
    win = XCreateWindow(display, parent, x, y, w, h, 0, DefaultDepth(dpy, screen), InputOutput, DefaultVisual(dpy, screen), 
                CWBackPixmap, &xwa);
    /* window stuff */
    XSetWMNormalHints(display, win, &hints);
	XStoreName(display, win, "User Stats.");
    XChangeProperty(display, win, wt, XA_ATOM, 32, PropModeReplace, (unsigned char*)&wtnormal, 1);
    XSelectInput(display, win, ExposureMask|StructureNotifyMask);
    return win;
}

void
trackersetup()
{
    int x; 
    int y;
    int w;
    int h;
    int curx, cury;
    curx = cury = 0;
    w = sw * .70;
    h = sh *.90;
    x = (sw - w) >> 1;
    y = (sh - h) >> 1;
    mainwin = trckwin(dpy, root, x, y, w, h);
    /* draw empty canvas */
    drw_setscheme(drw, scheme[SchemeNorm]);
    drw_rect(drw, 0, 0, w, h, 1, 1);
    cury += drw->fonts->h;
    trckdrwnodes(w >> 1, 0);
    drw_map(drw, mainwin, 0, 0, mainwin, mainwin);
    XMapWindow(dpy, mainwin);
}

void
trckdrwnode(int x, int y, int w, int h, char *text)
{
    int textlen = TEXTW(text);
    drw_text(drw, x, y + ((h - drw->fonts->h) >> 1), w, drw->fonts->h, MAX((w - textlen) >> 1, 0), text, 0);
    drw_rect(drw, x, y, w, h, 0, 0);
}

void
trckdrwnodes(int x, int y)
{
    int w, h;
    int mw, mh; /* max(w/h) */
    int txtw;
    Window rootwin, parentwin, win, *wins = NULL;
    XWindowAttributes xwa;
    unsigned int num;
    unsigned int i;
    char *status;
    char *winname;
    if (!XQueryTree(dpy, root, &rootwin, &parentwin, &wins, &num)) return;
    if(!XGetWindowAttributes(dpy, mainwin, &xwa)) return;
    w = 0;
    h = drw->fonts->h << 1;
    mw = xwa.width;
    mh = xwa.height;
    txtw = 0;

    for(i = 0; i < num; ++i)
    {
        win = wins[i];
        status = trckgetstatus(win);
        winname = XGetWindowName(dpy, win);
        txtw = TEXTW(winname);
        trckdrwnode(w, y + h, txtw, drw->fonts->h, winname);
        w += txtw;
        drw_text(drw, mw - (TEXTW(status)), y + h, TEXTW(status), drw->fonts->h, 1, status, 0);
        drw_rect(drw, w, y + h, mw, drw->fonts->h, 0, 0);
        if(status) free(status);
        /* reset coordinates */
        h += drw->fonts->h + 1; /* +1 for alignment */ 
        w = 0;
    }
    free(wins);
}

char *trckgetstatus(Window win)
{
    /* [0] Window [1] Ping time [4] Success/Failure/NotSet */
    long unsigned int *pingdata = NULL;
    pid_t pid = -1;
    int responsetime = 0;
    char *response = "NOT_SET";
    pid = XGetPid(dpy, win);
    //pingdata = XPingWindow(dpy, win, 1);
    if(pingdata)
    {
        if(!pingdata[4]) response = "GOOD";
        else if(pingdata[4] == 1) response = "FAILED";
        responsetime = pingdata[1];
        free(pingdata);
    }
    return smprintf("%d [%s] [%lums]", pid, response, responsetime);
}

char *spinner()
{
    static int angle = 0; angle^=1;
    if(angle) return "|";
    return "--";
}


char *trckgetwinattr(Window win)
{
    XWindowAttributes xwa;
    char *ret;
    if(!XGetWindowAttributes(dpy, win, &xwa)) return NULL;
    ret = smprintf( 
        "-Window dimentions-"
        "\n(x: %d,y: %d) (w: %d,h: %d)"
        "\nborder_width: %d depth: %d"
        "\n-Storing-"
        "\nclass: %d back_store: %d save_under %d"
        "\nmap_installed: %d map_state: %d override_redirect: %d"
        "\n-Event Masks-"
        "\nall_event_masks: %d your_event_mask: %d"
        "\ndo_not_propagate_mask %d"
        ,
        xwa.x, xwa.y, xwa.width, xwa.height, 
        xwa.border_width, xwa.depth,
        xwa.class, xwa.backing_store, xwa.save_under,
        xwa.map_installed, xwa.map_state, xwa.override_redirect,
        xwa.all_event_masks, xwa.your_event_mask,
        xwa.do_not_propagate_mask
        );
    return ret;
}
void
trackerrun()
{
    if(!mainwin) trackersetup();
    else trackerstop();
}

void
trackerstop()
{
    if(mainwin)
    {
        XUnmapWindow(dpy, mainwin);
        XDestroyWindow(dpy, mainwin);
    }
}


int
main(int argc, char **argv)
{
    arghandler(argc, argv);
    setup();
    run();
    cleanup();
    return EXIT_SUCCESS;
}
