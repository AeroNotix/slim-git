/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004 Simone Rota <sip@varlock.com>
   Copyright (C) 2004 Johannes Winkelmann <jw@tks6.net>
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/


#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdio>

#include <sstream>
#include "app.h"
#include "image.h"


extern App* LoginApp;

void CatchSignal(int sig) {
    cerr << APPNAME << ": unexpected signal " << sig << endl;
    LoginApp->StopServer();
    LoginApp->RemoveLock();
    exit(ERR_EXIT);
}


void AlarmSignal(int sig) {
    int pid = LoginApp->GetServerPID();
    if(waitpid(pid, NULL, WNOHANG) == pid) {
        LoginApp->StopServer();
        LoginApp->RemoveLock();
        exit(OK_EXIT);
    }
    signal(sig, AlarmSignal);
    alarm(2);
}


void User1Signal(int sig) {
    signal(sig, User1Signal);
}


App::App(int argc, char** argv) {
    int tmp;
    ServerPID = -1;

    // Parse command line
    while((tmp = getopt(argc, argv, "vh?")) != EOF) {
        switch (tmp) {
        case 'v':	// Version
            cout << APPNAME << " version " << VERSION << endl;
            exit(OK_EXIT);
            break;
        case '?':	// Ilegal
            cerr << endl;
        case 'h':   // Help
            cerr << "usage:  " << APPNAME << " [option ...]" << endl
            << "options:" << endl
            << "    -version" << endl;
            exit(OK_EXIT);
            break;
        }
    }
}


void App::Run() {
    // Read configuration and theme
    cfg.readConf(CFGFILE);
    string themefile = "";
    themefile = themefile + THEMESDIR + "/" + cfg.getOption("current_theme") +"/slim.theme";
    cfg.readConf(themefile);

     // Create lock file
    LoginApp->GetLock();
   
    // Start x-server
    setenv("DISPLAY", DISPLAY, 1);
    signal(SIGQUIT, CatchSignal);
    signal(SIGTERM, CatchSignal);
    signal(SIGKILL, CatchSignal);
    signal(SIGINT, CatchSignal);
    signal(SIGHUP, CatchSignal);
    signal(SIGPIPE, CatchSignal);
    signal(SIGUSR1, User1Signal);
    signal(SIGALRM, AlarmSignal);

    OpenLog();    
    
    if (daemon(0, 1) == -1) {
        cerr << APPNAME << ": " << strerror(errno) << endl;
        exit(ERR_EXIT);
    }
   
    StartServer();
    alarm(2);

    // Open display
    if((Dpy = XOpenDisplay(DISPLAY)) == 0) {
        cerr << APPNAME << ": could not open display '" << DISPLAY << "'" << endl;
        StopServer();
        exit(ERR_EXIT);
    }

    // Get screen and root window
    Scr = DefaultScreen(Dpy);
    Root = RootWindow(Dpy, Scr);

    // Create panel
    LoginPanel = new Panel(Dpy, Scr, Root, &cfg);

    // Start looping
    XEvent event;
    int panelclosed = 1;
    int Action;
    bool firstloop = true; // 1st time panel is shown (for automatic username)

    while(1) {
        if(panelclosed) {
            // Init root
            setBackground();

            // Close all clients
            KillAllClients(False);
            KillAllClients(True);

            // Show panel
            LoginPanel->OpenPanel();
        }

        Action = WAIT;
        LoginPanel->GetInput()->Reset();
        if (firstloop && cfg.getOption("default_user") != ""){
            LoginPanel->GetInput()->SetName(cfg.getOption("default_user") );
            firstloop = false;
        }
        
        while(Action == WAIT) {
            XNextEvent(Dpy, &event);
            Action = LoginPanel->EventHandler(&event);
        }

        if(Action == FAIL) {
            panelclosed = 0;
            LoginPanel->ClearPanel();
            XBell(Dpy, 100);
        } else {
            panelclosed = 1;
            LoginPanel->ClosePanel();

            switch(Action) {
            case LOGIN:
                Login();
                break;
            case CONSOLE:
                Console();
                break;
            case REBOOT:
                Reboot();
                break;
            case HALT:
                Halt();
                break;
            case EXIT:
                Exit();
                break;
            }
        }
    }
}


int App::GetServerPID() {
    return ServerPID;
}


