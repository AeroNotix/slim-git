/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004-05 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-05 Johannes Winkelmann <jw@tks6.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#ifndef _CONST_H_
#define _CONST_H_


#define APPNAME		"slim"

#define DISPLAY		":0.0"

#define CONSOLE_STR     "console"
#define HALT_STR        "halt"
#define REBOOT_STR      "reboot"
#define EXIT_STR        "exit"
#define SUSPEND_STR     "suspend"

#define HIDE		0
#define SHOW		1

#define WAIT		0
#define LOGIN		1
#define FAIL		2
#define CONSOLE		3
#define REBOOT		4
#define HALT		5
#define EXIT		6
#define SUSPEND		7

#define GET_NAME    0
#define GET_PASSWD	1

#define	OK_EXIT		0
#define	ERR_EXIT	1

// duration for showing error messages, as "login command failed", in seconds
#define ERROR_DURATION  5


#endif

