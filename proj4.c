#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <php_main.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <zend_types.h>
#include <zend_string.h>
#include <ext/standard/php_string.h>
#include <proj.h>
#include <php_proj4.h>

int le_proj4;

static zend_function_entry proj4_functions[] = {
    ZEND_FE(pj_init_plus, NULL)
    ZEND_FE(pj_transform_string, NULL)
    ZEND_FE(pj_transform_array, NULL)
    ZEND_FE(pj_transform_point, NULL)
    ZEND_FE(pj_is_latlong, NULL)
    ZEND_FE(pj_is_geocent, NULL)
    ZEND_FE(pj_get_def, NULL)
    ZEND_FE(pj_errno_string, NULL)
    ZEND_FE(pj_get_errno, NULL)
    ZEND_FE(pj_get_release, NULL)
    ZEND_FE(pj_free, NULL) {
        NULL, NULL, NULL
    }
};

zend_module_entry proj4_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_PROJ4_EXTNAME,
    proj4_functions,
    PHP_MINIT(proj4),           /* module init function */
    PHP_MSHUTDOWN(proj4),       /* module shutdown function */
    PHP_RINIT(proj4),           /* request init function */
    NULL,                       /* request shutdown function, would be PHP_RSHUTDOWN(proj4) */
    PHP_MINFO(proj4),           /* module info function */
    PHP_PROJ4_VERSION,
    //PHP_MODULE_GLOBALS(geos),     /* globals descriptor */
    //PHP_GINIT(geos),              /* globals ctor */
    //NULL,                         /* globals dtor */
    //NULL,                         /* post deactivate */
    STANDARD_MODULE_PROPERTIES /* or STANDARD_MODULE_PROPERTIES_EX if above used */
};

static void php_proj4_dtor(zend_resource *resource TSRMLS_DC) {
    PJ *proj = (PJ*) resource->ptr;
    if (proj != NULL && proj) {
        proj_destroy(proj);
    }
}

PHP_RINIT_FUNCTION(proj4) {
    return SUCCESS;
}

PHP_MINIT_FUNCTION(proj4) {
    le_proj4 = zend_register_list_destructors_ex(php_proj4_dtor, NULL, PHP_PROJ4_RES_NAME, module_number);
    return SUCCESS;
}

PHP_MINFO_FUNCTION(proj4) {
    php_info_print_table_start();
    php_info_print_table_header(2, "proj.4 module", "enabled");
    php_info_print_table_row(2, "Proj4 EXT version", PHP_PROJ4_VERSION);
    php_info_print_table_row(2, "Author", "Swen Zanon");
    php_info_print_table_row(2, "Powered by", "geoGLIS oHG");
    php_info_print_table_row(2, "Proj.4 release", proj_info().release);
    php_info_print_table_row(2, "Proj.4 paths", proj_info().paths);
    php_info_print_table_row(2, "Proj.4 searchpath", proj_info().searchpath);
    php_info_print_table_end();
}

PHP_MSHUTDOWN_FUNCTION(proj4) {
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}

#ifdef COMPILE_DL_PROJ4

ZEND_GET_MODULE(proj4)
#endif


/*###########################################################################
  Internal static Functions
  ###########################################################################
 */
/*
static void tellMeWhatYouAre(zval *arg) {
    zval *zv;

    php_printf("\nThe Element is of type: ");
    switch (Z_TYPE_P(arg)) {
        case IS_NULL:
            php_printf("NULL");
            break;
        case IS_TRUE:
        case IS_FALSE:
            php_printf("Boolean: %s", Z_LVAL_P(arg) ? "TRUE" : "FALSE");
            break;
        case IS_LONG:
            php_printf("Long: %ld", Z_LVAL_P(arg));
            break;
        case IS_DOUBLE:
            php_printf("Double: %f", Z_DVAL_P(arg));
            break;
        case IS_STRING:
            //php_printf("String: ");
            //PHPWRITE(Z_STRVAL_P(arg), Z_STRLEN_P(arg));
            php_printf("String: %s", Z_STRVAL_P(arg));
            php_printf("");
            break;
        case IS_ARRAY:
            php_printf("Array, with this content:\n");
            HashTable *hash = Z_ARR_P(arg);

            ZEND_HASH_FOREACH_VAL(hash, zv) {
                tellMeWhatYouAre(zv);
            }
            ZEND_HASH_FOREACH_END();
            break;
        default:
            php_printf("Unknown");
    }
    php_printf("\n");
}
 */
