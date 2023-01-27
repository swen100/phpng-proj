#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "php_proj.h"

int proj_destructor;
int proj_area_destructor;


/*###########################################################################
  Internal static Functions
  ###########################################################################
 */
static zval proj4_transform_coord_static(PJ *srcProj, PJ *tgtProj, double x, double y, double z)
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

static zval proj4_transform_array_static(PJ *srcProj, PJ *tgtProj, zval *xyz_arr)
{
    zval *x, *y, z, *t, coord;
    HashTable *xyz_hash = Z_ARR_P(xyz_arr);

    if (NULL != (x = zend_hash_index_find(xyz_hash, 0)) &&
        NULL != (y = zend_hash_index_find(xyz_hash, 1))) {

        if (NULL == (t = zend_hash_index_find(xyz_hash, 2))) {
            ZVAL_DOUBLE(&z, 0.0);
        } else {
            ZVAL_COPY_VALUE(&z, t);
            convert_to_double_ex(&z);
        }

        convert_to_double_ex(x);
        convert_to_double_ex(y);

        coord = proj4_transform_coord_static(srcProj, tgtProj, Z_DVAL_P(x), Z_DVAL_P(y), Z_DVAL(z));
        return coord;
    }

    array_init(&coord);
    add_assoc_double(&coord, "x", 0);
    add_assoc_double(&coord, "y", 0);
    add_assoc_double(&coord, "z", 0);
    return coord;
}

static zval proj_transform_array_static(PJ *Proj, zval *xyz_arr)
{
    zval *x, *y, z, *t, coord;
    double cx = 0.0, cy = 0.0, cz = 0.0;
    PJ_COORD c;
    HashTable *xyz_hash = Z_ARR_P(xyz_arr);

    if (NULL != (x = zend_hash_index_find(xyz_hash, 0)) &&
        NULL != (y = zend_hash_index_find(xyz_hash, 1))) {
        
        if (NULL == (t = zend_hash_index_find(xyz_hash, 2))) {
            ZVAL_DOUBLE(&z, 0.0);
        } else {
            ZVAL_COPY_VALUE(&z, t);
            convert_to_double_ex(&z);
        }

        convert_to_double_ex(x);
        convert_to_double_ex(y);

        c = proj_coord(Z_DVAL_P(x), Z_DVAL_P(y), Z_DVAL(z), 0);
        c = proj_trans(Proj, PJ_FWD, c);

        cx = c.xyz.x;
        cy = c.xyz.y;
        cz = c.xyz.z;
    }

    array_init(&coord);
    add_assoc_double(&coord, "x", cx);
    add_assoc_double(&coord, "y", cy);
    add_assoc_double(&coord, "z", cz);
    return coord;
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
ZEND_BEGIN_ARG_INFO(proj_create_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, definition)
ZEND_END_ARG_INFO()

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

    RETURN_RES(zend_register_resource(P, proj_destructor));
}

/**
 * @param string crs code provided by authorities (such as "EPSG:4326", etc...) for source
 * @param string crs code provided by authorities (such as "EPSG:4326", etc...) for target
 * @param resource area of interest
 * @return resource
 */
ZEND_BEGIN_ARG_INFO(proj_create_crs_to_crs_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, srid_from)
    ZEND_ARG_INFO(0, srid_to)
    ZEND_ARG_INFO(0, proj_area)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_create_crs_to_crs)
{
    PJ *Proj;
    PJ_AREA *Area = NULL;
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
        Area = (PJ_AREA*) zend_fetch_resource_ex(proj_area, PHP_PROJ_AREA_RES_NAME, proj_area_destructor);
        if (!Area ) {
            RETURN_FALSE;
        }
    }
    
    //C = proj_context_create();
    Proj = proj_create_crs_to_crs(PJ_DEFAULT_CTX, ZSTR_VAL(srid_from), ZSTR_VAL(srid_to), Area);
    
    if (0==Proj) {
        RETURN_FALSE;
    }

    /* For that particular use case, this is not needed.
       proj_normalize_for_visualization() ensures that the coordinate
       order expected and returned by proj_trans() will be longitude,
       latitude for geographic CRS, and easting, northing for projected
       CRS. If instead of using PROJ strings, "EPSG:XXXX" codes had been used,
       this might had been necessary. */
    PJ* Proj_for_GIS = proj_normalize_for_visualization(PJ_DEFAULT_CTX, Proj);
    if( NULL != Proj_for_GIS )  {
        //proj_destroy(Proj);
        Proj = Proj_for_GIS;
    }
    
    RETURN_RES(zend_register_resource(Proj, proj_destructor));
}

