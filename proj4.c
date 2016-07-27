#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <php_main.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <zend_types.h>
#include <ext/standard/php_string.h>
#include <proj_api.h>
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
    ZEND_FE(pj_latlong_from_proj, NULL)
    ZEND_FE(pj_deallocate_grids, NULL)
    ZEND_FE(pj_strerrno, NULL)
    ZEND_FE(pj_get_errno_ref, NULL)
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
    projPJ *proj = (projPJ*) resource->ptr;
    if (proj != NULL && proj) {
        pj_free(proj);
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
    php_info_print_table_row(2, "proj.4 version", pj_get_release());
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
}*/

static zval projCoord_static(projPJ srcProj, projPJ tgtProj, double x, double y, double z) {

    int p;
    zval return_value;

    // deg2rad
    if (pj_is_latlong(srcProj) == 1) {
        x *= DEG_TO_RAD;
        y *= DEG_TO_RAD;
    }

    //    php_printf("x: %f \n", x);
    //    php_printf("y: %f \n", y);
    //    php_printf("z: %f \n", z);

    p = pj_transform(srcProj, tgtProj, 1, 0, &x, &y, &z);

    if (p = 1) {
        // rad2deg
        if (pj_is_latlong(tgtProj) == 1) {
            x *= RAD_TO_DEG;
            y *= RAD_TO_DEG;
        }

        array_init(&return_value);
//        add_index_double(&return_value, 0, x);
//        add_index_double(&return_value, 1, y);
//        add_index_double(&return_value, 2, z);
        add_assoc_double(&return_value, "x", x);
        add_assoc_double(&return_value, "y", y);
        add_assoc_double(&return_value, "z", z);
    }

    return return_value;
}

static zval projCoordViaWGS84_static(projPJ srcProj, projPJ tgtProj, projPJ wgsProj, double x, double y, double z) {

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

    //    php_printf("x: %f \n", x);
    //    php_printf("y: %f \n", y);
    //    php_printf("z: %f \n", z);

    p = pj_transform(srcProj, tgtProj, 1, 0, &x, &y, &z);

    if (p = 1) {
        // rad2deg
        if (pj_is_latlong(tgtProj) == 1) {
            x *= RAD_TO_DEG;
            y *= RAD_TO_DEG;
        }

        array_init(&return_value);
//        add_index_double(&return_value, 0, x);
//        add_index_double(&return_value, 1, y);
//        add_index_double(&return_value, 2, z);
        add_assoc_double(&return_value, "x", x);
        add_assoc_double(&return_value, "y", y);
        add_assoc_double(&return_value, "z", z);
    }

    return return_value;
}

static zval transformCoordArray_static(projPJ srcProj, projPJ tgtProj, zval xy_arr) {
    zval *x, *y, z, *t, empty_arr;
    //projPJ wgsProj;
    HashTable *xyz_hash = Z_ARR_P(&xy_arr);

    if (NULL != (x = zend_hash_index_find(xyz_hash, 0)) &&
            NULL != (y = zend_hash_index_find(xyz_hash, 1))) {

        if (NULL == (t = zend_hash_index_find(xyz_hash, 2))) {
            //php_printf("no value for z found \n");
            ZVAL_DOUBLE(&z, 0.0);
            //tellMeWhatYouAre(&z);
        } else {
            //php_printf("value for z found \n");
            ZVAL_COPY_VALUE(&z, t);
            zval_dtor(t);
            convert_to_double_ex(&z);
            //tellMeWhatYouAre(&z);
        }

        convert_to_double_ex(x);
        convert_to_double_ex(y);

        /*
         * make sure to go over WGS84 for all transformation between non-geographic coordinate systems
         */
        if (!pj_is_latlong(srcProj) && !pj_is_latlong(tgtProj)) {
            //wgsProj = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
            return projCoordViaWGS84_static(srcProj, tgtProj, pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"), Z_DVAL_P(x), Z_DVAL_P(y), Z_DVAL(z));
            //pj_free(wgsProj);
//            zval_dtor(x);
//            zval_dtor(y);
//            return coord;
        } else {
            return projCoord_static(srcProj, tgtProj, Z_DVAL_P(x), Z_DVAL_P(y), Z_DVAL(z));
//            zval_dtor(x);
//            zval_dtor(y);
//            return coord;
        }
    }

    array_init(&empty_arr);
//    add_index_double(&empty_arr, 0, 0);
//    add_index_double(&empty_arr, 1, 0);
//    add_index_double(&empty_arr, 2, 0);
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
ZEND_FUNCTION(pj_init_plus) {
    projPJ pj_merc;
    char *definition;
    size_t definition_length;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &definition, &definition_length) == FAILURE) {
        RETURN_FALSE;
    }

    pj_merc = pj_init_plus(definition);
    if (pj_merc == NULL) {
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(pj_merc, le_proj4));
}

/**
 * 
 * @return void
 */
