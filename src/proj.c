#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "php_proj.h"

int proj_destructor;
int proj_area_destructor;

ZEND_DECLARE_MODULE_GLOBALS(proj)

#if defined(ZTS) && defined(COMPILE_DL_PROJ)
ZEND_TSRMLS_CACHE_DEFINE()
#endif


/*###########################################################################
  Internal static Functions
  ###########################################################################
 */

/*
 * Reads x, y and an optional z from an array [x, y(, z)] or a string "x y( z)".
 * Missing z is 0. Returns 0 if fewer than two coordinates are found.
 * The input is never modified: it belongs to the caller and may live in read-only OPcache memory.
 */
static int php_proj_read_point(zval *zv, double *xyz)
{
    xyz[2] = 0.0;
    ZVAL_DEREF(zv);

    if (Z_TYPE_P(zv) == IS_ARRAY) {
        zval *x = zend_hash_index_find(Z_ARRVAL_P(zv), 0);
        zval *y = zend_hash_index_find(Z_ARRVAL_P(zv), 1);
        zval *z = zend_hash_index_find(Z_ARRVAL_P(zv), 2);

        if (x == NULL || y == NULL) {
            return 0;
        }
        xyz[0] = zval_get_double(x);
        xyz[1] = zval_get_double(y);
        if (z != NULL) {
            xyz[2] = zval_get_double(z);
        }
        return 1;
    }

    zend_string *str = zval_get_string(zv);
    const char *p = ZSTR_VAL(str);
    const char *end;
    int n = 0;

    /* numbers separated by any amount of whitespace, so "x  y" does not give y = 0 */
    while (n < 3) {
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
            p++;
        }
        double v = zend_strtod(p, &end);
        if (end == p) {
            break;
        }
        xyz[n++] = v;
        p = end;
    }

    zend_string_release(str);
    return n >= 2;
}

/*
 * Transforms one point and writes array('x' => .., 'y' => .., 'z' => ..) or false into out.
 * tgt == NULL: src is a transformation (from proj_create_crs_to_crs*), applied forward.
 * tgt != NULL: legacy proj4 mode, src and tgt are two coordinate systems.
 */
static void php_proj_transform(PJ *src, PJ *tgt, const double *xyz, zval *out)
{
    PJ_COORD c;
    double x = xyz[0], y = xyz[1], z = xyz[2];

    /* PROJ keeps the last error on the object, so clear it before each point */
    proj_errno_reset(src);

    if (tgt == NULL) {
        c = proj_trans(src, PJ_FWD, proj_coord(x, y, z, 0));
        if (proj_errno(src) != 0 || c.xyz.x == HUGE_VAL) {
            ZVAL_FALSE(out);
            return;
        }
    } else {
        proj_errno_reset(tgt);

        if (proj_angular_output(src, PJ_FWD) == 1) {
            // source is already lat/lon -> deg2rad
            x = x > 180 ? 180 : (x < -180 ? -180 : x);
            y = y > 90 ? 89.9999999 : (y < -90 ? -89.9999999 : y);
            c = proj_coord(proj_torad(x), proj_torad(y), z, 0);
        } else {
            // transform source-coord to lat/lon
            c = proj_trans(src, PJ_INV, proj_coord(x, y, z, 0));
        }

        c = proj_trans(tgt, PJ_FWD, c);

        if (proj_errno(src) != 0 || proj_errno(tgt) != 0 || c.xyz.x == HUGE_VAL) {
            ZVAL_FALSE(out);
            return;
        }

        if (proj_angular_output(tgt, PJ_FWD) == 1) {
            c.xyz.x = proj_todeg(c.xyz.x);
            c.xyz.y = proj_todeg(c.xyz.y);
        }
    }

    array_init(out);
    add_assoc_double(out, "x", c.xyz.x);
    add_assoc_double(out, "y", c.xyz.y);
    add_assoc_double(out, "z", c.xyz.z);
}

/* Transforms every point of the array; a point that cannot be read or transformed becomes false. */
static void php_proj_transform_array(PJ *src, PJ *tgt, HashTable *points, zval *return_value)
{
    zval *zv, coord;
    double xyz[3];

    array_init_size(return_value, zend_hash_num_elements(points));

    ZEND_HASH_FOREACH_VAL(points, zv) {
        if (php_proj_read_point(zv, xyz)) {
            php_proj_transform(src, tgt, xyz, &coord);
        } else {
            ZVAL_FALSE(&coord);
        }
        add_next_index_zval(return_value, &coord);
    } ZEND_HASH_FOREACH_END();
}

