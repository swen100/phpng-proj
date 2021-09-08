#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <zend_types.h>
#include <zend_string.h>
#include <ext/standard/php_string.h>
#include <php_proj.h>

int proj4_destructor;
int proj4_area_destructor;

static zend_function_entry proj4_functions[] = {
    ZEND_FE(proj_create, NULL)
    ZEND_FE(proj_create_crs_to_crs, NULL)
    ZEND_FE(proj_transform_string, NULL)
    ZEND_FE(proj_transform_array, NULL)
    ZEND_FE(proj6_transform_array, NULL)
    ZEND_FE(proj_transform_point, NULL)
    ZEND_FE(proj6_transform_point, NULL)
    ZEND_FE(proj_is_latlong, NULL)
    ZEND_FE(proj_is_geocent, NULL)
    ZEND_FE(proj_get_def, NULL)
    ZEND_FE(proj_get_errno, NULL)
    ZEND_FE(proj_get_errno_string, NULL)
    ZEND_FE(proj_get_release, NULL)
    ZEND_FE(proj_get_info, NULL)
    ZEND_FE(proj_list_units, NULL)
    ZEND_FE(proj_list_ellps, NULL)
    ZEND_FE(proj_area_create, NULL)
    ZEND_FE(proj_area_set_bbox, NULL)
    ZEND_FE(proj_free, NULL) {
        NULL, NULL, NULL
    }
};

zend_module_entry proj4_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_PROJ_EXTNAME,
    proj4_functions,
    PHP_MINIT(proj4),           /* module init function */
    PHP_MSHUTDOWN(proj4),       /* module shutdown function */
    PHP_RINIT(proj4),           /* request init function */
    NULL,                       /* request shutdown function, would be PHP_RSHUTDOWN(proj4) */
    PHP_MINFO(proj4),           /* module info function */
    PHP_PROJ_VERSION,
    //PHP_MODULE_GLOBALS(geos),     /* globals descriptor */
    //PHP_GINIT(geos),              /* globals ctor */
    //NULL,                         /* globals dtor */
    //NULL,                         /* post deactivate */
    STANDARD_MODULE_PROPERTIES /* or STANDARD_MODULE_PROPERTIES_EX if above used */
};

static void php_proj4_dtor(zend_resource *resource) {
    PJ *proj = (PJ*) resource->ptr;
    if (proj != NULL && proj) {
        proj_destroy(proj);
    }
}

static void php_proj4_area_dtor(zend_resource *resource) {
    PJ_AREA *area = (PJ_AREA*) resource->ptr;
    if (area != NULL && area) {
        proj_area_destroy(area);
    }
}

PHP_RINIT_FUNCTION(proj4) {
    return SUCCESS;
}

PHP_MINIT_FUNCTION(proj4) {
    proj4_destructor = zend_register_list_destructors_ex(php_proj4_dtor, NULL, PHP_PROJ_RES_NAME, module_number);
    proj4_area_destructor = zend_register_list_destructors_ex(php_proj4_area_dtor, NULL, PHP_PROJ_AREA_RES_NAME, module_number);
    return SUCCESS;
}

