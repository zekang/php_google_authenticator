dnl $Id$
dnl config.m4 for extension google_authenticator

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(google_authenticator, for google_authenticator support,
dnl Make sure that the comment is aligned:
dnl [  --with-google_authenticator             Include google_authenticator support])

dnl Otherwise use enable:

 PHP_ARG_ENABLE(google_authenticator, whether to enable google_authenticator support,
 Make sure that the comment is aligned:
 [  --enable-google_authenticator           Enable google_authenticator support])

if test "$PHP_GOOGLE_AUTHENTICATOR" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-google_authenticator -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/google_authenticator.h"  # you most likely want to change this
  dnl if test -r $PHP_GOOGLE_AUTHENTICATOR/$SEARCH_FOR; then # path given as parameter
  dnl   GOOGLE_AUTHENTICATOR_DIR=$PHP_GOOGLE_AUTHENTICATOR
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for google_authenticator files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       GOOGLE_AUTHENTICATOR_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$GOOGLE_AUTHENTICATOR_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the google_authenticator distribution])
  dnl fi

  dnl # --with-google_authenticator -> add include path
  dnl PHP_ADD_INCLUDE($GOOGLE_AUTHENTICATOR_DIR/include)

  dnl # --with-google_authenticator -> check for lib and symbol presence
  dnl LIBNAME=google_authenticator # you may want to change this
  dnl LIBSYMBOL=google_authenticator # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $GOOGLE_AUTHENTICATOR_DIR/$PHP_LIBDIR, GOOGLE_AUTHENTICATOR_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_GOOGLE_AUTHENTICATORLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong google_authenticator lib version or lib not found])
  dnl ],[
  dnl   -L$GOOGLE_AUTHENTICATOR_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(GOOGLE_AUTHENTICATOR_SHARED_LIBADD)

  PHP_NEW_EXTENSION(google_authenticator, google_authenticator.c, $ext_shared)
fi