/**
 * @param resource source_crs
 * @param resource target_crs
 * @param resource area of interest
 * @param string options
 * @return resource
 */
ZEND_BEGIN_ARG_INFO(proj_create_crs_to_crs_from_pj_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, src_crs)
    ZEND_ARG_INFO(0, tgtc_crs)
    ZEND_ARG_INFO(0, proj_area)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_create_crs_to_crs_from_pj)
{
    PJ *Proj;
    PJ_AREA *Area = NULL;
    
    char *options;
    size_t options_len;
    
    zval *proj_area = NULL;
    zval *src_crs, *tgt_crs;
    PJ *srcP, *tgtP;
    
    ZEND_PARSE_PARAMETERS_START(2, 4)
            Z_PARAM_RESOURCE(src_crs)
            Z_PARAM_RESOURCE(tgt_crs)
            Z_PARAM_OPTIONAL
            Z_PARAM_RESOURCE(proj_area)
            Z_PARAM_STRING(options, options_len)
    ZEND_PARSE_PARAMETERS_END();
    
    srcP = (PJ*) zend_fetch_resource_ex(src_crs, PHP_PROJ_RES_NAME, proj_destructor);
    tgtP = (PJ*) zend_fetch_resource_ex(tgt_crs, PHP_PROJ_RES_NAME, proj_destructor);
    
    if (srcP == NULL || tgtP == NULL) {
        RETURN_FALSE;
    }
    
    if (proj_area && proj_area != NULL) {
        Area = (PJ_AREA*) zend_fetch_resource_ex(proj_area, PHP_PROJ_AREA_RES_NAME, proj_area_destructor);
        if (!Area ) {
            RETURN_FALSE;
        }
    }
    
    //C = proj_context_create();
    Proj = proj_create_crs_to_crs_from_pj(PJ_DEFAULT_CTX, srcP, tgtP, Area, NULL);
    
    if (0==Proj) {
        RETURN_FALSE;
    }

    /* For that particular use case, this is not needed.
       proj_normalize_for_visualization() ensures that the coordinate
       order expected and returned by proj_trans() will be longitude,
       latitude for geographic CRS, and easting, northing for projected
       CRS. If instead of using PROJ strings, "EPSG:XXXX" codes had been used,
       this might had been necessary. */
    PJ* Proj_for_GIS = proj_normalize_for_visualization(PJ_DEFAULT_CTX, Proj);
    if( NULL != Proj_for_GIS )  {
        //proj_destroy(Proj);
        Proj = Proj_for_GIS;
    }
    
    RETURN_RES(zend_register_resource(Proj, proj_destructor));
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

    RETURN_RES(zend_register_resource(A, proj_area_destructor));
}

/**
 * @param resource Area
 * @param double west_lon_degree
 * @param double south_lat_degree
 * @param double east_lon_degree
 * @param double north_lat_degree
 * @return void
 */
ZEND_BEGIN_ARG_INFO(proj_area_set_bbox_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, ProjArea)
    ZEND_ARG_INFO(0, west_lon_degree)
    ZEND_ARG_INFO(0, south_lat_degree)
    ZEND_ARG_INFO(0, east_lon_degree)
    ZEND_ARG_INFO(0, north_lat_degree)
ZEND_END_ARG_INFO()

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
    
    A = (PJ_AREA*) zend_fetch_resource_ex(area, PHP_PROJ_AREA_RES_NAME, proj_area_destructor);
    
    if (0==A) {
        RETURN_FALSE;
    }
    
    proj_area_set_bbox(A, west_lon_degree, south_lat_degree, east_lon_degree, north_lat_degree);
}

/**
 * @param resource Proj
 * @return void
 */
