/* SLiM - Simple Login Manager
   Copyright (C) 2004-05 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-05 Johannes Winkelmann <jw@tks6.net>
      
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#ifndef _CFG_H_
#define _CFG_H_

#include <string>
#include <map>
#include <vector>

#define INPUT_MAXLENGTH_NAME    30
#define INPUT_MAXLENGTH_PASSWD  50

#define CFGFILE SYSCONFDIR"/slim.conf"
#define THEMESDIR PKGDATADIR"/themes"

using namespace std;

class Cfg {

public:
    Cfg();
    void readConf(string configfile);
    string parseOption(string line, string option);
    const string& getError() const;
    string& getOption(string option);
    string getWelcomeMessage();
    string getLoginCommand(const string& session);
        
    static int absolutepos(const string& position, int max, int width);
    static int string2int(const char* string, bool* ok = 0);
    
    string nextSession(string current);

private:
    void split(vector<string>& v, const string& str, char c);
    string Trim(const string& s);
    map<string,string> options;
    string error;

};

#endif
