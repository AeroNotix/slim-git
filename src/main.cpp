/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004 Simone Rota <sip@varlock.com>
   Copyright (C) 2004 Johannes Winkelmann <jw@tks6.net>
      
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include "app.h"
#include "const.h"

App* LoginApp = 0;

int main(int argc, char** argv) {
    if(getuid() == 0) {
        LoginApp = new App(argc, argv);
        LoginApp->Run();
    } else
        cerr << APPNAME << ": only root can run this program" << endl;
    return 0;
}

