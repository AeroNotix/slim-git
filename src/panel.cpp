/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004 Simone Rota <sip@varlock.com>
   Copyright (C) 2004 Johannes Winkelmann <jw@tks6.net>
      
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <sstream>
#include "panel.h"

using namespace std;

Panel::Panel(Display* dpy, int scr, Window root, Cfg* config) {
    // Set display
    Dpy = dpy;
    Scr = scr;
    Root = root;
    cfg = config;

    // Init GC
    XGCValues gcv;
    unsigned long gcm;
    gcm = GCForeground|GCBackground|GCGraphicsExposures;
    gcv.foreground = GetColor("black");
    gcv.background = GetColor("white");
    gcv.graphics_exposures = False;
    TextGC = XCreateGC(Dpy, Root, gcm, &gcv);

    font = XftFontOpenName(Dpy, Scr, cfg->getOption("input_font").c_str());
    welcomefont = XftFontOpenName(Dpy, Scr, cfg->getOption("welcome_font").c_str());
    enterfont = XftFontOpenName(Dpy, Scr, cfg->getOption("enter_font").c_str());
    msgfont = XftFontOpenName(Dpy, Scr, cfg->getOption("msg_font").c_str());

    Visual* visual = DefaultVisual(Dpy, Scr);
    Colormap colormap = DefaultColormap(Dpy, Scr);
    // NOTE: using XftColorAllocValue() would be a better solution. Lazy me.
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("input_fgcolor").c_str(), &fgcolor);
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("input_bgcolor").c_str(), &bgcolor);
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("msg_color").c_str(), &msgcolor);
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("welcome_color").c_str(), &welcomecolor);
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("enter_color").c_str(), &entercolor);

    // Load properties from config / theme
    input_name_x = Cfg::string2int(cfg->getOption("input_name_x").c_str());
    input_name_y = Cfg::string2int(cfg->getOption("input_name_y").c_str());
    input_pass_x = Cfg::string2int(cfg->getOption("input_pass_x").c_str());
    input_pass_y = Cfg::string2int(cfg->getOption("input_pass_y").c_str());
    if (input_pass_x < 0 || input_pass_y < 0){ // single inputbox mode
        input_pass_x = input_name_x;
        input_pass_y = input_name_y;
    }

    // Load panel and background image
    string panelpng = "";
    panelpng = panelpng + THEMESDIR + "/" + cfg->getOption("current_theme") +"/panel.png";
    image = new Image;
    bool loaded = image->Read(panelpng.c_str());
    if (!loaded) {
        cerr << APPNAME << ": could not load panel image" << endl;
        exit(ERR_EXIT);
    }
    Image* bg = new Image;
    panelpng = "";
    panelpng = panelpng + THEMESDIR + "/" + cfg->getOption("current_theme") +"/background.png";
    loaded = bg->Read(panelpng.c_str());
    if (!loaded) { // try jpeg if png failed
        panelpng = "";
        panelpng = panelpng + THEMESDIR + "/" + cfg->getOption("current_theme") +"/background.jpg";
        loaded = bg->Read(panelpng.c_str());
        if (!loaded){
            cerr << APPNAME << ": could not load background image" << endl;
            exit(ERR_EXIT);
        }
    }
    string bgstyle = cfg->getOption("background_style");
    if (bgstyle == "stretch"){
        bg->Resize(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)));
    } else {
        bg->Tile(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)));
    }
    
    // Calculate position. Can be absolute (expressed in pixels) or in percentage
    // TODO: create a function for percentage position calculation?
    int n = -1;
    string cfgX = cfg->getOption("input_panel_x");
    string cfgY = cfg->getOption("input_panel_y");
    int x, y;
    n = cfgX.find("%");
    if (n>0) { // X Position expressed in percentage
        cfgX = cfgX.substr(0, n);
        x = Cfg::string2int(cfgX.c_str());
        X = (XWidthOfScreen(ScreenOfDisplay(Dpy, Scr))*x/100) - (image->Width() / 2);
    } else { // Absolute X position
        X = Cfg::string2int(cfgX.c_str());
    }
    n = cfgY.find("%");
    if (n>0) { // Y Position expressed in percentage
        cfgY = cfgY.substr(0, n);
        y = Cfg::string2int(cfgY.c_str());
        Y = (XHeightOfScreen(ScreenOfDisplay(Dpy, Scr))*y/100) - (image->Height() / 2);
    } else { // Absolute Y position
        Y = Cfg::string2int(cfgY.c_str());
    }

    // Merge image into background
    image->Merge(bg, X, Y);
    PanelPixmap = image->createPixmap(Dpy, Scr, Root);

    
    // Read text position for welcome / enter messages
    // NOTE: x and y values are read when painting (ShowText())
    // since we allow % position and need to know fornt metrics
    welcome_y = Cfg::string2int(cfg->getOption("welcome_y").c_str());
    enter_y = Cfg::string2int(cfg->getOption("enter_y").c_str());
    welcome_message = cfg->getWelcomeMessage();

         
    // Init In
    In = new Input(cfg);
    
}


