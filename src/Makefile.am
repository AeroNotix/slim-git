bin_PROGRAMS = slim
slim_SOURCES = const.h \
                  cfg.h cfg.cpp \
				  image.h image.cpp \
                  png.c jpeg.c \
                  switchuser.h switchuser.cpp \
                  app.h app.cpp \
                  input.h input.cpp \
                  panel.h panel.cpp \
                  main.cpp

INCLUDES = $(DEPS_CFLAGS) -I/usr/include
LIBS = $(DEPS_LIBS) -lcrypt -lXpm -lXmu -lpng -ljpeg

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" -DSYSCONFDIR=\"$(sysconfdir)\"
