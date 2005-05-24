AC_INIT(src/main.cpp)
AM_INIT_AUTOMAKE(slim,1.2.1)

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

PKG_CHECK_MODULES(DEPS, xft >= 2.0)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

AC_CHECK_LIB(jpeg, jpeg_read_header)
if test "x$ac_cv_lib_jpeg_jpeg_read_header" != "xyes" ; then
         echo "ERROR: libjpeg seems to be missing on your system..."
         echo "  please install first"
         exit 1
fi

AC_CHECK_LIB(png,png_info_init)
if test "x$ac_cv_lib_png_png_info_init" != "xyes" ; then
         echo "ERROR: libpng seems to be missing on your system..."
         echo "  please install first"
         exit 1
fi

AC_CONFIG_HEADERS(config.h)
AC_CHECK_HEADERS(libgen.h)
AC_CHECK_FUNCS(basename dirname)

AC_PREFIX_DEFAULT(/usr)


AC_ARG_ENABLE(shadow, AC_HELP_STRING([--disable-shadow], [Disable support for shadow passwords]),,
		enable_shadow=yes)

if test "A$enable_shadow" = "Ayes"; then
  AC_CHECK_HEADER([shadow.h], , shadow_available=no)
fi

AC_MSG_CHECKING(for shadow passwords support)

if test "A$enable_shadow" = "Ayes"; then
  if test "A$shadow_available" = "Ano"; then
	AC_MSG_RESULT(no)
 	echo "***"
	echo "*** Warning: shadow passwords support disabled"
	echo "***"
	enable_shadow="no"
  else
 	AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_SHADOW,1,[Shadow Password support ])
  fi
else
 	AC_MSG_RESULT(no)
	echo "***"
	echo "*** Shadow passwords support disabled on command line"
	echo "*** Use --enable-shadow to enable it"
	echo "***"
	enable_shadow="no"
fi

AC_OUTPUT(Makefile
          src/Makefile
          doc/Makefile
          orig/Makefile
          themes/Makefile
          themes/default/Makefile)