ZEND_FUNCTION(proj_free)
{
    zval *zpj;
    PJ *P;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();

    P = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj_destructor);
    
    if (P != NULL) {
        zend_list_delete(Z_RES_P(zpj));
    }
}

/**
 * @param resource srcDefn
 * @param resource tgtDefn
 * @param float x
 * @param float y
 * @param float z
 * @return mixed array ('x' => x, 'y' => y 'z' => z) or false on error
 */
ZEND_BEGIN_ARG_INFO(proj4_transform_point_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, src)
    ZEND_ARG_INFO(0, tgt)
    ZEND_ARG_INFO(0, x)
    ZEND_ARG_INFO(0, y)
    ZEND_ARG_INFO(0, z)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj4_transform_point)
{
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

    srcProj = (PJ*) zend_fetch_resource_ex(src, PHP_PROJ_RES_NAME, proj_destructor);
    tgtProj = (PJ*) zend_fetch_resource_ex(tgt, PHP_PROJ_RES_NAME, proj_destructor);

    if (srcProj == NULL || tgtProj == NULL) {
        RETURN_FALSE;
    }

    coord = proj4_transform_coord_static(srcProj, tgtProj, x, y, z);
    RETVAL_ARR(Z_ARR(coord));
    //return_value = proj4_transform_coord_static(srcProj, tgtProj, x, y, z);
}

/**
 * @param resource Proj
 * @param float x
 * @param float y
 * @param float z
 * @return mixed array ('x' => x, 'y' => y 'z' => z) or false on error
 */
ZEND_BEGIN_ARG_INFO(proj_transform_point_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, Proj)
    ZEND_ARG_INFO(0, x)
    ZEND_ARG_INFO(0, y)
    ZEND_ARG_INFO(0, z)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_transform_point)
{
    double x, y, z, t = 0;
    zval *Proj;
    PJ *P;
    PJ_COORD c;

    ZEND_PARSE_PARAMETERS_START(3, 4)
            Z_PARAM_RESOURCE(Proj)
            Z_PARAM_DOUBLE(x)
            Z_PARAM_DOUBLE(y)
            Z_PARAM_OPTIONAL
            Z_PARAM_DOUBLE(z)
    ZEND_PARSE_PARAMETERS_END();

    P = (PJ*) zend_fetch_resource_ex(Proj, PHP_PROJ_RES_NAME, proj_destructor);

    if (P == NULL) {
        RETURN_FALSE;
    }

    c = proj_coord (x, y, z, t);
    c = proj_trans(P, PJ_FWD, c);
    
    array_init(return_value);
    add_assoc_double(return_value, "x", c.xyz.x);
    add_assoc_double(return_value, "y", c.xyz.y);
    add_assoc_double(return_value, "z", c.xyz.z);
    //*return_value = proj4_transform_coord_static(P, x, y, z);
}

/**
 * 
 * @param resource srcDefn
 * @param resource tgtDefn
 * @param array points-array
 * @return mixed array ('x' => x, 'y' => y 'z' => z) or false on error
 */
