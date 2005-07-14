AC_INIT(src/main.cpp)
AM_INIT_AUTOMAKE(slim,1.2.2)

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

PKG_CHECK_MODULES(DEPS, [xft >= 2.0 libpng >= 1.2.7])
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

AC_CHECK_LIB(jpeg, jpeg_read_header)
if test "x$ac_cv_lib_jpeg_jpeg_read_header" != "xyes" ; then
         echo "ERROR: libjpeg seems to be missing on your system..."
         echo "  please install first"
         exit 1
fi

AC_PREFIX_DEFAULT(/usr)

AC_OUTPUT(Makefile
          src/Makefile
          doc/Makefile
          orig/Makefile
          themes/Makefile
          themes/default/Makefile)