void App::Login() {
    struct passwd *pw;
    pid_t pid;

    pw = LoginPanel->GetInput()->GetPasswdStruct();
    if(pw == 0)
        return;

    // Create new process
    pid = fork();
    if(pid == 0) {
        // Login process starts here
        SwitchUser Su(pw, &cfg);
        Su.Login(cfg.getOption("login_cmd").c_str());
        exit(OK_EXIT);
    }

    // Wait until user is logging out (login process terminates)
    pid_t wpid = -1;
    while (wpid != pid)
        wpid = wait(NULL);

    //    waitpid(pid, 0, 0);

    // Close all clients
    KillAllClients(False);
    KillAllClients(True);

    // Send HUP signal to clientgroup
    killpg(pid, SIGHUP);

    // Send TERM signal to clientgroup, if error send KILL
    if(killpg(pid, SIGTERM))
    killpg(pid, SIGKILL);
}


void App::Reboot() {
    // Stop alarm clock
    alarm(0);

    // Write message
    LoginPanel->Message("System rebooting...");
    sleep(3);

    // Stop server and reboot
    StopServer();
    RemoveLock();
    system(cfg.getOption("reboot_cmd").c_str());
    exit(OK_EXIT);
}


void App::Halt() {
    // Stop alarm clock
    alarm(0);

    // Write message
    LoginPanel->Message("System halting...");
    sleep(3);

    // Stop server and halt
    StopServer();
    RemoveLock();
    system(cfg.getOption("halt_cmd").c_str());
    exit(OK_EXIT);
}


void App::Console() {
    int posx = 40;
    int posy = 40;
    int fontx = 9;
    int fonty = 15;
    int width = (XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)) - (posx * 2)) / fontx;
    int height = (XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)) - (posy * 2)) / fonty;

    // Execute console
    const char* cmd = cfg.getOption("console_cmd").c_str();
    char *tmp = new char[strlen(cmd) + 60];
    sprintf(tmp, cmd, width, height, posx, posy, fontx, fonty);
    system(tmp);
    delete [] tmp;
}


void App::Exit() {
    // Deallocate and stop server
    delete LoginPanel;
    StopServer();
    RemoveLock();
    exit(OK_EXIT);
}


int CatchErrors(Display *dpy, XErrorEvent *ev) {
    return 0;
}


void App::KillAllClients(Bool top) {
    Window dummywindow;
    Window *children;
    unsigned int nchildren;
    unsigned int i;
    XWindowAttributes attr;

    XSync(Dpy, 0);
    XSetErrorHandler(CatchErrors);

    nchildren = 0;
    XQueryTree(Dpy, Root, &dummywindow, &dummywindow, &children, &nchildren);
    if(!top) {
        for(i=0; i<nchildren; i++) {
            if(XGetWindowAttributes(Dpy, children[i], &attr) && (attr.map_state == IsViewable))
                children[i] = XmuClientWindow(Dpy, children[i]);
            else
                children[i] = 0;
        }
    }

    for(i=0; i<nchildren; i++) {
        if(children[i])
            XKillClient(Dpy, children[i]);
    }
    XFree((char *)children);

    XSync(Dpy, 0);
    XSetErrorHandler(NULL);
}


int App::ServerTimeout(int timeout, char* text) {
    int	i = 0;
    int pidfound = -1;
    static char	*lasttext;

    for(;;) {
        pidfound = waitpid(ServerPID, NULL, WNOHANG);
        if(pidfound == ServerPID)
            break;
        if(timeout) {
            if(i == 0 && text != lasttext)
                cerr << endl << APPNAME << ": waiting for " << text;
            else
                cerr << ".";
        }
        if(timeout)
            sleep(1);
        if(++i > timeout)
            break;
    }

    if(i > 0)
        cerr << endl;
    lasttext = text;

    return (ServerPID != pidfound);
}


int App::WaitForServer() {
    int	ncycles	 = 120;
    int	cycles;

    for(cycles = 0; cycles < ncycles; cycles++) {
        if((Dpy = XOpenDisplay(DISPLAY))) {
            return 1;
        } else {
            if(!ServerTimeout(1, "X server to begin accepting connections"))
                break;
        }
    }

    cerr << "Giving up." << endl;

    return 0;
}


int App::StartServer() {
    ServerPID = vfork();

    static const int MAX_XSERVER_ARGS = 256;
    static char* server[MAX_XSERVER_ARGS+2] = { NULL };
    server[0] = (char *)cfg.getOption("default_xserver").c_str();
    string argOption = cfg.getOption("xserver_arguments");
    char* args = new char[argOption.length()+1];
    strcpy(args, argOption.c_str());

    int argc = 1;
    int pos = 0;
    while (args[pos] != '\0') {
        if (args[pos] == ' ' || args[pos] == '\t') {
            *(args+pos) = '\0';
            server[argc++] = args+pos+1;
        } else if (pos == 0) {
            server[argc++] = args+pos;
        }
        ++pos;

        if (argc+1 >= MAX_XSERVER_ARGS) {
            // ignore _all_ arguments to make sure the server starts at
            // all
            argc = 1; 
            break;
        }
    }
    server[argc] = NULL;

    switch(ServerPID) {
    case 0:
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGUSR1, SIG_IGN);
        setpgid(0,getpid());
        
        
        execvp(server[0], server);
        cerr << APPNAME << ": X server could not be started" << endl;
        exit(ERR_EXIT);
        break;

    case -1:
        break;

    default:
        errno = 0;
        if(!ServerTimeout(0, "")) {
            ServerPID = -1;
            break;
        }
        alarm(15);
        pause();
        alarm(0);

        // Wait for server to start up
        if(WaitForServer() == 0) {
            cerr << APPNAME << ": unable to connect to X server" << endl;
            StopServer();
            ServerPID = -1;
            exit(ERR_EXIT);
        }
        break;
    }

    delete args;

    return ServerPID;
}


