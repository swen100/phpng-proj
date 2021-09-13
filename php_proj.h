/*
 * File:   php_proj.hpp
 * Author: Swen Zanon
 *
 */
#include <proj.h>

#ifndef PHP_PROJ_H
#define PHP_PROJ_H 1

#define PHP_PROJ_VERSION "3.0"
#define PHP_PROJ_EXTNAME "proj"
#define PHP_PROJ_RES_NAME "Proj"
#define PHP_PROJ_AREA_RES_NAME "Area"

#ifdef ZTS
#include <TSRM.h>
#endif

PHP_MINIT_FUNCTION(proj);
PHP_MSHUTDOWN_FUNCTION(proj);
PHP_RINIT_FUNCTION(proj);
PHP_MINFO_FUNCTION(proj);

ZEND_FUNCTION(proj_create);
ZEND_FUNCTION(proj_create_crs_to_crs);
ZEND_FUNCTION(proj_create_crs_to_crs_from_pj);
ZEND_FUNCTION(proj4_transform_string);
ZEND_FUNCTION(proj_transform_string);
ZEND_FUNCTION(proj4_transform_array);
ZEND_FUNCTION(proj_transform_array);
ZEND_FUNCTION(proj4_transform_point);
ZEND_FUNCTION(proj_transform_point);
ZEND_FUNCTION(proj_is_latlong);
ZEND_FUNCTION(proj_is_geocent);
ZEND_FUNCTION(proj_get_def);
ZEND_FUNCTION(proj_get_pj_info);
ZEND_FUNCTION(proj_get_errno);
ZEND_FUNCTION(proj_get_errno_string);
ZEND_FUNCTION(proj_get_release);
ZEND_FUNCTION(proj_get_info);
ZEND_FUNCTION(proj_list_units);
ZEND_FUNCTION(proj_list_ellps);
ZEND_FUNCTION(proj_area_create);
ZEND_FUNCTION(proj_area_set_bbox);
ZEND_FUNCTION(proj_free);

extern zend_module_entry proj_module_entry;
#define phpext_proj_ptr &proj_module_entry

#endif


#if PROJ_VERSION_MAJOR < 7
struct PJ_UNITS {
    const char  *id;        // units keyword
    const char  *to_meter;  // multiply by value to get meters
    const char  *name;      // comments
    double      factor;     // to_meter factor in actual numbers
};

struct PJ_ELLPS {
    const char  *id;
    const char  *major;
    const char  *ell;
    const char  *name;
};
#endif
