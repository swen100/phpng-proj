dnl
dnl Proj for PHP
dnl

PHP_ARG_WITH(proj, whether to enable Proj support,
[  --with-proj[=DIR]       Enable Proj support. DIR is the PROJ install prefix])

if test "$PHP_PROJ" != "no"; then
  if test "$PHP_PROJ" = "yes"; then
    PROJ_DIR=/usr
  else
    PROJ_DIR=$PHP_PROJ
    if test ! -f "$PROJ_DIR/include/proj.h"; then
      AC_MSG_ERROR([proj.h not found in $PROJ_DIR/include])
    fi
  fi

  PHP_ADD_INCLUDE($PROJ_DIR/include)

  PHP_CHECK_LIBRARY(proj, proj_trans, [
    PHP_ADD_LIBRARY_WITH_PATH(proj, $PROJ_DIR/$PHP_LIBDIR, PROJ_SHARED_LIBADD)
    AC_DEFINE(HAVE_PROJ, 1, [Whether you have Proj])
  ], [
    AC_MSG_ERROR([proj library or proj_trans() function in this library not found in $PROJ_DIR. Check config.log for more information.])
  ], [
    -L$PROJ_DIR/$PHP_LIBDIR
  ])

  PHP_NEW_EXTENSION(proj, src/proj.c, $ext_shared)
  PHP_SUBST(PROJ_SHARED_LIBADD)
fi
