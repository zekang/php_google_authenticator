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
   | Author:                                                              |
   +----------------------------------------------------------------------+
   */

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "Zend/zend_exceptions.h"
#include "php_google_authenticator.h"
#include "ext/hash/php_hash.h"
#include "ext/standard/md5.h"

/* If you declare any globals in php_google_authenticator.h uncomment this:
*/
ZEND_DECLARE_MODULE_GLOBALS(google_authenticator)

		/* True global resources - no need for thread safety here */
static int le_google_authenticator;
typedef unsigned char uchar;

zend_class_entry *google_authenticator_ce;

static inline void php_hash_string_xor_char(uchar *out, const uchar *in, const uchar xor_with, const int length) 
{
		int i;
		for (i=0; i < length; i++) {
				out[i] = in[i] ^ xor_with;
		}
}

static inline void php_hash_string_xor(uchar *out, const uchar *in, const uchar *xor_with, const int length)
{
		int i;
		for (i=0; i < length; i++) {
				out[i] = in[i] ^ xor_with[i];
		}
}

static inline void php_hash_hmac_prep_key(uchar *K, const php_hash_ops *ops, void *context, const uchar *key, const int key_len)
{
		memset(K, 0, ops->block_size);
		if (key_len > ops->block_size) {
				/* Reduce the key first */
				ops->hash_init(context);
				ops->hash_update(context, key, key_len);
				ops->hash_final(K, context);
		} else {
				memcpy(K, key, key_len);
		}
		/* XOR the key with 0x36 to get the ipad) */
		php_hash_string_xor_char(K, K, 0x36, ops->block_size);
}

static inline void php_hash_hmac_round( uchar *final, const php_hash_ops *ops, void *context, const uchar *key, const uchar *data, const long data_size) 
{
		ops->hash_init(context);
		ops->hash_update(context, key, ops->block_size);
		ops->hash_update(context, data, data_size);
		ops->hash_final(final, context);
}

static int hmac_sha1(const char *key,int keyLength,const char *data,int dataLength,char **result TSRMLS_DC)
{
		char *K, *digest;
		const php_hash_ops *ops;
		void *context;

		ops = php_hash_fetch_ops(ZEND_STRL("sha1"));
		context = emalloc(ops->context_size);

		K = emalloc(ops->block_size);
		digest = emalloc(ops->digest_size + 1);
		php_hash_hmac_prep_key((uchar *) K, ops, context, (uchar *) key, keyLength);		
		php_hash_hmac_round((uchar *) digest, ops, context, (uchar *) K, (uchar *) data, dataLength);
		php_hash_string_xor_char((uchar *) K, (uchar *) K, 0x6A, ops->block_size);
		php_hash_hmac_round((uchar *) digest, ops, context, (uchar *) K, (uchar *) digest, ops->digest_size);
		/* Zero the key */
		memset(K, 0, ops->block_size);
		efree(K);
		efree(context);
		digest[ops->digest_size] = 0;
		*result = digest;
		return ops->digest_size;
}


int base32_decode(const uchar *encoded,int length, uchar **return_value TSRMLS_DC) {
		int buffer = 0;
		int bitsLeft = 0;
		int count = 0;
		const uchar *ptr;
		uchar *result ;
		result = (uchar *) safe_emalloc(length,1,1);
		for (ptr = encoded;  *ptr; ++ptr) {
				uchar ch = *ptr;
				if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '-') {
						continue;
				}
				buffer <<= 5;
				if (ch == '0') {
						ch = 'O';
				} else if (ch == '1') {
						ch = 'L';
				} else if (ch == '8') {
						ch = 'B';
				}
				if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
						ch = (ch & 0x1F) - 1;
				} else if (ch >= '2' && ch <= '7') {
						ch -= '2' - 26;
				} else {
						efree(result);
						return -1;
				}
				buffer |= ch;
				bitsLeft += 5;
				if (bitsLeft >= 8) {
						result[count++] = buffer >> (bitsLeft - 8);
						bitsLeft -= 8;
				}
		}
		result[count] = '\000';
		*return_value = result;
		return count;
}

