PHP_ARG_ENABLE(proj, whether to enable Proj support,
[ --enable-proj   Enable Proj support])

if test "$PHP_PROJ" = "yes"; then
  PHP_ADD_LIBPATH(/usr/lib)
  AC_DEFINE(HAVE_PROJ, 1, [Whether you have Proj])
  PHP_NEW_EXTENSION(proj, proj.c, $ext_shared)
  PHP_SUBST(PROJ_SHARED_LIBADD)
  PHP_ADD_LIBRARY(proj, 1, PROJ_SHARED_LIBADD)
  PHP_CHECK_LIBRARY(proj, proj_trans, [], [AC_MSG_ERROR([proj library or proj_trans() function in this library not found. Check config.log for more information.])])
fi