ZEND_BEGIN_ARG_INFO(proj_transform_array_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, Proj)
    ZEND_ARG_INFO(0, points)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_transform_array)
{
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

    P = (PJ*) zend_fetch_resource_ex(src, PHP_PROJ_RES_NAME, proj_destructor);

    if (P == NULL) {
        RETURN_FALSE;
    }

    array_init(return_value);
    delimiter = zend_string_init(" ", 1, 0);

    ZEND_HASH_FOREACH_VAL(Z_ARR_P(xyz_arr_p), zv) {
        
        // split into x,y,z-array
        if (Z_TYPE_P(zv) != IS_ARRAY) {
            // needed to make sure we have a string. Throws an error if not.
            convert_to_string_ex(zv);
            array_init(&xyz_arr);
            trimmed_point_string = php_trim(Z_STR_P(zv), NULL, 0, 3);
            php_explode(delimiter, trimmed_point_string, &xyz_arr, LONG_MAX);
            coord = proj_transform_array_static(P, &xyz_arr);
            zval_ptr_dtor(zv);
            zval_ptr_dtor(&xyz_arr);
        } else {
            coord = proj_transform_array_static(P, zv);
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
ZEND_BEGIN_ARG_INFO(proj4_transform_array_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, srcProj)
    ZEND_ARG_INFO(0, tgtProj)
    ZEND_ARG_INFO(0, points)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj4_transform_array)
{
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

    srcProj = (PJ*) zend_fetch_resource_ex(srcDefn, PHP_PROJ_RES_NAME, proj_destructor);
    tgtProj = (PJ*) zend_fetch_resource_ex(tgtDefn, PHP_PROJ_RES_NAME, proj_destructor);

    if (srcProj == NULL || tgtProj == NULL) {
        RETURN_FALSE;
    }

    array_init(return_value);
    delimiter = zend_string_init(" ", 1, 0);
    HashTable *pts_hash = Z_ARR_P(xyz_arr_p);

    ZEND_HASH_FOREACH_VAL(pts_hash, zv) {

        // split into into x,y,z-Array
        if (Z_TYPE_P(zv) != IS_ARRAY) {
            // needed to make sure we have a string. Throws an error if not.
            convert_to_string_ex(zv);
            array_init(&xyz_arr);
            trimmed_point_string = php_trim(Z_STR_P(zv), NULL, 0, 3);
            php_explode(delimiter, trimmed_point_string, &xyz_arr, LONG_MAX);
            coord = proj4_transform_array_static(srcProj, tgtProj, &xyz_arr);
            zval_ptr_dtor(zv);
            zval_ptr_dtor(&xyz_arr);
        } else {
            coord = proj4_transform_array_static(srcProj, tgtProj, zv);
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
ZEND_BEGIN_ARG_INFO(proj4_transform_string_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, srcProj)
    ZEND_ARG_INFO(0, tgtProj)
    ZEND_ARG_INFO(0, geometry)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj4_transform_string)
{
    /* user-params */
    zval *srcDefn, *tgtDefn;
    zend_string *str;
    
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

    srcProj = (PJ*) zend_fetch_resource_ex(srcDefn, PHP_PROJ_RES_NAME, proj_destructor);
    tgtProj = (PJ*) zend_fetch_resource_ex(tgtDefn, PHP_PROJ_RES_NAME, proj_destructor);

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
            // needed to make sure we have a string. Throws an error if not.
            convert_to_string_ex(zv);

            // split into in x,y,z
            array_init(&xyz_arr);
            trimmed_point_string = php_trim(Z_STR_P(zv), NULL, 0, 3);

            php_explode(delimiter2, trimmed_point_string, &xyz_arr, LONG_MAX);

            coord = proj4_transform_array_static(srcProj, tgtProj, &xyz_arr);
            add_next_index_zval(return_value, &coord);

            // cleanup
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

/**
 * @param resource Proj
 * @param string geometry
 * @return mixed array ('x' => x, 'y' => y 'z' => z) or FALSE on error
 */
ZEND_BEGIN_ARG_INFO(proj_transform_string_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, Proj)
    ZEND_ARG_INFO(0, geometry)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_transform_string)
{
    /* user-params */
    zval *Proj;
    zend_string *str;

    /* projection-params */
    PJ *P;

    /* coord-params */
    zval pts_arr, xyz_arr;
    zend_string *delimiter, *delimiter2, *trimmed_geom_string, *trimmed_point_string;
    HashTable *pts_hash;
    zval coord;
    zval *zv;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_RESOURCE(Proj)
            Z_PARAM_STR(str)
    ZEND_PARSE_PARAMETERS_END();

    P = (PJ*) zend_fetch_resource_ex(Proj, PHP_PROJ_RES_NAME, proj_destructor);

    if (P == NULL) {
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
            convert_to_string_ex(zv);
            
            // split into in x,y,z
            array_init(&xyz_arr);
            trimmed_point_string = php_trim(Z_STR_P(zv), NULL, 0, 3);

            php_explode(delimiter2, trimmed_point_string, &xyz_arr, LONG_MAX);

            coord = proj_transform_array_static(P, &xyz_arr);
            add_next_index_zval(return_value, &coord);

            // cleanup
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

/**
 * https://proj.org/development/reference/functions.html#c.proj_lp_dist
 *
 * @param resource Proj
 * @param float lon1
 * @param float lat1
 * @param float lon2
 * @param float lat2
 * @return double distance in meters or false on error
 */
ZEND_BEGIN_ARG_INFO(proj_distance_lp_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, Proj)
    ZEND_ARG_INFO(0, lon1)
    ZEND_ARG_INFO(0, lat1)
    ZEND_ARG_INFO(0, lon2)
    ZEND_ARG_INFO(0, lat2)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_distance_lp)
{
    double lon1, lat1, lon2, lat2 = 0;
    zval *src;
    PJ *P;

    ZEND_PARSE_PARAMETERS_START(5, 5)
        Z_PARAM_RESOURCE(src)
        Z_PARAM_DOUBLE(lon1)
        Z_PARAM_DOUBLE(lat1)
        Z_PARAM_DOUBLE(lon2)
        Z_PARAM_DOUBLE(lat2)
    ZEND_PARSE_PARAMETERS_END();

    P = (PJ*) zend_fetch_resource_ex(src, PHP_PROJ_RES_NAME, proj_destructor);

    if (P == NULL) {
        RETURN_FALSE;
    }

    PJ_COORD c1 = proj_coord(lon1, lat1, 0, 0);
    PJ_COORD c2 = proj_coord(lon2, lat2, 0, 0);

    RETURN_DOUBLE( proj_lp_dist(P, c1, c2) );
}

/**
 * https://proj.org/development/reference/functions.html#c.proj_lpz_dist
 *
 * @param resource Proj
 * @param float lon1
 * @param float lat1
 * @param float z1
 * @param float lon2
 * @param float lat2
 * @param float z2
 * @return double distance in meters or false on error
 */
ZEND_BEGIN_ARG_INFO(proj_distance_lpz_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, Proj)
    ZEND_ARG_INFO(0, lon1)
    ZEND_ARG_INFO(0, lat1)
    ZEND_ARG_INFO(0, z1)
    ZEND_ARG_INFO(0, lon2)
    ZEND_ARG_INFO(0, lat2)
    ZEND_ARG_INFO(0, z2)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_distance_lpz)
{
    double lon1, lat1, z1, lon2, lat2, z2, t = 0;
    zval *src;
    PJ *P;

    ZEND_PARSE_PARAMETERS_START(7, 7)
        Z_PARAM_RESOURCE(src)
        Z_PARAM_DOUBLE(lon1)
        Z_PARAM_DOUBLE(lat1)
        Z_PARAM_DOUBLE(z1)
        Z_PARAM_DOUBLE(lon2)
        Z_PARAM_DOUBLE(lat2)
        Z_PARAM_DOUBLE(z2)
    ZEND_PARSE_PARAMETERS_END();

    P = (PJ*) zend_fetch_resource_ex(src, PHP_PROJ_RES_NAME, proj_destructor);

    if (P == NULL) {
        RETURN_FALSE;
    }

    PJ_COORD c1 = proj_coord(lon1, lat1, z1, t);
    PJ_COORD c2 = proj_coord(lon2, lat2, z2, t);

    RETURN_DOUBLE( proj_lpz_dist(P, c1, c2) );
}

/**
 * https://proj.org/development/reference/functions.html#c.proj_xy_dist
 *
 * @param float x1
 * @param float y1
 * @param float x2
 * @param float y2
 * @return double distance in meters or false on error
 */
ZEND_BEGIN_ARG_INFO(proj_distance_xy_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, x1)
    ZEND_ARG_INFO(0, y1)
    ZEND_ARG_INFO(0, x2)
    ZEND_ARG_INFO(0, y2)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_distance_xy)
{
    double x1, y1, x2, y2 = 0;

    ZEND_PARSE_PARAMETERS_START(4, 4)
        Z_PARAM_DOUBLE(x1)
        Z_PARAM_DOUBLE(y1)
        Z_PARAM_DOUBLE(x2)
        Z_PARAM_DOUBLE(y2)
    ZEND_PARSE_PARAMETERS_END();

    PJ_COORD c1 = proj_coord(x1, y1, 0, 0);
    PJ_COORD c2 = proj_coord(x2, y2, 0, 0);

    RETURN_DOUBLE( proj_xy_dist(c1, c2) );
}

/**
 * https://proj.org/development/reference/functions.html#c.proj_xyz_dist
 *
 * @param float x1
 * @param float y1
 * @param float z1
 * @param float x2
 * @param float y2
 * @param float z2
 * @return double distance in meters or false on error
 */
ZEND_BEGIN_ARG_INFO(proj_distance_xyz_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, x1)
    ZEND_ARG_INFO(0, y1)
    ZEND_ARG_INFO(0, z1)
    ZEND_ARG_INFO(0, x2)
    ZEND_ARG_INFO(0, y2)
    ZEND_ARG_INFO(0, z2)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_distance_xyz)
{
    double x1, y1, z1, x2, y2, z2 = 0;

    ZEND_PARSE_PARAMETERS_START(6, 6)
        Z_PARAM_DOUBLE(x1)
        Z_PARAM_DOUBLE(y1)
        Z_PARAM_DOUBLE(z1)
        Z_PARAM_DOUBLE(x2)
        Z_PARAM_DOUBLE(y2)
        Z_PARAM_DOUBLE(z2)
    ZEND_PARSE_PARAMETERS_END();

    PJ_COORD c1 = proj_coord(x1, y1, z1, 0);
    PJ_COORD c2 = proj_coord(x2, y2, z2, 0);

    RETURN_DOUBLE( proj_xyz_dist(c1, c2) );
}


/*###########################################################################
  Advanced Functions
  ###########################################################################
 */

/*
 * @param resource projection
 * @return boolean
 */
ZEND_BEGIN_ARG_INFO(proj_is_latlong_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, Proj)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_is_latlong)
{
    zval *zpj;
    PJ *pj;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();
    
    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj_destructor);

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
ZEND_BEGIN_ARG_INFO(proj_is_geocent_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, Proj)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_is_geocent)
{
    zval *zpj;
    PJ *pj;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();

    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj_destructor);

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
ZEND_BEGIN_ARG_INFO(proj_get_def_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, Proj)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_get_def)
{
    PJ_PROJ_INFO result;
    zval *zpj;
    PJ *pj;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();

    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj_destructor);

    if (pj != NULL && pj) {
        result = proj_pj_info(pj);

        RETURN_STRING(result.definition);
    }

    RETURN_FALSE;
}

