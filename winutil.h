#ifndef WINTUTIL_H_
#define WINTUTIL_H_

#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define TEXTW(X)                (drw_fontset_getwidth(drw, (X)))
#define OPAQUE                  0xffU
#define SESSION_FILE            "/tmp/dwm-session"
#define BROKEN                  "borked."
/* motif window decoration */
#define MWM_HINTS_FLAGS_FIELD       0
#define MWM_HINTS_DECORATIONS_FIELD 2
#define MWM_HINTS_DECORATIONS       (1 << 1)
#define MWM_DECOR_ALL               (1 << 0)
#define MWM_DECOR_BORDER            (1 << 1)
#define MWM_DECOR_TITLE             (1 << 3)



long unsigned int *XPingWindow(Display *display, Window win, int wait_time_seconds);
char *XGetWindowName(Display *display, Window win);
int XGetTextProp(Display *display, Window w, Atom atom, char *text, unsigned int size);
pid_t XGetPid(Display *display, Window win);

#endif