PHP_MINFO_FUNCTION(proj4) {
    php_info_print_table_start();
    php_info_print_table_header(2, "proj module", "enabled");
    php_info_print_table_row(2, "Version", PHP_PROJ_VERSION);
    php_info_print_table_row(2, "Author", "Swen Zanon");
    php_info_print_table_row(2, "Powered by", "geoGLIS oHG");
    php_info_print_table_row(2, "PROJ Release", proj_info().release);
    php_info_print_table_row(2, "PROJ Version", proj_info().version);
    php_info_print_table_row(2, "PROJ Searchpath", proj_info().searchpath);
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
static zval projCoord_static(PJ *srcProj, PJ *tgtProj, double x, double y, double z)
{
    PJ_COORD a, b;
    zval coord;
    
    // source is already lat/lon -> deg2rad
    // proj_get_type
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
    //php_printf ("longitude: %g, latitude: %g\n", a.lp.lam, a.lp.phi);
    
    // transform to target-projection
    b = proj_trans(tgtProj, PJ_FWD, a);
    //php_printf ("easting: %g, northing: %g\n", b.enu.e, b.enu.n);
    
    if (proj_errno(tgtProj) == 0) {
        // rad2deg
        if (proj_angular_output(tgtProj, PJ_FWD) == 1) {
            b.xyz.x = proj_todeg(b.xyz.x);
            b.xyz.y = proj_todeg(b.xyz.y);
        }

        array_init(&coord);
        add_assoc_double(&coord, "x", b.xyz.x);
        add_assoc_double(&coord, "y", b.xyz.y);
        add_assoc_double(&coord, "z", b.xyz.z);
    }

    return coord;
}

static zval transformCoordArray_static(PJ *srcProj, PJ *tgtProj, zval xyz_arr)
{
    zval *x, *y, z, *t;//, *empty_arr;
    zval coord;
    HashTable *xyz_hash = Z_ARR(xyz_arr);

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

    array_init(&coord);
    add_assoc_double(&coord, "x", 0);
    add_assoc_double(&coord, "y", 0);
    add_assoc_double(&coord, "z", 0);
    return coord;
}

static zval transformCoordArray6_static(PJ *P, zval xyz_arr)
{
    zval *x, *y, z, *t, return_arr;
    double cx, cy, cz = 0;
    PJ_COORD c;
    HashTable *xyz_hash = Z_ARR(xyz_arr);

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

        c = proj_coord(Z_DVAL_P(x), Z_DVAL_P(y), Z_DVAL(z), 0);
        c = proj_trans(P, PJ_FWD, c);
        zval_dtor(x);
        zval_dtor(y);
        cx = c.xyz.x;
        cy = c.xyz.y;
        cz = c.xyz.z;
    }

    array_init(&return_arr);
    add_assoc_double(&return_arr, "x", cx);
    add_assoc_double(&return_arr, "y", cy);
    add_assoc_double(&return_arr, "z", cz);
    return return_arr;
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
ZEND_FUNCTION(proj_create)
{
    //PJ_CONTEXT *C;
    PJ *P;
    zend_string *definition;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(definition)
    ZEND_PARSE_PARAMETERS_END();

    //C = proj_context_create();
    P = proj_create(PJ_DEFAULT_CTX, ZSTR_VAL(definition));
    
    if (0==P) {
        //proj_context_destroy(C);
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(P, proj4_destructor));
}

/**
 * @param string crs code provided by authorities (such as "EPSG:4326", etc...) for source
 * @param string crs code provided by authorities (such as "EPSG:4326", etc...) for target
 * @param resource area of interest
 * @return resource
 */
ZEND_FUNCTION(proj_create_crs_to_crs)
{
    PJ *P;
    PJ_AREA *A = NULL;
    //PJ_CONTEXT *C;
    
    zend_string *srid_from, *srid_to;
    zval *proj_area = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 3)
            Z_PARAM_STR(srid_from)
            Z_PARAM_STR(srid_to)
            Z_PARAM_OPTIONAL
            Z_PARAM_RESOURCE(proj_area)
    ZEND_PARSE_PARAMETERS_END();

    if (proj_area && proj_area != NULL) {
        A = (PJ_AREA*) zend_fetch_resource_ex(proj_area, PHP_PROJ_AREA_RES_NAME, proj4_area_destructor);
        if (!A ) {
            RETURN_FALSE;
        }
    }
    
    //C = proj_context_create();
    P = proj_create_crs_to_crs(PJ_DEFAULT_CTX, ZSTR_VAL(srid_from), ZSTR_VAL(srid_to), A);
    
    if (0==P) {
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(P, proj4_destructor));
}

/**
 * @return resource
 */
ZEND_FUNCTION(proj_area_create)
{
    PJ_AREA *A;

    ZEND_PARSE_PARAMETERS_NONE();

    A = proj_area_create();
    
    if (0==A) {
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(A, proj4_area_destructor));
}

/**
 * @param resource
 * @param double west_lon_degree
 * @param double south_lat_degree
 * @param double east_lon_degree
 * @param double north_lat_degree
 * @return void
 */