/**
 * 
 * @param resource projection
 * @param long
 * @return mixed array or false on error
 */
ZEND_BEGIN_ARG_INFO(proj_get_pj_info_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, Proj)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_get_pj_info)
{
    PJ_PROJ_INFO result;
    zval *zpj;
    PJ *pj;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();

    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj_destructor);

    if (pj == NULL) {
        RETURN_FALSE;
    }
    
    array_init(return_value);
    result = proj_pj_info(pj);

    add_assoc_string(return_value, "id", result.id ? result.id : "");
    add_assoc_string(return_value, "definition", result.definition);
    add_assoc_string(return_value, "description", result.description);
    add_assoc_double(return_value, "accuracy", result.accuracy);
    add_assoc_long(return_value, "has_inverse", result.has_inverse);
}

/*###########################################################################
  Environment Functions
  ###########################################################################
 */

/**
 * 
 * @return numeric
 */
ZEND_BEGIN_ARG_INFO(proj_get_errno_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, Proj)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_get_errno)
{
    zval *zpj;
    PJ *pj;
    zend_long error_code;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();
    
    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj_destructor);
    
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
ZEND_BEGIN_ARG_INFO(proj_get_errno_string_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, errorCode)
ZEND_END_ARG_INFO()

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


/* ----------------------------------------------------------------
    Proj Definition and registration
------------------------------------------------------------------*/