/* Transforms the comma separated points of a string like "x y, x y z". */
static void php_proj_transform_string(PJ *src, PJ *tgt, zend_string *str, zval *return_value)
{
    zval points;
    zend_string *delimiter = zend_string_init(",", 1, 0);

    array_init(&points);
    php_explode(delimiter, str, &points, ZEND_LONG_MAX);
    php_proj_transform_array(src, tgt, Z_ARRVAL(points), return_value);

    zval_ptr_dtor(&points);
    zend_string_release(delimiter);
}

/* PROJ logs the reason only to stderr, which a web request does not show. */
static void php_proj_warn_create_failed(void)
{
    int err = proj_context_errno(PROJ_G(ctx));
    const char *msg = err ? proj_errno_string(err) : NULL;

    php_error_docref(NULL, E_WARNING, "%s", msg ? msg : "Could not create PROJ object");
}

/* Swaps P for a copy with GIS axis order (lon/lat, easting/northing) and frees the original. */
static PJ *php_proj_normalize(PJ *P)
{
    PJ *normalized = proj_normalize_for_visualization(PROJ_G(ctx), P);

    if (normalized == NULL) {
        return P;
    }
    proj_destroy(P);
    return normalized;
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
    PJ *P;
    zend_string *definition;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(definition)
    ZEND_PARSE_PARAMETERS_END();

    P = proj_create(PROJ_G(ctx), ZSTR_VAL(definition));

    if (0==P) {
        php_proj_warn_create_failed();
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

    zend_string *srid_from, *srid_to;
    zval *proj_area = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 3)
            Z_PARAM_STR(srid_from)
            Z_PARAM_STR(srid_to)
            Z_PARAM_OPTIONAL
            Z_PARAM_RESOURCE(proj_area)
    ZEND_PARSE_PARAMETERS_END();

    if (proj_area != NULL) {
        Area = (PJ_AREA*) zend_fetch_resource_ex(proj_area, PHP_PROJ_AREA_RES_NAME, proj_area_destructor);
        if (!Area ) {
            RETURN_FALSE;
        }
    }

    Proj = proj_create_crs_to_crs(PROJ_G(ctx), ZSTR_VAL(srid_from), ZSTR_VAL(srid_to), Area);

    if (0==Proj) {
        php_proj_warn_create_failed();
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(php_proj_normalize(Proj), proj_destructor));
}

/**
 * @param resource source_crs
 * @param resource target_crs
 * @param resource area of interest
 * @param string option for PROJ, e.g. "ALLOW_BALLPARK=NO"
 * @return resource
 */
ZEND_BEGIN_ARG_INFO(proj_create_crs_to_crs_from_pj_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, src_crs)
    ZEND_ARG_INFO(0, tgtc_crs)
    ZEND_ARG_INFO(0, proj_area)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_create_crs_to_crs_from_pj)
{
    PJ *Proj;
    PJ_AREA *Area = NULL;

    char *options = NULL;
    size_t options_len;
    const char *option_list[2] = {NULL, NULL};

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

    if (proj_area != NULL) {
        Area = (PJ_AREA*) zend_fetch_resource_ex(proj_area, PHP_PROJ_AREA_RES_NAME, proj_area_destructor);
        if (!Area ) {
            RETURN_FALSE;
        }
    }

    option_list[0] = options;
    Proj = proj_create_crs_to_crs_from_pj(PROJ_G(ctx), srcP, tgtP, Area, options ? option_list : NULL);

    if (0==Proj) {
        php_proj_warn_create_failed();
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(php_proj_normalize(Proj), proj_destructor));
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
ZEND_BEGIN_ARG_INFO(proj_free_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, Proj)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_free)
{
    zval *zpj;
    PJ *P;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();

    P = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj_destructor);

    if (P != NULL) {
        /* close statt delete: $pj haelt noch eine Referenz, delete gibt die Resource frei -> use-after-free */
        zend_list_close(Z_RES_P(zpj));
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
    double xyz[3] = {0, 0, 0};
    zval *src, *tgt;
    PJ *srcProj, *tgtProj;

    ZEND_PARSE_PARAMETERS_START(4, 5)
            Z_PARAM_RESOURCE(src)
            Z_PARAM_RESOURCE(tgt)
            Z_PARAM_DOUBLE(xyz[0])
            Z_PARAM_DOUBLE(xyz[1])
            Z_PARAM_OPTIONAL
            Z_PARAM_DOUBLE(xyz[2])
    ZEND_PARSE_PARAMETERS_END();

    srcProj = (PJ*) zend_fetch_resource_ex(src, PHP_PROJ_RES_NAME, proj_destructor);
    tgtProj = (PJ*) zend_fetch_resource_ex(tgt, PHP_PROJ_RES_NAME, proj_destructor);

    if (srcProj == NULL || tgtProj == NULL) {
        RETURN_FALSE;
    }

    php_proj_transform(srcProj, tgtProj, xyz, return_value);
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
    double xyz[3] = {0, 0, 0};
    zval *Proj;
    PJ *P;

    ZEND_PARSE_PARAMETERS_START(3, 4)
            Z_PARAM_RESOURCE(Proj)
            Z_PARAM_DOUBLE(xyz[0])
            Z_PARAM_DOUBLE(xyz[1])
            Z_PARAM_OPTIONAL
            Z_PARAM_DOUBLE(xyz[2])
    ZEND_PARSE_PARAMETERS_END();

    P = (PJ*) zend_fetch_resource_ex(Proj, PHP_PROJ_RES_NAME, proj_destructor);

    if (P == NULL) {
        RETURN_FALSE;
    }

    php_proj_transform(P, NULL, xyz, return_value);
}