ZEND_FUNCTION(proj_area_set_bbox)
{
    PJ_AREA *A;
    double west_lon_degree, south_lat_degree, east_lon_degree, north_lat_degree;
    zval *area;
    //PJ *P;

    ZEND_PARSE_PARAMETERS_START(5, 5)
            Z_PARAM_RESOURCE(area)
            Z_PARAM_DOUBLE(west_lon_degree)
            Z_PARAM_DOUBLE(south_lat_degree)
            Z_PARAM_DOUBLE(east_lon_degree)
            Z_PARAM_DOUBLE(north_lat_degree)
    ZEND_PARSE_PARAMETERS_END();
    
    A = (PJ_AREA*) zend_fetch_resource_ex(area, PHP_PROJ_AREA_RES_NAME, proj4_area_destructor);
    
    if (0==A) {
        RETURN_FALSE;
    }
    
    proj_area_set_bbox(A, west_lon_degree, south_lat_degree, east_lon_degree, north_lat_degree);
}

/**
 * @param resource
 * @return void
 */
ZEND_FUNCTION(proj_free)
{
    zval *zpj;
    //PJ *P;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();

    // @Todo: Prüfen - wofür dieser Aufruf? P wird ja nicht benutzt...
    //P = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj4_destructor);
    zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj4_destructor);

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
ZEND_FUNCTION(proj_transform_point) {

    double x, y, z = 0;
    zval *src, *tgt, coord;
    PJ *srcProj, *tgtProj;

    ZEND_PARSE_PARAMETERS_START(4, 5)
            Z_PARAM_RESOURCE(src)
            Z_PARAM_RESOURCE(tgt)
            Z_PARAM_DOUBLE(x)
            Z_PARAM_DOUBLE(y)
            Z_PARAM_OPTIONAL
            Z_PARAM_DOUBLE(z)
    ZEND_PARSE_PARAMETERS_END();

    srcProj = (PJ*) zend_fetch_resource_ex(src, PHP_PROJ_RES_NAME, proj4_destructor);
    tgtProj = (PJ*) zend_fetch_resource_ex(tgt, PHP_PROJ_RES_NAME, proj4_destructor);

    if (srcProj == NULL || tgtProj == NULL) {
        RETURN_FALSE;
    }

    coord = projCoord_static(srcProj, tgtProj, x, y, z);
    RETVAL_ARR(Z_ARR(coord));
    //return_value = projCoord_static(srcProj, tgtProj, x, y, z);
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
ZEND_FUNCTION(proj6_transform_point) {

    double x, y, z, t = 0;
    zval *src;
    PJ *P;
    PJ_COORD c;

    ZEND_PARSE_PARAMETERS_START(3, 4)
            Z_PARAM_RESOURCE(src)
            Z_PARAM_DOUBLE(x)
            Z_PARAM_DOUBLE(y)
            Z_PARAM_OPTIONAL
            Z_PARAM_DOUBLE(z)
    ZEND_PARSE_PARAMETERS_END();

    P = (PJ*) zend_fetch_resource_ex(src, PHP_PROJ_RES_NAME, proj4_destructor);

    if (P == NULL) {
        RETURN_FALSE;
    }

    c = proj_coord (x, y, z, t);
    c = proj_trans(P, PJ_FWD, c);
    
    array_init(return_value);
    add_assoc_double(return_value, "x", c.xyz.x);
    add_assoc_double(return_value, "y", c.xyz.y);
    add_assoc_double(return_value, "z", c.xyz.z);
    //*return_value = projCoord_static(P, x, y, z);
}

/**
 * 
 * @param resource srcDefn
 * @param resource tgtDefn
 * @param array points-array
 * @return mixed array ('x' => x, 'y' => y 'z' => z) or false on error
 */
ZEND_FUNCTION(proj6_transform_array) {

    /* method-params */
    zval xyz_arr, *zv, coord;
    zend_string *delimiter, *trimmed_point_string;

    /* user-params */
    zval *src, *xyz_arr_p;

    /* projection-params */
    PJ *P;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_RESOURCE(src)
            Z_PARAM_ARRAY(xyz_arr_p)
    ZEND_PARSE_PARAMETERS_END();

    P = (PJ*) zend_fetch_resource_ex(src, PHP_PROJ_RES_NAME, proj4_destructor);

    if (P == NULL) {
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
            coord = transformCoordArray6_static(P, xyz_arr);
            zval_ptr_dtor(zv);
            zval_ptr_dtor(&xyz_arr);
        } else {
            //ZVAL_COPY_VALUE(&xyz_arr, zv);
            coord = transformCoordArray6_static(P, *zv);
        }

        add_next_index_zval(return_value, &coord);
    }
    ZEND_HASH_FOREACH_END();

    // cleanup
    zend_string_release(delimiter);
}

