/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004 Simone Rota <sip@varlock.com>
   Copyright (C) 2004 Johannes Winkelmann <jw@tks6.net>
      
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#ifndef _PANEL_H_
#define _PANEL_H_

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/cursorfont.h>
#include <X11/Xmu/WinUtil.h>
#include <sys/wait.h>
#include <getopt.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include "switchuser.h"
#include "input.h"
#include "const.h"
#include "image.h"


class Panel {
public:
    Panel(Display* dpy, int scr, Window root, Cfg* config);
    ~Panel();
    void OpenPanel();
    void ClosePanel();
    void ClearPanel();
    void Message(char* text);
    Input* GetInput();
    int EventHandler(XEvent* event);

private:
    Panel();
    void Cursor(int visible);
    unsigned long GetColor(const char* colorname);
    void OnExpose(XEvent* event);
    void OnKeyPress(XEvent* event);
    void ShowText();
    Cfg* cfg;

    // Private data
    Window Win;
    Window Root;
    Display* Dpy;
    int Scr;
    int X, Y;
    GC TextGC;
    XftFont* font;
    XftColor fgcolor;
    XftColor bgcolor;
    XftColor inputcolor;
    XftColor msgcolor;
    XftFont* msgfont;
    XftColor introcolor;
    XftFont* introfont;
    XftFont* welcomefont;
    XftColor welcomecolor;
    XftFont* enterfont;
    XftColor entercolor;
    int Action;

    // Configuration
    int input_name_x;
    int input_name_y;
    int input_pass_x;
    int input_pass_y;
    int input_cursor_height;
    int welcome_x;
    int welcome_y;
    int intro_x;
    int intro_y;
    int username_x;
    int username_y;
    int password_x;
    int password_y;
    string welcome_message;
    string intro_message;

    // Pixmap data
    Pixmap PanelPixmap;

    // Name/Passwd handler
    Input* In;

    Image* image;

};

#endif


