#ifndef _H_KEYBOARD
#define _H_KEYBOARD
enum
{
    KEY_TAB=1,
    KEY_CAPLOCK,
    KEY_SHIFT,
    KEY_CTRL,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT
};
#ifndef __USER__LIB
void keyboard_init();
char keyboard_get_key();
#endif
#endif