/**
 *
 * @param resource Proj
 * @param array points-array
 * @return mixed array with array ('x' => x, 'y' => y 'z' => z) or false per point; false on error
 */
ZEND_BEGIN_ARG_INFO(proj_transform_array_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, Proj)
    ZEND_ARG_INFO(0, points)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_transform_array)
{
    zval *src, *xyz_arr_p;
    PJ *P;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_RESOURCE(src)
            Z_PARAM_ARRAY(xyz_arr_p)
    ZEND_PARSE_PARAMETERS_END();

    P = (PJ*) zend_fetch_resource_ex(src, PHP_PROJ_RES_NAME, proj_destructor);

    if (P == NULL) {
        RETURN_FALSE;
    }

    php_proj_transform_array(P, NULL, Z_ARRVAL_P(xyz_arr_p), return_value);
}

/**
 *
 * @param resource srcDefn
 * @param resource tgtDefn
 * @param array points-array
 * @return mixed array with array ('x' => x, 'y' => y 'z' => z) or false per point; false on error
 */
ZEND_BEGIN_ARG_INFO(proj4_transform_array_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, srcProj)
    ZEND_ARG_INFO(0, tgtProj)
    ZEND_ARG_INFO(0, points)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj4_transform_array)
{
    zval *srcDefn, *tgtDefn, *xyz_arr_p;
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

    php_proj_transform_array(srcProj, tgtProj, Z_ARRVAL_P(xyz_arr_p), return_value);
}

/**
 * @param resource srcDefn
 * @param resource tgtDefn
 * @param string geometry
 * @return mixed array with array ('x' => x, 'y' => y 'z' => z) or false per point; false on error
 */
ZEND_BEGIN_ARG_INFO(proj4_transform_string_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, srcProj)
    ZEND_ARG_INFO(0, tgtProj)
    ZEND_ARG_INFO(0, geometry)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj4_transform_string)
{
    zval *srcDefn, *tgtDefn;
    zend_string *str;
    PJ *srcProj, *tgtProj;

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

    php_proj_transform_string(srcProj, tgtProj, str, return_value);
}

/**
 * @param resource Proj
 * @param string geometry
 * @return mixed array with array ('x' => x, 'y' => y 'z' => z) or false per point; false on error
 */