jmp_buf CloseEnv;
int IgnoreXIO(Display *d) {
    cerr << APPNAME << ": connection to X server lost." << endl;
    longjmp(CloseEnv, 1);
}


void App::StopServer() {
    // Stop alars clock and ignore signals
    alarm(0);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, SIG_DFL);
    signal(SIGKILL, SIG_DFL);
    signal(SIGALRM, SIG_DFL);

    // Catch X error
    XSetIOErrorHandler(IgnoreXIO);
    if(!setjmp(CloseEnv))
        XCloseDisplay(Dpy);

    // Send HUP to process group
    errno = 0;
    if((killpg(getpid(), SIGHUP) != 0) && (errno != ESRCH))
        cerr << APPNAME << ": can't send HUP to process group " << getpid() << endl;

    // Send TERM to server
    if(ServerPID < 0)
        return;
    errno = 0;
    if(killpg(ServerPID, SIGTERM) < 0) {
        if(errno == EPERM) {
            cerr << APPNAME << ": can't kill X server" << endl;
            exit(ERR_EXIT);
        }
        if(errno == ESRCH)
            return;
    }

    // Wait for server to shut down
    if(!ServerTimeout(10, "X server to shut down")) {
        cerr << endl;
        return;
    }

    cerr << endl << APPNAME << ":  X server slow to shut down, sending KILL signal." << endl;

    // Send KILL to server
    errno = 0;
    if(killpg(ServerPID, SIGKILL) < 0) {
        if(errno == ESRCH)
            return;
    }

    // Wait for server to die
    if(ServerTimeout(3, "server to die")) {
        cerr << endl << APPNAME << ": can't kill server" << endl;
        exit(ERR_EXIT);
    }
    cerr << endl;
}

void App::setBackground() {
    string filename = "";
    filename = filename + THEMESDIR + "/" + cfg.getOption("current_theme") +"/background.png";
    Image *image = new Image;
    bool loaded = image->Read(filename.c_str());
    if (!loaded){ // try jpeg if png failed
        filename = "";
        filename = filename + THEMESDIR + "/" + cfg.getOption("current_theme") +"/background.jpg";
        loaded = image->Read(filename.c_str());
    }
    if (loaded) {
        string bgstyle = cfg.getOption("background_style");
        if (bgstyle == "stretch") {
            image->Resize(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)));
        } else {
            image->Tile(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)));
        }
        Pixmap p = image->createPixmap(Dpy, Scr, Root);
        XSetWindowBackgroundPixmap(Dpy, Root, p);
    }
    XClearWindow(Dpy, Root);

    XFlush(Dpy);
}

// Lock or die!
void App::GetLock() {
    int fd;
    fd=open(cfg.getOption("lockfile").c_str(),O_WRONLY | O_CREAT | O_EXCL);
    if (fd<0 && errno==EEXIST) {
        cerr << APPNAME << ": It appears there is another instance of the program already running" <<endl
            << "If not, try to remove the lockfile: " << cfg.getOption("lockfile") <<endl;
        exit(ERR_EXIT);
    } else if (fd < 0) {
        cerr << APPNAME << ": Could not accesss lock file: " << cfg.getOption("lockfile") << endl;
        exit(ERR_EXIT);
    }
}

// Remove lockfile and close logs
void App::RemoveLock() {
    remove(cfg.getOption("lockfile").c_str());
    fclose(stderr); // release stderr (and stdout) association for logfile
}

// Redirect stdout and stderr to log file
void App::OpenLog() {
    FILE *log = fopen (cfg.getOption("logfile").c_str(),"a");
    if (!log) {
        cerr <<  APPNAME << ": Could not accesss log file: " << cfg.getOption("logfile") << endl;
        RemoveLock();
        exit(ERR_EXIT);
    }
    fclose(log);
    freopen (cfg.getOption("logfile").c_str(),"a",stdout);
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    freopen (cfg.getOption("logfile").c_str(),"a",stderr);
    setvbuf(stderr, NULL, _IONBF, BUFSIZ);
}