int base32_encode(const uchar *data, int length, uchar **return_value TSRMLS_DC) 
{
		uchar * result;
		if (length < 0 || length > (1 << 28)) {
				return -1;
		}
		int count = 0;
		int bufSize = ((length+4) /5 *8) + 1;
		result = (uchar *) safe_emalloc(bufSize,sizeof(char),1);
		if (length > 0) {
				int buffer = data[0];
				int next = 1;
				int bitsLeft = 8;
				while (count<bufSize && (bitsLeft > 0 || next < length)) {
						if (bitsLeft < 5) {
								if (next < length) {
										buffer <<= 8;
										buffer |= data[next++] & 0xFF;
										bitsLeft += 8;
								} else {
										int pad = 5 - bitsLeft;
										buffer <<= pad;
										bitsLeft += pad;
								}
						}
						int index = 0x1F & (buffer >> (bitsLeft - 5));
						bitsLeft -= 5;
						result[count++] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"[index];
				}
		}
		if (count < bufSize) {
				result[count] = '\000';
		}
		*return_value = result;
		return count;
}

static int generateCode(const char *key,int key_len,ulong tm TSRMLS_DC)
{
	uchar challenge[8],*secret=NULL,*hash=NULL;	
	int i,secret_len,hash_len,offset;
	uint truncatedHash = 0;
	for(i=8;i--;tm >>= 8){
		challenge[i] = tm;
	}
	secret_len = base32_decode(key,key_len,&secret TSRMLS_CC);	
	if(secret_len  < 0){
		zend_throw_exception(NULL,"base32_decode secret key failure!",500);
		return -1;
	}
	hash_len = hmac_sha1(secret,secret_len,challenge,8,(char **)&hash);
	offset = hash[hash_len - 1] & 0xF;
	for ( i = 0; i < 4; ++i) {
			truncatedHash <<= 8;
			truncatedHash  |= hash[offset + i];
	}
	truncatedHash &= 0x7FFFFFFF;
	truncatedHash %= 1000000;
	if(secret){
		efree(secret);
	}
	if(hash){
		efree(hash);
	}
	return truncatedHash;
}
/* {{{ arg_info
 * */
ZEND_BEGIN_ARG_INFO_EX(arg_info_validate, 0, 0, 2)
ZEND_ARG_INFO(0,secretKey)
ZEND_ARG_INFO(0,code)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arg_info_base32_encode, 0, 0, 1)
ZEND_ARG_INFO(0,encodeStr)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arg_info_base32_decode, 0, 0, 1)
ZEND_ARG_INFO(0,decodeStr)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arg_info_generate_secret_key, 0, 0, 1)
ZEND_ARG_INFO(0,token)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arg_info_generate_code, 0, 0, 1)
ZEND_ARG_INFO(0,secretKey)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arg_info_get_QRBarcode, 0, 0, 2)
ZEND_ARG_INFO(0,user)
ZEND_ARG_INFO(0,secretKey)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arg_info_get_QRBarcode_URL, 0, 0, 3)
ZEND_ARG_INFO(0,host)
ZEND_ARG_INFO(0,user)
ZEND_ARG_INFO(0,secretKey)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ google_authenticator_functions[]
 *
 * Every user visible function must have an entry in google_authenticator_functions[].
 */
const zend_function_entry google_authenticator_functions[] = {
		PHP_FE_END	/* Must be the last line in google_authenticator_functions[] */
};
/* }}} */


/*{{{	proto public GoogleAuthenticator::log($secretKey, $code)
 * */