ZEND_FUNCTION(pj_free) {
    zval *zpj;
    projPJ pj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zpj) == FAILURE) {
        RETURN_FALSE;
    }

    //ZEND_FETCH_RESOURCE(pj, projPJ*, &zpj, -1, PHP_PROJ4_RES_NAME, le_proj4);
    pj = (projPJ*) zend_fetch_resource_ex(zpj, PHP_PROJ4_RES_NAME, le_proj4);

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
    projPJ srcProj, tgtProj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrdd|d", &srcDefn, &tgtDefn, &x, &y, &z) == FAILURE) {
        RETURN_FALSE;
    }

    srcProj = (projPJ*) zend_fetch_resource_ex(srcDefn, PHP_PROJ4_RES_NAME, le_proj4);
    tgtProj = (projPJ*) zend_fetch_resource_ex(tgtDefn, PHP_PROJ4_RES_NAME, le_proj4);

    if (srcProj == NULL || tgtProj == NULL) {
        RETURN_FALSE;
    }

    if (!pj_is_latlong(srcProj) && !pj_is_latlong(tgtProj)) {
        *return_value = projCoordViaWGS84_static(srcProj, tgtProj, pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"), x, y, z);
    } else {
        *return_value = projCoord_static(srcProj, tgtProj, x, y, z);
    }
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
    projPJ srcProj, tgtProj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rra", &srcDefn, &tgtDefn, &xyz_arr_p) == FAILURE) {
        RETURN_FALSE;
    }

    srcProj = (projPJ*) zend_fetch_resource_ex(srcDefn, PHP_PROJ4_RES_NAME, le_proj4);
    tgtProj = (projPJ*) zend_fetch_resource_ex(tgtDefn, PHP_PROJ4_RES_NAME, le_proj4);

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
    zend_string *str;

    /* projection-params */
    projPJ srcProj, tgtProj;

    /* coord-params */
    zval pts_arr, xyz_arr;
    zend_string *delimiter, *delimiter2, *trimmed_geom_string, *trimmed_point_string;
    HashTable *pts_hash;
    zval coord;
    zval *zv;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrS", &srcDefn, &tgtDefn, &str) == FAILURE) {
        RETURN_FALSE;
    }

    srcProj = (projPJ*) zend_fetch_resource_ex(srcDefn, PHP_PROJ4_RES_NAME, le_proj4);
    tgtProj = (projPJ*) zend_fetch_resource_ex(tgtDefn, PHP_PROJ4_RES_NAME, le_proj4);

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
            
            convert_to_string_ex(zv);
            
            // in x,y,z zerteilen
            array_init(&xyz_arr);
            trimmed_point_string = php_trim(Z_STR_P(zv), NULL, 0, 3);
            php_explode(delimiter2, trimmed_point_string, &xyz_arr, LONG_MAX);
            
            coord = transformCoordArray_static(srcProj, tgtProj, xyz_arr);
            add_next_index_zval(return_value, &coord);
            
            zval_ptr_dtor(zv);
            zval_ptr_dtor(&xyz_arr);
        }
        ZEND_HASH_FOREACH_END();
    }

    // cleanup
    zend_string_release(delimiter);
    zend_string_release(delimiter2);
    //zend_hash_clean(pts_hash);
    zval_ptr_dtor(&pts_arr);
    zend_string_release(trimmed_geom_string);
    zend_string_release(trimmed_point_string);
}


/*###########################################################################
  Advanced Functions
  ###########################################################################
*/

/*
 * @param resource projection
 * @return boolean
 */
ZEND_FUNCTION(pj_is_latlong) {
    zval *zpj;
    projPJ pj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zpj) == FAILURE) {
        RETURN_FALSE;
    }

    pj = (projPJ*) zend_fetch_resource_ex(zpj, PHP_PROJ4_RES_NAME, le_proj4);

    if (pj_is_latlong(pj)) {
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
ZEND_FUNCTION(pj_is_geocent) {
    zval *zpj;
    projPJ pj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zpj) == FAILURE) {
        RETURN_FALSE;
    }

    pj = (projPJ*) zend_fetch_resource_ex(zpj, PHP_PROJ4_RES_NAME, le_proj4);

    if (pj_is_geocent(pj)) {
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
ZEND_FUNCTION(pj_get_def) {
    zend_long options = 0;
    char *result;
    zval *zpj;
    projPJ pj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &zpj, &options) == FAILURE) {
        RETURN_FALSE;
    }

    pj = (projPJ*) zend_fetch_resource_ex(zpj, PHP_PROJ4_RES_NAME, le_proj4);

    if (pj != NULL && pj) {
        result = pj_get_def(pj, options);

        RETURN_STRING(result);
    }

    RETURN_FALSE;
}

/**
 * 
 * @param resource projection
 * @return mixed resource or false on error
 */
ZEND_FUNCTION(pj_latlong_from_proj) {
    zval *zpj_in;
    projPJ pj_in, pj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zpj_in) == FAILURE) {
        RETURN_FALSE;
    }

    pj_in = (projPJ*) zend_fetch_resource_ex(zpj_in, PHP_PROJ4_RES_NAME, le_proj4);

    if (pj_in != NULL && pj_in) {
        pj = pj_latlong_from_proj(pj_in);

        if (pj == NULL) {
            RETURN_FALSE;
        }

        RETURN_RES(zend_register_resource(pj, le_proj4));
    } else {
        RETURN_FALSE;
    }
}

/*###########################################################################
  Environment Functions
  ###########################################################################
*/
/**
 * 
 * @return void
 */
ZEND_FUNCTION(pj_deallocate_grids) {
    pj_deallocate_grids();
}

/**
 * 
 * @return numeric
 */
ZEND_FUNCTION(pj_get_errno_ref) {
    RETURN_LONG(*pj_get_errno_ref());
}

/**
 * 
 * @param integer error-code
 * @return string error-message
 */
ZEND_FUNCTION(pj_strerrno) {
    zend_long error_code;
    char *result;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &error_code) == FAILURE) {
        RETURN_FALSE;
    }

    result = pj_strerrno(error_code);

    RETURN_STRING(result);
}

/**
 * 
 * @return string
 */
ZEND_FUNCTION(pj_get_release) {
    const char *result;

    result = pj_get_release();

    RETURN_STRING(result);
}