Panel::~Panel() {
    XftColorFree (Dpy, DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr), &fgcolor);
    XftColorFree (Dpy, DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr), &bgcolor);
    XftColorFree (Dpy, DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr), &msgcolor);
    XftColorFree (Dpy, DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr), &welcomecolor);
    XftColorFree (Dpy, DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr), &entercolor);
    XFreeGC(Dpy, TextGC);
    delete In;
    delete image;

}


void Panel::OpenPanel() {
    // Create window
    Win = XCreateSimpleWindow(Dpy, Root, X, Y,
                              image->Width(),
                              image->Height(),
                              0, GetColor("white"), GetColor("white"));
    //Win = Root;
    // Events
    XSelectInput(Dpy, Win, ExposureMask | KeyPressMask);

    // Set background
    XSetWindowBackgroundPixmap(Dpy, Win, PanelPixmap);
    
    // Show window
    XMapWindow(Dpy, Win);
    XMoveWindow(Dpy, Win, X, Y); // override wm positioning (for tests)

    // Grab keyboard
    XGrabKeyboard(Dpy, Win, False, GrabModeAsync, GrabModeAsync, CurrentTime);

    XFlush(Dpy);

}


void Panel::ClosePanel() {
    XUngrabKeyboard(Dpy, CurrentTime);
    XUnmapWindow(Dpy, Win);
    XDestroyWindow(Dpy, Win);
    XFlush(Dpy);
}


void Panel::ClearPanel() {
    In->Reset();
    XClearWindow(Dpy, Win);
    Cursor(SHOW);
    ShowText();
    XFlush(Dpy);
}

void Panel::Message(char* text) {
    XftDraw *draw = XftDrawCreate(Dpy, Root,
                                  DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr));
    XftDrawString8 (draw, &msgcolor, msgfont, 10, 25, (XftChar8*)text, strlen(text));
    XFlush(Dpy);
    XftDrawDestroy(draw);
}


Input* Panel::GetInput() {
    return In;
}


unsigned long Panel::GetColor(const char* colorname) {
    XColor color;
    XWindowAttributes attributes;

    XGetWindowAttributes(Dpy, Root, &attributes);
    color.pixel = 0;

    if(!XParseColor(Dpy, attributes.colormap, colorname, &color))
        cerr << APPNAME << ": can't parse color " << colorname << endl;
    else if(!XAllocColor(Dpy, attributes.colormap, &color))
        cerr << APPNAME << ": can't allocate color " << colorname << endl;

    return color.pixel;
}


void Panel::Cursor(int visible) {
    char* text;
    int xx, yy, x2,y2, cheight;
    char* txth = "Wj"; // used to get cursor height

    switch(In->GetField()) {
    case GET_PASSWD:
        text = In->GetHiddenPasswd();
        xx = input_pass_x;
        yy = input_pass_y;
        break;

    case GET_NAME:
        text = In->GetName();
        xx = input_name_x;
        yy = input_name_y;
        break;
    }

    if(visible == SHOW)
        XSetForeground(Dpy, TextGC, GetColor(cfg->getOption("input_fgcolor").c_str()));
    else
        XSetForeground(Dpy, TextGC, GetColor(cfg->getOption("input_bgcolor").c_str()));

    XGlyphInfo extents;
    XftTextExtents8(Dpy, font, (XftChar8*)txth, strlen(txth), &extents);
    cheight = extents.height;
    y2 = yy - extents.y + extents.height;
    XftTextExtents8(Dpy, font, (XftChar8*)text, strlen(text), &extents);
    xx += extents.width;

    XDrawLine(Dpy, Win, TextGC,
              xx+1, yy-cheight,
              xx+1, y2);
}


int Panel::EventHandler(XEvent* event) {
    Action = WAIT;

    switch(event->type) {
    case Expose:
        OnExpose(event);
        break;

    case KeyPress:
        OnKeyPress(event);
        break;
    }

    return Action;
}


void Panel::OnExpose(XEvent* event) {
    char* name = In->GetName();
    char* passwd = In->GetHiddenPasswd();
    XftDraw *draw = XftDrawCreate(Dpy, Win,
                        DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr));
    if (input_pass_x != input_name_x || input_pass_y != input_name_y){ 
        XftDrawString8 (draw, &fgcolor, font, input_name_x, input_name_y,
             (XftChar8*)name, strlen(name));
        XftDrawString8 (draw, &fgcolor, font, input_pass_x, input_pass_y,
             (XftChar8*)passwd, strlen(passwd));
    } else { //single input mode
        switch(In->GetField()) {
            case GET_PASSWD:
                XftDrawString8 (draw, &fgcolor, font, input_pass_x, input_pass_y,
                     (XftChar8*)passwd, strlen(passwd));
                break;
            case GET_NAME:
                XftDrawString8 (draw, &fgcolor, font, input_name_x, input_name_y,
                     (XftChar8*)name, strlen(name));
                break;
        }    
    }

    XftDrawDestroy (draw);
    Cursor(SHOW);
    ShowText();
}