ZEND_METHOD(GoogleAuthenticator,validate)
{
		char *key = NULL;
		int key_len,code,i,window_size,hash;
		ulong t;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &key, &key_len,&code) == FAILURE) {
				return;
		}
		if(key_len < 1 || code < 1){
			RETURN_FALSE;
		}
		t = time(NULL) / 30;
		window_size = GOOGLE_AUTHENTICATOR_G(window_size);
		for( i = -1 * window_size;i <= window_size; i++	){
				hash = generateCode(key,key_len,t + i TSRMLS_CC) ;
				if(hash == -1){
						RETURN_FALSE;
				}
				if (hash == code){
					RETURN_TRUE;
				}
		}
		RETURN_FALSE;
}
/* }}} */

/*{{{	proto public GoogleAuthenticator::base32_encode($encodeStr)
 * */
ZEND_METHOD(GoogleAuthenticator,base32_encode)
{
		char *str = NULL,*tmp;
		int str_len,len;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) == FAILURE) {
				return;
		}
		len = base32_encode(str,str_len,(uchar **)&tmp TSRMLS_CC);
		RETURN_STRINGL(tmp, len, 0);
}
/* }}} */

/*{{{	proto public GoogleAuthenticator::base32_decode($decodeStr)
 * */
ZEND_METHOD(GoogleAuthenticator,base32_decode)
{
		char *str = NULL,*tmp;
		int str_len,len;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) == FAILURE) {
				return;
		}
		len = base32_decode(str,str_len,(uchar **)&tmp TSRMLS_CC);
		if(len < 0){
			RETURN_FALSE;
		}
		RETURN_STRINGL(tmp, len, 0);
}
/* }}} */


/*{{{	proto public GoogleAuthenticator::generateSecretKey($token=NULL)
 * */
ZEND_METHOD(GoogleAuthenticator,generateSecretKey)
{
		char *str = NULL,*tmp;
		int str_len = 0 ,len;
		int secret_size = 10;
		PHP_MD5_CTX context;
		uchar digest[16];
		char buf[32]={0};
		struct timeval tp ={0};
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &str, &str_len) == FAILURE) {
				return;
		}
		if(str_len ==0){
			if(gettimeofday(&tp,NULL)){
				RETURN_FALSE;
			}
			str_len = snprintf(buf,sizeof(buf),"%f",(double)tp.tv_sec+ tp.tv_usec / 1000000.00);
			str = buf;
		}
		PHP_MD5Init(&context);
		PHP_MD5Update(&context, str, str_len);
		PHP_MD5Final(digest, &context);
		len = base32_encode(digest,secret_size,(uchar **)&tmp TSRMLS_CC);
		RETURN_STRINGL(tmp, len, 0);
}
/* }}} */

/*{{{	proto public GoogleAuthenticator::generateCode($secretKey)
 * */
ZEND_METHOD(GoogleAuthenticator,generateCode)
{
		char *key = NULL;
		int key_len;
		ulong t;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
				return;
		}
		if(key_len < 1){
			RETURN_FALSE;
		}
		t = time(NULL) / 30;
		RETURN_LONG(generateCode(key,key_len,t TSRMLS_CC));
}
/* }}} */

/*{{{	proto public GoogleAuthenticator::getQRBarcode($user,$secretKey)
 * */
ZEND_METHOD(GoogleAuthenticator,getQRBarcode)
{
		char *user,*key ,*ret;
		int user_len,key_len,ret_len;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",&user,&user_len,&key, &key_len) == FAILURE) {
				return;
		}
		ret_len = spprintf(&ret,0,"otpauth://totp/%s?secret=%s",user,key);
		RETURN_STRINGL(ret, ret_len, 0);
}
/* }}} */

/*{{{	proto public GoogleAuthenticator::getQRBarcodeURL($host,$user,$secretKey)
 * */
ZEND_METHOD(GoogleAuthenticator,getQRBarcodeURL)
{
		char *host,*user,*key ,*ret;
		int host_len,user_len,key_len,ret_len;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss",&host,&host_len,&user,&user_len,&key, &key_len) == FAILURE) {
				return;
		}
		ret_len = spprintf(&ret,0,"http://www.google.com/chart?chs=200x200&chld=M%%7C0&cht=qr&chl=otpauth://totp/%s@%s?secret=%s",user,host,key);
		RETURN_STRINGL(ret, ret_len, 0);
}
/* }}} */