static zval projCoord_static(PJ *srcProj, PJ *tgtProj, double x, double y, double z)
{
    PJ_COORD a, b;
    zval return_value;

    // source is already lat/lon -> deg2rad
    if (proj_angular_output(srcProj, PJ_FWD) == 1) {
        x = x > 180 ? 180 : (x < -180 ? -180 : x);
        y = y > 90 ? 89.9999999 : (y < -90 ? -89.9999999 : y);
        a = proj_coord (proj_torad(x), proj_torad(y), z, 0);
    }
    // transform source-coord to lat/lon
    else {
        a = proj_coord (x, y, z, 0);
        a = proj_trans(srcProj, PJ_INV, a);
    }
    //printf ("longitude: %g, latitude: %g\n", a.lp.lam, a.lp.phi);
    
    // transform to target-projection
    b = proj_trans(tgtProj, PJ_FWD, a);
    //printf ("easting: %g, northing: %g\n", b.enu.e, b.enu.n);
    
    if (proj_errno(tgtProj) == 0) {
        // rad2deg
        if (proj_angular_output(tgtProj, PJ_FWD) == 1) {
            b.xyz.x = proj_todeg(b.xyz.x);
            b.xyz.y = proj_todeg(b.xyz.y);
        }

        array_init(&return_value);
        add_assoc_double(&return_value, "x", b.xyz.x);
        add_assoc_double(&return_value, "y", b.xyz.y);
        add_assoc_double(&return_value, "z", b.xyz.z);
    }

    return return_value;
}

/*static zval projCoordViaWGS84_static(projPJ srcProj, projPJ tgtProj, projPJ wgsProj, double x, double y, double z)
{
    int p;
    zval return_value;

    pj_transform(srcProj, wgsProj, 1, 0, &x, &y, &z);
    x *= RAD_TO_DEG;
    y *= RAD_TO_DEG;
    srcProj = wgsProj;

    // deg2rad
    if (pj_is_latlong(srcProj) == 1) {
        x *= DEG_TO_RAD;
        y *= DEG_TO_RAD;
    }

    p = pj_transform(srcProj, tgtProj, 1, 0, &x, &y, &z);

    if (p = 1) {
        // rad2deg
        if (pj_is_latlong(tgtProj) == 1) {
            x *= RAD_TO_DEG;
            y *= RAD_TO_DEG;
        }

        array_init(&return_value);
        add_assoc_double(&return_value, "x", x);
        add_assoc_double(&return_value, "y", y);
        add_assoc_double(&return_value, "z", z);
    }

    return return_value;
}*/

static zval transformCoordArray_static(PJ *srcProj, PJ *tgtProj, zval xy_arr)
{
    zval *x, *y, z, *t, empty_arr;
    zval coord;
    HashTable *xyz_hash = Z_ARR_P(&xy_arr);

    if (NULL != (x = zend_hash_index_find(xyz_hash, 0)) &&
            NULL != (y = zend_hash_index_find(xyz_hash, 1))) {

        if (NULL == (t = zend_hash_index_find(xyz_hash, 2))) {
            ZVAL_DOUBLE(&z, 0.0);
        } else {
            ZVAL_COPY_VALUE(&z, t);
            zval_dtor(t);
            convert_to_double_ex(&z);
        }

        convert_to_double_ex(x);
        convert_to_double_ex(y);

        coord = projCoord_static(srcProj, tgtProj, Z_DVAL_P(x), Z_DVAL_P(y), Z_DVAL(z));
        zval_dtor(x);
        zval_dtor(y);
        return coord;
    }

    array_init(&empty_arr);
    add_assoc_double(&empty_arr, "x", 0);
    add_assoc_double(&empty_arr, "y", 0);
    add_assoc_double(&empty_arr, "z", 0);
    return empty_arr;
}

/*###########################################################################
  API Functions
  ###########################################################################
 */

/**
 * 
 * @param string projection-definition
 * @return resource
 */
ZEND_FUNCTION(pj_init_plus)
{
    //PJ_CONTEXT *C;
    PJ *P;
    char *definition;
    size_t definition_length;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &definition, &definition_length) == FAILURE) {
        RETURN_FALSE;
    }

    //C = proj_context_create();
    P = proj_create(0, definition);
    
    if (0==P) {
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(P, le_proj4));
}

