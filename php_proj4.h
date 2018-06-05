/*
 * File:   php_proj4.hpp
 * Author: Swen Zanon
 *
 * Created on 28. Juli 2016, 11:10
 */

#ifndef PHP_PROJ4_H
#define PHP_PROJ4_H 1

#define PHP_PROJ4_VERSION "2.0"
#define PHP_PROJ4_EXTNAME "proj.4"
#define PHP_PROJ4_RES_NAME "Proj.4"

#ifdef ZTS
#include <TSRM.h>
#endif

PHP_MINIT_FUNCTION(proj4);
PHP_MSHUTDOWN_FUNCTION(proj4);
PHP_RINIT_FUNCTION(proj4);
PHP_MINFO_FUNCTION(proj4);

ZEND_FUNCTION(pj_init_plus);
ZEND_FUNCTION(pj_transform_string);
ZEND_FUNCTION(pj_transform_array);
ZEND_FUNCTION(pj_transform_point);
ZEND_FUNCTION(pj_is_latlong);
ZEND_FUNCTION(pj_is_geocent);
ZEND_FUNCTION(pj_get_def);
ZEND_FUNCTION(pj_get_errno);
ZEND_FUNCTION(pj_errno_string);
ZEND_FUNCTION(pj_get_release);
ZEND_FUNCTION(pj_free);

extern zend_module_entry proj4_module_entry;
#define phpext_proj4_ptr &proj4_module_entry

#endif