/* {{{ google_authenticator_methods[]
 */
const zend_function_entry google_authenticator_methods[] = {
	ZEND_ME(GoogleAuthenticator,validate,arg_info_validate,ZEND_ACC_PUBLIC)
	ZEND_ME(GoogleAuthenticator,base32_encode,arg_info_base32_encode,ZEND_ACC_PUBLIC)
	ZEND_ME(GoogleAuthenticator,base32_decode,arg_info_base32_decode,ZEND_ACC_PUBLIC)
	ZEND_ME(GoogleAuthenticator,generateSecretKey,arg_info_generate_secret_key,ZEND_ACC_PUBLIC)
	ZEND_ME(GoogleAuthenticator,generateCode,arg_info_generate_code,ZEND_ACC_PUBLIC)
	ZEND_ME(GoogleAuthenticator,getQRBarcode,arg_info_get_QRBarcode,ZEND_ACC_PUBLIC)
	ZEND_ME(GoogleAuthenticator,getQRBarcodeURL,arg_info_get_QRBarcode_URL,ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/* }}} */

/* {{{ google_authenticator_module_entry
*/
zend_module_entry google_authenticator_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
		STANDARD_MODULE_HEADER,
#endif
		"google_authenticator",
		google_authenticator_functions,
		PHP_MINIT(google_authenticator),
		PHP_MSHUTDOWN(google_authenticator),
		PHP_RINIT(google_authenticator),		/* Replace with NULL if there's nothing to do at request start */
		PHP_RSHUTDOWN(google_authenticator),	/* Replace with NULL if there's nothing to do at request end */
		PHP_MINFO(google_authenticator),
#if ZEND_MODULE_API_NO >= 20010901
		PHP_GOOGLE_AUTHENTICATOR_VERSION,
#endif
		STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_GOOGLE_AUTHENTICATOR
ZEND_GET_MODULE(google_authenticator)
#endif

static PHP_INI_MH(OnUpdateWindowSize)
{
	int default_value = 3;
	int tmp = 0;
	if(new_value){
			tmp = atoi(new_value);		
			if(tmp < 1 ){
				default_value = 3;
			}else if(tmp > 100){
				default_value = 100;
			}
	}
	GOOGLE_AUTHENTICATOR_G(window_size) = default_value;
	return SUCCESS;
}
/* {{{ PHP_INI */
PHP_INI_BEGIN()
STD_PHP_INI_ENTRY("google_authenticator.window_size","3", PHP_INI_ALL, OnUpdateWindowSize, window_size, zend_google_authenticator_globals, google_authenticator_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_google_authenticator_init_globals */
static void php_google_authenticator_init_globals(zend_google_authenticator_globals *google_authenticator_globals)
{
		google_authenticator_globals->window_size  = 3;
}
/* }}} */
/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(google_authenticator)
{
		zend_class_entry ce;
		INIT_CLASS_ENTRY(ce,"GoogleAuthenticator",google_authenticator_methods);
		google_authenticator_ce = zend_register_internal_class(&ce TSRMLS_CC);
		ZEND_INIT_MODULE_GLOBALS(google_authenticator,php_google_authenticator_init_globals,NULL);
		REGISTER_INI_ENTRIES();
		return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(google_authenticator)
{
		UNREGISTER_INI_ENTRIES();
		return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
*/
PHP_RINIT_FUNCTION(google_authenticator)
{
		return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
*/
PHP_RSHUTDOWN_FUNCTION(google_authenticator)
{
		return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(google_authenticator)
{
		php_info_print_table_start();
		php_info_print_table_header(2, "google_authenticator support", "enabled");
		php_info_print_table_end();
		DISPLAY_INI_ENTRIES();
}
/* }}} */



/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
   */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
