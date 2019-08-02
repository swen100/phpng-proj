/*
 * File:   php_proj4.hpp
 * Author: Swen Zanon
 *
 * Created on 28. Juli 2016, 11:10
 */

#ifndef PHP_PROJ4_H
#define PHP_PROJ4_H 1

#define PHP_PROJ4_VERSION "3.0"
#define PHP_PROJ4_EXTNAME "proj"
#define PHP_PROJ4_RES_NAME "Proj"

#ifdef ZTS
#include <TSRM.h>
#endif

PHP_MINIT_FUNCTION(proj4);
PHP_MSHUTDOWN_FUNCTION(proj4);
PHP_RINIT_FUNCTION(proj4);
PHP_MINFO_FUNCTION(proj4);

ZEND_FUNCTION(proj_create);
ZEND_FUNCTION(proj_create_crs_to_crs);
ZEND_FUNCTION(proj_transform_string);
ZEND_FUNCTION(proj_transform_array);
ZEND_FUNCTION(proj6_transform_array);
ZEND_FUNCTION(proj_transform_point);
ZEND_FUNCTION(proj6_transform_point);
ZEND_FUNCTION(proj_is_latlong);
ZEND_FUNCTION(proj_is_geocent);
ZEND_FUNCTION(proj_get_def);
ZEND_FUNCTION(proj_get_errno);
ZEND_FUNCTION(proj_get_errno_string);
ZEND_FUNCTION(proj_get_release);
ZEND_FUNCTION(proj_get_info);
ZEND_FUNCTION(proj_free);

extern zend_module_entry proj4_module_entry;
#define phpext_proj4_ptr &proj4_module_entry

#endif