ZEND_BEGIN_ARG_INFO(proj_transform_string_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, Proj)
    ZEND_ARG_INFO(0, geometry)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_transform_string)
{
    zval *Proj;
    zend_string *str;
    PJ *P;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_RESOURCE(Proj)
            Z_PARAM_STR(str)
    ZEND_PARSE_PARAMETERS_END();

    P = (PJ*) zend_fetch_resource_ex(Proj, PHP_PROJ_RES_NAME, proj_destructor);

    if (P == NULL) {
        RETURN_FALSE;
    }

    php_proj_transform_string(P, NULL, str, return_value);
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
    double lon1, lat1, lon2, lat2;
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
    double lon1, lat1, z1, lon2, lat2, z2;
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

    PJ_COORD c1 = proj_coord(lon1, lat1, z1, 0);
    PJ_COORD c2 = proj_coord(lon2, lat2, z2, 0);

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
    double x1, y1, x2, y2;

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
    double x1, y1, z1, x2, y2, z2;

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

    if (pj == NULL) {
        RETURN_FALSE;
    }

    RETURN_BOOL(proj_angular_output(pj, PJ_FWD) == 1);
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
    PJ *pj, *target;
    PJ_PROJ_INFO info;
    PJ_LOG_LEVEL log_level;
    int geocent;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();

    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj_destructor);

    if (pj == NULL) {
        RETURN_FALSE;
    }

    // proj-string "+proj=geocent ..."
    info = proj_pj_info(pj);
    if (info.id != NULL && strcmp(info.id, "geocent") == 0) {
        RETURN_TRUE;
    }

    if (proj_get_type(pj) == PJ_TYPE_GEOCENTRIC_CRS) {
        RETURN_TRUE;
    }

    // transformation: look at its target CRS; objects without one would log an error to stderr
    log_level = proj_log_level(PROJ_G(ctx), PJ_LOG_NONE);
    target = proj_get_target_crs(PROJ_G(ctx), pj);
    proj_log_level(PROJ_G(ctx), log_level);

    geocent = target != NULL && proj_get_type(target) == PJ_TYPE_GEOCENTRIC_CRS;
    if (target != NULL) {
        proj_destroy(target);
    }

    RETURN_BOOL(geocent);
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

    if (pj == NULL) {
        RETURN_FALSE;
    }

    result = proj_pj_info(pj);
    RETURN_STRING(result.definition ? result.definition : "");
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
    add_assoc_string(return_value, "definition", result.definition ? result.definition : "");
    add_assoc_string(return_value, "description", result.description ? result.description : "");
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

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_RESOURCE(zpj)
    ZEND_PARSE_PARAMETERS_END();

    pj = (PJ*) zend_fetch_resource_ex(zpj, PHP_PROJ_RES_NAME, proj_destructor);

    if (pj == NULL) {
        RETURN_NULL();
    }

    RETURN_LONG(proj_errno(pj));
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
    const char *msg;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_LONG(errorCode)
    ZEND_PARSE_PARAMETERS_END();

    if( errorCode == 0 ) {
        RETURN_EMPTY_STRING();
    }

    msg = proj_errno_string((int)errorCode);
    RETURN_STRING(msg ? msg : "");
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
    PJ_INFO info = proj_info();

    array_init(return_value);
    add_assoc_string(return_value, "version", (char *) info.version);
    add_assoc_long(return_value, "major", info.major);
    add_assoc_long(return_value, "minor", info.minor);
    add_assoc_long(return_value, "patch", info.patch);
    add_assoc_string(return_value, "release", (char *) info.release);
    add_assoc_string(return_value, "searchpath", (char *) info.searchpath);
}

/**
 * @hint PROJ 7.1 replaced the unit list by the database, so the fields differ by PROJ version
 * @see https://proj.org/development/reference/datatypes.html#c.PJ_UNITS
 * @return array
 */
ZEND_FUNCTION(proj_list_units)
{
    zval tmp;

    array_init(return_value);

#if PROJ_VERSION_MAJOR < 7
    const PJ_UNITS *unit;

    for (unit=proj_list_units(); unit->id ; ++unit) {
        array_init(&tmp);
        add_assoc_string(&tmp, "name", (char *) unit->name);
        add_assoc_string(&tmp, "to_meter", (char *) unit->to_meter);
        add_assoc_double(&tmp, "factor", unit->factor);
        add_assoc_zval(return_value, unit->id, &tmp);
    }
#else
    int result_count = 0;
    PROJ_UNIT_INFO** units;
    /* PROJ schreibt die Anzahl durch den Zeiger */
    units = proj_get_units_from_database(PROJ_G(ctx), NULL, "linear", 0, &result_count);

    for (int i = 0; units && units[i]; i++) {
        array_init(&tmp);
        add_assoc_string(&tmp, "name", units[i]->name);
        add_assoc_string(&tmp, "auth_name", units[i]->auth_name);
        add_assoc_string(&tmp, "category", units[i]->category);
        add_assoc_double(&tmp, "conv_factor", units[i]->conv_factor);
        add_assoc_zval(return_value, units[i]->code, &tmp);
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
        array_init(&tmp);
        add_assoc_string(&tmp, "name", (char *) ellps->name);
        add_assoc_string(&tmp, "major", (char *) ellps->major);
        add_assoc_string(&tmp, "ell", (char *) ellps->ell);
        add_assoc_zval(return_value, ellps->id, &tmp);
    }
}

/**
 * Set PROJ resource search paths for the current request.
 * seit PROJ 6.0
 *
 * PHP:
 * proj_context_set_search_paths(array $paths): bool
 */