/**
 * 
 * @return void
 */
ZEND_FUNCTION(pj_free)
{
    zval *zpj;
    PJ *P;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zpj) == FAILURE) {
        RETURN_FALSE;
    }

    // @Todo: Prüfen - wofür dieser Aufruf? P wird ja nicht benutzt...
    P = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ4_RES_NAME, le_proj4);

    zend_list_delete(Z_RES_P(zpj));
}

/**
 * 
 * @param resource srcDefn
 * @param resource tgtDefn
 * @param float x
 * @param float y
 * @param float z
 * @return mixed array ('x' => x, 'y' => y 'z' => z) or false on error
 */
ZEND_FUNCTION(pj_transform_point) {

    double x, y, z = 0;
    zval *srcDefn, *tgtDefn;
    PJ *srcProj, *tgtProj;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rrdd|d", &srcDefn, &tgtDefn, &x, &y, &z) == FAILURE) {
        RETURN_FALSE;
    }

    srcProj = (PJ*) zend_fetch_resource_ex(srcDefn, PHP_PROJ4_RES_NAME, le_proj4);
    tgtProj = (PJ*) zend_fetch_resource_ex(tgtDefn, PHP_PROJ4_RES_NAME, le_proj4);

    if (srcProj == NULL || tgtProj == NULL) {
        RETURN_FALSE;
    }

    *return_value = projCoord_static(srcProj, tgtProj, x, y, z);
}

/**
 * 
 * @param resource srcDefn
 * @param resource tgtDefn
 * @param array points-array
 * @return mixed array ('x' => x, 'y' => y 'z' => z) or false on error
 */
ZEND_FUNCTION(pj_transform_array) {

    /* method-params */
    zval xyz_arr, *zv, coord;
    zend_string *delimiter, *trimmed_point_string;

    /* user-params */
    zval *srcDefn, *tgtDefn, *xyz_arr_p;

    /* projection-params */
    PJ *srcProj, *tgtProj;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rra", &srcDefn, &tgtDefn, &xyz_arr_p) == FAILURE) {
        RETURN_FALSE;
    }

    srcProj = (PJ*) zend_fetch_resource_ex(srcDefn, PHP_PROJ4_RES_NAME, le_proj4);
    tgtProj = (PJ*) zend_fetch_resource_ex(tgtDefn, PHP_PROJ4_RES_NAME, le_proj4);

    if (srcProj == NULL || tgtProj == NULL) {
        RETURN_FALSE;
    }

    array_init(return_value);
    delimiter = zend_string_init(" ", 1, 0);
    HashTable *pts_hash = Z_ARR_P(xyz_arr_p);

    ZEND_HASH_FOREACH_VAL(pts_hash, zv) {

        // in x,y,z-Array zerteilen
        if (Z_TYPE_P(zv) != IS_ARRAY) {
            convert_to_string_ex(zv);
            array_init(&xyz_arr);
            trimmed_point_string = php_trim(Z_STR_P(zv), NULL, 0, 3);
            php_explode(delimiter, trimmed_point_string, &xyz_arr, LONG_MAX);
            coord = transformCoordArray_static(srcProj, tgtProj, xyz_arr);
            zval_ptr_dtor(zv);
            zval_ptr_dtor(&xyz_arr);
        } else {
            //ZVAL_COPY_VALUE(&xyz_arr, zv);
            coord = transformCoordArray_static(srcProj, tgtProj, *zv);
        }

        add_next_index_zval(return_value, &coord);


    }
    ZEND_HASH_FOREACH_END();

    // cleanup
    zend_string_release(delimiter);
}

/**
 * @param resource srcDefn
 * @param resource tgtDefn
 * @param string geometry
 * @return mixed array ('x' => x, 'y' => y 'z' => z) or false on error
 */
