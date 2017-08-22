/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: wanghouqian <whq654321@126.com>                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_GOOGLE_AUTHENTICATOR_H
#define PHP_GOOGLE_AUTHENTICATOR_H

extern zend_module_entry google_authenticator_module_entry;
#define phpext_google_authenticator_ptr &google_authenticator_module_entry

#define PHP_GOOGLE_AUTHENTICATOR_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_GOOGLE_AUTHENTICATOR_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_GOOGLE_AUTHENTICATOR_API __attribute__ ((visibility("default")))
#else
#	define PHP_GOOGLE_AUTHENTICATOR_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(google_authenticator);
PHP_MSHUTDOWN_FUNCTION(google_authenticator);
PHP_RINIT_FUNCTION(google_authenticator);
PHP_RSHUTDOWN_FUNCTION(google_authenticator);
PHP_MINFO_FUNCTION(google_authenticator);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     
*/
ZEND_BEGIN_MODULE_GLOBALS(google_authenticator)
	long  window_size;
ZEND_END_MODULE_GLOBALS(google_authenticator)

/* In every utility function you add that needs to use variables 
   in php_google_authenticator_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as GOOGLE_AUTHENTICATOR_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define GOOGLE_AUTHENTICATOR_G(v) TSRMG(google_authenticator_globals_id, zend_google_authenticator_globals *, v)
#else
#define GOOGLE_AUTHENTICATOR_G(v) (google_authenticator_globals.v)
#endif

#ifndef PHP_FE_END 
#define ZEND_FE_END            { NULL, NULL, NULL, 0, 0 }
#define PHP_FE_END ZEND_FE_END
#endif

#endif	/* PHP_GOOGLE_AUTHENTICATOR_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