ZEND_BEGIN_ARG_INFO(Proj_method_no_args, ZEND_SEND_BY_VAL)
ZEND_END_ARG_INFO()

static const zend_function_entry proj_functions[] = {
    ZEND_FE(proj_create, proj_create_args)
    ZEND_FE(proj_create_crs_to_crs, proj_create_crs_to_crs_args)
    ZEND_FE(proj_create_crs_to_crs_from_pj, proj_create_crs_to_crs_from_pj_args)
    ZEND_FE(proj4_transform_string, proj4_transform_string_args)
    ZEND_FE(proj_transform_string, proj_transform_string_args)
    ZEND_FE(proj4_transform_array, proj4_transform_array_args)
    ZEND_FE(proj_transform_array, proj_transform_array_args)
    ZEND_FE(proj4_transform_point, proj4_transform_point_args)
    ZEND_FE(proj_transform_point, proj_transform_point_args)
    ZEND_FE(proj_is_latlong, proj_is_latlong_args)
    ZEND_FE(proj_is_geocent, proj_is_geocent_args)
    ZEND_FE(proj_get_def, proj_get_def_args)
    ZEND_FE(proj_get_pj_info, proj_get_pj_info_args)
    ZEND_FE(proj_get_errno, proj_get_errno_args)
    ZEND_FE(proj_get_errno_string, proj_get_errno_string_args)
    ZEND_FE(proj_get_release, Proj_method_no_args)
    ZEND_FE(proj_get_info, Proj_method_no_args)
    ZEND_FE(proj_list_units, Proj_method_no_args)
    ZEND_FE(proj_list_ellps, Proj_method_no_args)
    ZEND_FE(proj_area_create, Proj_method_no_args)
    ZEND_FE(proj_area_set_bbox, proj_area_set_bbox_args)
    ZEND_FE(proj_distance_lp, proj_distance_lp_args)
    ZEND_FE(proj_distance_lpz, proj_distance_lpz_args)
    ZEND_FE(proj_distance_xy, proj_distance_xy_args)
    ZEND_FE(proj_distance_xyz, proj_distance_xyz_args)
    ZEND_FE(proj_free, Proj_method_no_args) {
        NULL, NULL, NULL
    }
};