/**
 * 
 * @param resource srcDefn
 * @param resource tgtDefn
 * @param array points-array
 * @return mixed array ('x' => x, 'y' => y 'z' => z) or false on error
 */
ZEND_FUNCTION(proj_transform_array) {

    /* method-params */
    zval xyz_arr, *zv, coord;
    zend_string *delimiter, *trimmed_point_string;

    /* user-params */
    zval *srcDefn, *tgtDefn, *xyz_arr_p;

    /* projection-params */
    PJ *srcProj, *tgtProj;

    ZEND_PARSE_PARAMETERS_START(3, 3)
            Z_PARAM_RESOURCE(srcDefn)
            Z_PARAM_RESOURCE(tgtDefn)
            Z_PARAM_ARRAY(xyz_arr_p)
    ZEND_PARSE_PARAMETERS_END();

    srcProj = (PJ*) zend_fetch_resource_ex(srcDefn, PHP_PROJ_RES_NAME, proj4_destructor);
    tgtProj = (PJ*) zend_fetch_resource_ex(tgtDefn, PHP_PROJ_RES_NAME, proj4_destructor);

    if (srcProj == NULL || tgtProj == NULL) {
        RETURN_FALSE;
    }

    array_init(return_value);
    delimiter = zend_string_init(" ", 1, 0);
    HashTable *pts_hash = Z_ARR_P(xyz_arr_p);

    ZEND_HASH_FOREACH_VAL(pts_hash, zv) {

        // split into in x,y,z-Array
        if (Z_TYPE_P(zv) != IS_ARRAY) {
            convert_to_string_ex(zv);
            array_init(&xyz_arr);
            trimmed_point_string = php_trim(Z_STR_P(zv), NULL, 0, 3);
            php_explode(delimiter, trimmed_point_string, &xyz_arr, LONG_MAX);
            zend_string_release(trimmed_point_string);
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
ZEND_FUNCTION(proj_transform_string) {

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

    ZEND_PARSE_PARAMETERS_START(3, 3)
            Z_PARAM_RESOURCE(srcDefn)
            Z_PARAM_RESOURCE(tgtDefn)
            Z_PARAM_STR(str)
    ZEND_PARSE_PARAMETERS_END();

    srcProj = (PJ*) zend_fetch_resource_ex(srcDefn, PHP_PROJ_RES_NAME, proj4_destructor);
    tgtProj = (PJ*) zend_fetch_resource_ex(tgtDefn, PHP_PROJ_RES_NAME, proj4_destructor);

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
        pts_hash = Z_ARR(pts_arr);

        ZEND_HASH_FOREACH_VAL(pts_hash, zv) {
            //convert_to_string_ex(zv);
            xyz_string = zend_string_dup(Z_STR_P(zv), 0);

            // split into in x,y,z
            array_init(&xyz_arr);
            trimmed_point_string = php_trim(xyz_string, NULL, 0, 3);

            php_explode(delimiter2, trimmed_point_string, &xyz_arr, LONG_MAX);

            coord = transformCoordArray_static(srcProj, tgtProj, xyz_arr);
            add_next_index_zval(return_value, &coord);

            // cleanup
            //zval_ptr_dtor(zv);
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
ZEND_FUNCTION(proj_is_latlong)
{
    zval *zpj;
    PJ *pj;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();
    
    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj4_destructor);

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
ZEND_FUNCTION(proj_is_geocent)
{
    zval *zpj;
    PJ *pj;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();

    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj4_destructor);

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
ZEND_FUNCTION(proj_get_def)
{
    PJ_PROJ_INFO result;
    zval *zpj;
    PJ *pj;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();

    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj4_destructor);

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
ZEND_FUNCTION(proj_get_errno)
{
    zval *zpj;
    PJ *pj;
    zend_long error_code;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();
    
    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj4_destructor);
    
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
ZEND_FUNCTION(proj_get_errno_string)
{
    zend_long errorCode;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_LONG(errorCode)
    ZEND_PARSE_PARAMETERS_END();
    
    if( errorCode == 0 ) {
        RETURN_EMPTY_STRING();
    }

    RETURN_STRING( proj_errno_string((int)errorCode) );
}

/**
 * 
 * @return string
 */
ZEND_FUNCTION(proj_get_release)
{
    RETURN_STRING( proj_info().release );
}

/**
 * 
 * @return array
 */
ZEND_FUNCTION(proj_get_info)
{
    zval info_arr;
    array_init(&info_arr);
    
    add_assoc_string(&info_arr, "version", strdup(proj_info().version));
    add_assoc_long(&info_arr, "major", proj_info().major);
    add_assoc_long(&info_arr, "minor", proj_info().minor);
    add_assoc_long(&info_arr, "patch", proj_info().patch);
    //add_assoc_string(&info_arr, "paths", strdup(proj_info().paths));
    add_assoc_string(&info_arr, "release", strdup(proj_info().release));
    add_assoc_string(&info_arr, "searchpath", strdup(proj_info().searchpath));
    
    RETURN_ARR( Z_ARR(info_arr) );
}

/**
 * @hint deprecated since PROJ 7.1
 * @see https://proj.org/development/reference/datatypes.html#c.PJ_UNITS
 * @return array
 */
ZEND_FUNCTION(proj_list_units)
{
    zval tmp;
    
    array_init(return_value);
    
#if PROJ_VERSION_MAJOR < 7 && PROJ_VERSION_MINOR < 2
    const PJ_UNITS *unit;
    
    for (unit=proj_list_units(); unit->id ; ++unit) {
        //php_printf("%9s %-16s %-16s %f\n",unit->id, unit->to_meter, unit->name, unit->factor);
        
        array_init(&tmp);
        add_assoc_string(&tmp, "name", unit->name);
        add_assoc_string(&tmp, "to_meter", unit->to_meter);
        add_assoc_double(&tmp, "factor", unit->factor);
        
        zval dup;
        ZVAL_COPY(&dup, &tmp);
        add_assoc_zval(return_value, unit->id, &dup);
        zval_dtor(&tmp);
    }
#else
    int *result_count;
    PROJ_UNIT_INFO** units;
    units = proj_get_units_from_database(NULL,NULL,"linear",0,result_count);
    
    for (int i = 0; units && units[i]; i++) {
        //php_printf("%9s %-16s %-16s %-16s %f\n",units[i]->code, units[i]->auth_name, units[i]->category, units[i]->name, units[i]->conv_factor);
        array_init(&tmp);
        
        add_assoc_string(&tmp, "name", units[i]->name);
        add_assoc_string(&tmp, "auth_name", units[i]->auth_name);
        add_assoc_string(&tmp, "category", units[i]->category);
        add_assoc_double(&tmp, "conv_factor", units[i]->conv_factor);
        
        zval dup;
        ZVAL_COPY(&dup, &tmp);
        add_assoc_zval(return_value, units[i]->code, &dup);
        zval_dtor(&tmp);
    }
    proj_unit_list_destroy(units);
#endif
}


/**
 * @see https://proj.org/development/reference/datatypes.html#c.PJ_ELLPS
 * @return array
 */
ZEND_FUNCTION(proj_list_ellps)
{
    const PJ_ELLPS *ellps;
    zval tmp;

    array_init(return_value);
    
    for (ellps=proj_list_ellps(); ellps->id ; ++ellps) {
        //php_printf("%9s %-16s %-16s %s\n",ellps->id, ellps->major, ellps->ell, ellps->name);
        
        array_init(&tmp);
        add_assoc_string(&tmp, "name", ellps->name);
        add_assoc_string(&tmp, "major", ellps->major);
        add_assoc_string(&tmp, "ell", ellps->ell);
        
        zval dup;
        ZVAL_COPY(&dup, &tmp);
        add_assoc_zval(return_value, ellps->id, &dup);
        zval_dtor(&tmp);
    }
}