ZEND_FUNCTION(pj_transform_string) {

    /* user-params */
    zval *srcDefn, *tgtDefn;
    zend_string *str, *xyz_string;

    /* projection-params */
    PJ *srcProj, *tgtProj;

    /* coord-params */
    zval pts_arr, xyz_arr;
    zend_string *delimiter, *delimiter2, *trimmed_geom_string, *trimmed_point_string;
    HashTable *pts_hash;
    zval coord;
    zval *zv;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rrS", &srcDefn, &tgtDefn, &str) == FAILURE) {
        RETURN_FALSE;
    }

    srcProj = (PJ*) zend_fetch_resource_ex(srcDefn, PHP_PROJ4_RES_NAME, le_proj4);
    tgtProj = (PJ*) zend_fetch_resource_ex(tgtDefn, PHP_PROJ4_RES_NAME, le_proj4);

    if (srcProj == NULL || tgtProj == NULL) {
        RETURN_FALSE;
    }

    array_init(return_value);

    delimiter = zend_string_init(",", 1, 0);
    delimiter2 = zend_string_init(" ", 1, 0);

    // in einzelne Koordinaten zerteilen
    array_init(&pts_arr);
    trimmed_geom_string = php_trim(str, NULL, 0, 3);
    php_explode(delimiter, trimmed_geom_string, &pts_arr, LONG_MAX);

    if (Z_TYPE(pts_arr) == IS_ARRAY) {
        pts_hash = Z_ARR_P(&pts_arr);

        ZEND_HASH_FOREACH_VAL(pts_hash, zv) {
            //            convert_to_string_ex(zv);
            xyz_string = zend_string_dup(Z_STR_P(zv), 0);

            // in x,y,z zerteilen
            array_init(&xyz_arr);
            trimmed_point_string = php_trim(xyz_string, NULL, 0, 3);

            php_explode(delimiter2, trimmed_point_string, &xyz_arr, LONG_MAX);

            coord = transformCoordArray_static(srcProj, tgtProj, xyz_arr);
            add_next_index_zval(return_value, &coord);

            // cleanup
            //            zval_ptr_dtor(zv);
            zend_string_release(xyz_string);
            zend_string_release(trimmed_point_string);
            zval_ptr_dtor(&xyz_arr);
        }
        ZEND_HASH_FOREACH_END();
    }

    // cleanup
    zval_ptr_dtor(&pts_arr);
    zend_string_release(delimiter);
    zend_string_release(delimiter2);
    zend_string_release(trimmed_geom_string);
}


/*###########################################################################
  Advanced Functions
  ###########################################################################
 */

/*
 * @param resource projection
 * @return boolean
 */
ZEND_FUNCTION(pj_is_latlong)
{
    zval *zpj;
    PJ *pj;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zpj) == FAILURE) {
        RETURN_FALSE;
    }

    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ4_RES_NAME, le_proj4);

    if (proj_angular_output(pj, PJ_FWD) == 1) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

/**
 * 
 * @param resource projection
 * @return boolean
 */
ZEND_FUNCTION(pj_is_geocent)
{
    zval *zpj;
    PJ *pj;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zpj) == FAILURE) {
        RETURN_FALSE;
    }

    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ4_RES_NAME, le_proj4);

    if (proj_angular_output(pj, PJ_FWD) == 0) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

/**
 * 
 * @param resource projection
 * @param long
 * @return mixed string or false on error
 */
ZEND_FUNCTION(pj_get_def)
{
    PJ_PROJ_INFO result;
    zval *zpj;
    PJ *pj;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zpj) == FAILURE) {
        RETURN_FALSE;
    }

    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ4_RES_NAME, le_proj4);

    if (pj != NULL && pj) {
        result = proj_pj_info(pj);

        RETURN_STRING(result.definition);
    }

    RETURN_FALSE;
}

/*###########################################################################
  Environment Functions
  ###########################################################################
 */

/**
 * 
 * @return numeric
 */
ZEND_FUNCTION(pj_get_errno)
{
    zval *zpj;
    PJ *pj;
    zend_long error_code;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zpj) == FAILURE) {
        RETURN_FALSE;
    }
    
    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ4_RES_NAME, le_proj4);
    
    if (pj != NULL && pj) {
        error_code = proj_errno(pj);
        RETURN_LONG(error_code);
    }
    
    RETURN_NULL();
}

/**
 * 
 * @param integer error-code
 * @return string error-message
 */
ZEND_FUNCTION(pj_errno_string)
{
    zend_long error_code;
    const char *result;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &error_code) == FAILURE) {
        RETURN_FALSE;
    }

    result = proj_errno_string(error_code);

    RETURN_STRING(result);
}

/**
 * 
 * @return string
 */
ZEND_FUNCTION(pj_get_release)
{
    RETURN_STRING( proj_info().release );
}