void Panel::OnKeyPress(XEvent* event) {
    char del;
    char buffer;
    KeySym keysym;
    XComposeStatus compstatus;
    int xx;
    int yy;
    char* text;

    bool singleInputMode = 
        input_name_x == input_pass_x && 
        input_name_y == input_pass_y;
    Cursor(HIDE);
    XLookupString(&event->xkey, &buffer, 1, &keysym, &compstatus);
    del = In->Key(buffer, keysym, singleInputMode);
    Action = In->GetAction();

    XGlyphInfo extents, delextents;
    XftDraw *draw = XftDrawCreate(Dpy, Win,
                                  DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr));

    switch(In->GetField()) {
    case GET_PASSWD:
        if (strlen(In->GetHiddenPasswd()) == 0){ 
            // clear name and welcome label if we just entered the password field
            if (singleInputMode) {
                xx = input_name_x;
                yy = input_name_y;
                text = In->GetName();
                XftTextExtents8(Dpy, font, (XftChar8*)text, strlen(text), &extents);
                XftDrawRect(draw, &bgcolor, xx-2, yy-extents.height-2,
                            extents.width+4, extents.height+4);
                XClearWindow(Dpy, Win);
            ShowText();
        }
        }
        text = In->GetHiddenPasswd();
        xx = input_pass_x;
        yy = input_pass_y;
        break;

    case GET_NAME:
        text = In->GetName();
        xx = input_name_x;
        yy = input_name_y;
        break;
    }
 
    XftTextExtents8(Dpy, font, (XftChar8*)text, strlen(text), &extents);

    if(del == 0) {
        // No character deleted
        XftDrawRect(draw, &bgcolor, xx-1, yy-extents.height-1,
                    extents.width+2, extents.height+2);
        XftDrawString8 (draw, &fgcolor, font, xx, yy, (XftChar8*)text, strlen(text));
    } else // Delete char
    {
        string tmp = "";
        tmp = tmp + text;
        tmp = tmp + del;
        XftTextExtents8(Dpy, font, (XftChar8*)tmp.c_str(), strlen(tmp.c_str()), &extents);
        char* txth = "Wj"; // get proper maximum height ?
        XftTextExtents8(Dpy, font, (XftChar8*)txth, strlen(txth), &extents);
        int mh = extents.height;
        XftTextExtents8(Dpy, font, (XftChar8*)tmp.c_str(), strlen(tmp.c_str()), &extents);
        XftDrawRect(draw, &bgcolor, xx-3, yy-mh-3,
                    extents.width+6, mh+6);
        XftDrawString8 (draw, &fgcolor, font, xx, yy, (XftChar8*)text, strlen(text));
    }

    XftDrawDestroy (draw);
    Cursor(SHOW);
    
}


// Draw welcome and "enter username" message
void Panel::ShowText(){
    string cfgX;
    int n=-1;
    int x;
    XGlyphInfo extents;
    
    XftDraw *draw = XftDrawCreate(Dpy, Win,
                                  DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr));
    // welcome message
    cfgX = cfg->getOption("welcome_x");
    n = cfgX.find("%");
    if (n>0) { // X Position expressed in percentage
        XftTextExtents8(Dpy, welcomefont, (XftChar8*)welcome_message.c_str(),
                        strlen(welcome_message.c_str()), &extents);
        cfgX = cfgX.substr(0, n);
        x = Cfg::string2int(cfgX.c_str());
        welcome_x = (image->Width()*x/100) - (extents.width / 2);
    } else { // Absolute X position
        welcome_x = Cfg::string2int(cfg->getOption("welcome_x").c_str());;
    }
    if (welcome_x>=0 && welcome_y >=0){
        XftDrawString8 (draw, &welcomecolor, welcomefont, welcome_x, welcome_y, 
            (XftChar8*)welcome_message.c_str(), strlen(welcome_message.c_str()));
    }
    
    // Enter username/password message
    string s;
    switch(In->GetField()) {
        case GET_PASSWD:
            s = cfg->getOption("password_msg");
            break;
        case GET_NAME:
            s = cfg->getOption("username_msg");
            break;
    }    
    cfgX = cfg->getOption("enter_x");
    n = cfgX.find("%");
    if (n>0) { // X Position expressed in percentage
        XftTextExtents8(Dpy, enterfont, (XftChar8*)s.c_str(),
                        strlen(s.c_str()), &extents);
        cfgX = cfgX.substr(0, n);
        x = Cfg::string2int(cfgX.c_str());
        enter_x = (image->Width()*x/100) - (extents.width / 2);
    } else { // Absolute X position
        enter_x = Cfg::string2int(cfg->getOption("enter_x").c_str());;
    }
    if (enter_x>=0 && enter_y >=0){
        XftDrawString8 (draw, &entercolor, enterfont, enter_x, enter_y, 
                (XftChar8*)s.c_str(), strlen(s.c_str()));
    }

    XftDrawDestroy(draw);
 
}