zend_module_entry proj_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_PROJ_EXTNAME,
    proj_functions,
    PHP_MINIT(proj),           /* module init function */
    PHP_MSHUTDOWN(proj),       /* module shutdown function */
    PHP_RINIT(proj),           /* request init function */
    NULL,                       /* request shutdown function, would be PHP_RSHUTDOWN(proj) */
    PHP_MINFO(proj),           /* module info function */
    PHP_PROJ_VERSION,
    //PHP_MODULE_GLOBALS(geos),     /* globals descriptor */
    //PHP_GINIT(geos),              /* globals ctor */
    //NULL,                         /* globals dtor */
    //NULL,                         /* post deactivate */
    STANDARD_MODULE_PROPERTIES /* or STANDARD_MODULE_PROPERTIES_EX if above used */
};

static void php_proj_dtor(zend_resource *resource) {
    PJ *proj = (PJ*) resource->ptr;
    if (proj != NULL && proj) {
        proj_destroy(proj);
    }
}

static void php_proj_area_dtor(zend_resource *resource) {
    PJ_AREA *area = (PJ_AREA*) resource->ptr;
    if (area != NULL && area) {
        proj_area_destroy(area);
    }
}

PHP_RINIT_FUNCTION(proj) {
    return SUCCESS;
}

PHP_MINIT_FUNCTION(proj) {
    proj_destructor = zend_register_list_destructors_ex(php_proj_dtor, NULL, PHP_PROJ_RES_NAME, module_number);
    proj_area_destructor = zend_register_list_destructors_ex(php_proj_area_dtor, NULL, PHP_PROJ_AREA_RES_NAME, module_number);
    return SUCCESS;
}

PHP_MINFO_FUNCTION(proj) {
    php_info_print_table_start();
    php_info_print_table_header(2, "proj module", "enabled");
    php_info_print_table_row(2, "Version", PHP_PROJ_VERSION);
    php_info_print_table_row(2, "Author", "Swen Zanon");
    php_info_print_table_row(2, "Powered by", "geoGLIS GmbH & Co. KG");
    php_info_print_table_row(2, "PROJ Release", proj_info().release);
    php_info_print_table_row(2, "PROJ Version", proj_info().version);
    php_info_print_table_row(2, "PROJ Searchpath", proj_info().searchpath);
    php_info_print_table_end();
}

PHP_MSHUTDOWN_FUNCTION(proj) {
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}

#ifdef COMPILE_DL_PROJ
ZEND_GET_MODULE(proj)
#endif