ZEND_BEGIN_ARG_INFO(proj_context_set_search_paths_args, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, paths)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(proj_context_set_search_paths)
{
    zval *zpaths;
    HashTable *ht;
    zval *entry;

    const char **paths = NULL;
    int count;
    int i = 0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(zpaths)
    ZEND_PARSE_PARAMETERS_END();

    ht = Z_ARRVAL_P(zpaths);
    count = (int) zend_hash_num_elements(ht);

    /*
     * Leeres Array:
     * explizite Suchpfade wieder zurücksetzen.
     */
    if (count == 0) {
        proj_context_set_search_paths(PROJ_G(ctx), 0, NULL);
        PROJ_G(paths_changed) = 0;
        RETURN_TRUE;
    }

    paths = safe_emalloc(count, sizeof(char *), 0);

    ZEND_HASH_FOREACH_VAL(ht, entry) {

        if (Z_TYPE_P(entry) != IS_STRING) {
            efree(paths);

            php_error_docref(
                NULL,
                E_WARNING,
                "All search paths must be strings"
            );

            RETURN_FALSE;
        }

        paths[i++] = Z_STRVAL_P(entry);

    } ZEND_HASH_FOREACH_END();

    proj_context_set_search_paths(
        PROJ_G(ctx),
        count,
        paths
    );
    /* der Kontext lebt laenger als der Request: RSHUTDOWN setzt die Pfade zurueck */
    PROJ_G(paths_changed) = 1;

    efree(paths);

    RETURN_TRUE;
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
    ZEND_FE(proj_context_set_search_paths, proj_context_set_search_paths_args)
    ZEND_FE(proj_list_units, Proj_method_no_args)
    ZEND_FE(proj_list_ellps, Proj_method_no_args)
    ZEND_FE(proj_area_create, Proj_method_no_args)
    ZEND_FE(proj_area_set_bbox, proj_area_set_bbox_args)
    ZEND_FE(proj_distance_lp, proj_distance_lp_args)
    ZEND_FE(proj_distance_lpz, proj_distance_lpz_args)
    ZEND_FE(proj_distance_xy, proj_distance_xy_args)
    ZEND_FE(proj_distance_xyz, proj_distance_xyz_args)
    ZEND_FE(proj_free, proj_free_args)
    ZEND_FE_END
};

static PHP_GINIT_FUNCTION(proj)
{
#if defined(COMPILE_DL_PROJ) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
#ifdef ZTS
    /* ein PROJ-Kontext darf nicht von zwei Threads gleichzeitig benutzt werden.
       shortcut: proj_get_info()['searchpath'] zeigt unter ZTS den Default-Kontext, nicht diesen */
    proj_globals->ctx = proj_context_create();
#else
    proj_globals->ctx = PJ_DEFAULT_CTX;
#endif
    proj_globals->paths_changed = 0;
}

static PHP_GSHUTDOWN_FUNCTION(proj)
{
#ifdef ZTS
    if (proj_globals->ctx != NULL) {
        proj_context_destroy(proj_globals->ctx);
    }
#endif
}

zend_module_entry proj_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_PROJ_EXTNAME,
    proj_functions,
    PHP_MINIT(proj),           /* module init function */
    NULL,                      /* module shutdown function */
    NULL,                      /* request init function */
    PHP_RSHUTDOWN(proj),       /* request shutdown function */
    PHP_MINFO(proj),           /* module info function */
    PHP_PROJ_VERSION,
    PHP_MODULE_GLOBALS(proj),
    PHP_GINIT(proj),
    PHP_GSHUTDOWN(proj),
    NULL,                      /* post deactivate */
    STANDARD_MODULE_PROPERTIES_EX
};

static void php_proj_dtor(zend_resource *resource) {
    PJ *proj = (PJ*) resource->ptr;
    if (proj != NULL) {
        proj_destroy(proj);
    }
}

static void php_proj_area_dtor(zend_resource *resource) {
    PJ_AREA *area = (PJ_AREA*) resource->ptr;
    if (area != NULL) {
        proj_area_destroy(area);
    }
}

PHP_MINIT_FUNCTION(proj) {
    proj_destructor = zend_register_list_destructors_ex(php_proj_dtor, NULL, PHP_PROJ_RES_NAME, module_number);
    proj_area_destructor = zend_register_list_destructors_ex(php_proj_area_dtor, NULL, PHP_PROJ_AREA_RES_NAME, module_number);
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(proj) {
    if (PROJ_G(paths_changed)) {
        proj_context_set_search_paths(PROJ_G(ctx), 0, NULL);
        PROJ_G(paths_changed) = 0;
    }
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

#ifdef COMPILE_DL_PROJ
ZEND_GET_MODULE(proj)
#endif
