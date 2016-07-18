#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <php_main.h>
#include <php_ini.h>
#include <ext/standard/info.h>
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
    PHP_MINIT(proj4),
    PHP_MSHUTDOWN(proj4),
    PHP_RINIT(proj4),
    NULL,
    PHP_MINFO(proj4),
    PHP_PROJ4_VERSION,
    STANDARD_MODULE_PROPERTIES
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

    //ZEND_REGISTER_RESOURCE(return_value, pj_merc, le_proj4);
    RETURN_RES(zend_register_resource(pj_merc, le_proj4));
}

static void tellMeWhatYouAre(zval *arg)
{
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
            php_printf("Array");
            break;
        default:
            php_printf("Unknown");
    }
    php_printf("\n");
}
/*
ZEND_FUNCTION(pj_transform) {
    int p;
    zend_long point_count, point_offset;
    zval *zx, *zy, *zz;
    zval *zpj_latlong, *zpj_merc;
    projPJ srcProj, tgtProj;

    zval *x_data, *y_data, *z_data;
    HashTable *x_array, *y_array, *z_array;
    HashPosition x_position, y_position, z_position;

    int x_count, y_count, z_count = 0, max_count = -1, current_array_position = 1, added_array_elements = 0;
    zend_bool srcProjIsLatLon, tgtProjIsLatLon;

    double *x_input_array;
    double *y_input_array;
    double *z_input_array;

    int current_input_array_index = 0;
    //double current_z = 0;

    int i;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrllaa|a", &zpj_latlong, &zpj_merc, &point_count, &point_offset, &zx, &zy, &zz) == FAILURE) {
        RETURN_FALSE;
    }

    srcProj = (projPJ*)zend_fetch_resource_ex(zpj_latlong, PHP_PROJ4_RES_NAME, le_proj4);
    tgtProj = (projPJ*)zend_fetch_resource_ex(zpj_merc, PHP_PROJ4_RES_NAME, le_proj4);
    
    if( srcProj == NULL || tgtProj == NULL ) {
        RETURN_FALSE;
    }

    if (point_offset < 0) {
        point_offset = 0;
    }
    if (point_count < 0) {
        point_count = 0;
    }

    srcProjIsLatLon = pj_is_latlong(srcProj);
    tgtProjIsLatLon = pj_is_latlong(tgtProj);

    x_array = Z_ARR_P(zx);
    y_array = Z_ARR_P(zy);
    x_count = zend_hash_num_elements(x_array);
    y_count = zend_hash_num_elements(y_array);
    if (Z_TYPE_P(zz) == IS_ARRAY) {
        php_printf("zz is an array \n");
        z_array = Z_ARR_P(zz);
        z_count = zend_hash_num_elements(z_array);
        php_printf("%d element(s) in z-array\n", z_count);
    } else {
        //tellMeWhatYouAre(zz);
        php_printf("no z given");
    }
    php_printf("%d element(s) in x-array\n", x_count);
    php_printf("%d element(s) in y-array\n", y_count);

    if (x_count > 0 && y_count > 0) {
        max_count = (x_count <= y_count) ? x_count : y_count;
        if (z_count > 0 && z_count < max_count) {
            max_count = z_count;
        }
        if (point_count < max_count) {
            max_count = point_count;
        }
        
        php_printf("checking if z_count > 0 ?");
        if (z_count > 0) {
            php_printf("yes\n");
            zend_hash_internal_pointer_reset_ex(z_array, &z_position);
        } else {
            php_printf("no\n");
        };
        
        zend_hash_internal_pointer_reset_ex(x_array, &x_position);
        zend_hash_internal_pointer_reset_ex(y_array, &y_position);
        php_printf("start fetching data from arrays\n");
        for (;
            (x_data = zend_hash_get_current_data_ex(x_array, &x_position)) != NULL &&
            (y_data = zend_hash_get_current_data_ex(y_array, &y_position)) != NULL;
            zend_hash_move_forward_ex(x_array, &x_position),
            zend_hash_move_forward_ex(y_array, &y_position),
            current_array_position++) {
            
            php_printf("data fetched from array\n");
            
            if (current_array_position < point_offset + 1) {
                continue;
            }
            if (added_array_elements >= point_count) {
                break;
            }

            if ((Z_TYPE_P(x_data) == IS_DOUBLE || Z_TYPE_P(x_data) == IS_LONG || Z_TYPE_P(x_data) == IS_STRING) &&
                (Z_TYPE_P(y_data) == IS_DOUBLE || Z_TYPE_P(y_data) == IS_LONG || Z_TYPE_P(y_data) == IS_STRING)) {
                if (z_count > 0) {
                    php_printf("fetching data from z_array \n");
                    
                    if ( (z_data = zend_hash_get_current_data_ex(z_array, &z_position)) != NULL ) {
                        php_printf("convert_to_double_ex");
                        if (Z_TYPE_P(z_data) != IS_DOUBLE) {
                            convert_to_double_ex(z_data);
                        }
                        //current_z = Z_DVAL_P(z_data);
                        zend_hash_move_forward_ex(z_array, &z_position);
                    } else {
                        RETURN_FALSE;
                    }
                } else {
                    php_printf("z_count = 0\n");
                }

                if (Z_TYPE_P(x_data) != IS_DOUBLE) {
                    convert_to_double_ex(x_data);
                }
                if (Z_TYPE_P(y_data) != IS_DOUBLE) {
                    convert_to_double_ex(y_data);
                }

//        tellMeWhatYouAre(x_data);
//        tellMeWhatYouAre(y_data);
//        tellMeWhatYouAre(z_data);
                x_input_array[current_input_array_index] = Z_DVAL_P(x_data);
                y_input_array[current_input_array_index] = Z_DVAL_P(y_data);
                z_input_array[current_input_array_index] = Z_DVAL_P(z_data);
//        RETURN_FALSE;
//                x_input_array[current_input_array_index] = (srcProjIsLatLon ? DEG_TO_RAD * Z_DVAL_P(x_data) : Z_DVAL_P(x_data));
//                y_input_array[current_input_array_index] = (srcProjIsLatLon ? DEG_TO_RAD * Z_DVAL_P(y_data) : Z_DVAL_P(y_data));
//                z_input_array[current_input_array_index] = Z_DVAL_P(z_data);
                current_input_array_index++;
                added_array_elements++;
                RETURN_FALSE;
            } else {
                php_printf("error while fetching data?\n");
                RETURN_FALSE;
            }
        }
php_printf("bla\n");

        p = pj_transform(srcProj, tgtProj, max_count, 0, x_input_array, y_input_array, z_input_array);
        if (p == 0) {
            zval x_array_element, y_array_element, z_array_element;
            
            array_init(&x_array_element);
            array_init(&y_array_element);
            array_init(&z_array_element);

            array_init(return_value);

            for (i = 0; i < max_count; i++) {
                add_next_index_double(&x_array_element, (tgtProjIsLatLon ? RAD_TO_DEG * x_input_array[i] : x_input_array[i]));
                add_next_index_double(&y_array_element, (tgtProjIsLatLon ? RAD_TO_DEG * y_input_array[i] : y_input_array[i]));
                add_next_index_double(&z_array_element, z_input_array[i]);
            }

            add_assoc_zval(return_value, "x", &x_array_element);
            add_assoc_zval(return_value, "y", &y_array_element);
            add_assoc_zval(return_value, "z", &z_array_element);
        }
    }

    if (p != 0) {
        RETURN_DOUBLE(p);
    }
}*/

/**
 * 
 * @param srcProj
 * @param tgtProj
 * @param str
 * @return array
 */
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
        add_assoc_double(&return_value, "x", x);
        add_assoc_double(&return_value, "y", y);
        add_assoc_double(&return_value, "z", z);
//        add_index_double(&return_value, 0, x);
//        add_index_double(&return_value, 1, y);
//        add_index_double(&return_value, 2, z);
    }

    return return_value;
}

/**
 * 
 * @param srcProj
 * @param tgtProj
 * @param str
 * @return array
 */
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

static zval transformCoordArray_static(zval *xy_arr_p, projPJ srcProj, projPJ tgtProj, projPJ wgsProj, zval *projViaWgs84 )
{
    zval coord;
    zval *x, *y, z, *t;
    
    HashTable *xyz_hash = Z_ARR_P(xy_arr_p);
    array_init(&coord);
    
    if (NULL != (x = zend_hash_index_find(xyz_hash, 0)) && 
        NULL != (y = zend_hash_index_find(xyz_hash, 1))) {
        
        if( NULL == (t = zend_hash_index_find(xyz_hash, 2)) ) {
                //php_printf("no value for z found \n");
                ZVAL_DOUBLE(&z, 0.0);
                //tellMeWhatYouAre(&z);
            } else {
                //php_printf("value for z found \n");
                ZVAL_COPY_VALUE(&z, t);
                zval_ptr_dtor(t);
                convert_to_double_ex(&z);
                //tellMeWhatYouAre(&z);
            }
        
        convert_to_double_ex(x);
        convert_to_double_ex(y);
        
        if (projViaWgs84) {
            //php_printf("transformiere über WGS84 \n");
            coord = projCoordViaWGS84_static(srcProj, tgtProj, wgsProj, Z_DVAL_P(x), Z_DVAL_P(y), Z_DVAL(z) );
        } else {
            //php_printf("transformiere direkt \n");
            coord = projCoord_static(srcProj, tgtProj, Z_DVAL_P(x), Z_DVAL_P(y), Z_DVAL(z));
        }
    }
    
    return coord;
}


/**
 * 
 * @param execute_data
 * @param return_value
 */
ZEND_FUNCTION(pj_transform_array) {
    /* method-params */
    zval *x, *y, z, *t;
    zval *xyz_arr_p, xyz_arr, *zv, coord;
    zend_string *delimiter;
    HashTable *xyz_hash;
    
    /* user-params */
    zval *srcDefn, *tgtDefn;
    
    /* projection-params */
    projPJ wgsProj, srcProj, tgtProj;
    zend_bool projViaWgs84;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rra", &srcDefn, &tgtDefn, &xyz_arr_p) == FAILURE) {
        RETURN_FALSE;
    }
    
    srcProj = (projPJ*) zend_fetch_resource_ex(srcDefn, PHP_PROJ4_RES_NAME, le_proj4);
    tgtProj = (projPJ*) zend_fetch_resource_ex(tgtDefn, PHP_PROJ4_RES_NAME, le_proj4);
    
    if( srcProj == NULL || tgtProj == NULL ) {
        RETURN_FALSE;
    }
    projViaWgs84 = !pj_is_latlong(srcProj) && !pj_is_latlong(tgtProj);
    
    /*
     * make sure to go over WGS84 for all transformation between non-geographic coordinate systems
     */
    if (projViaWgs84) {
        wgsProj = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
        if (wgsProj == NULL) {
            RETURN_FALSE;
        }
    }
    
    array_init(return_value);
    ZVAL_DOUBLE(&z, 0.0);
    delimiter = zend_string_init(" ", 1, 0);
    HashTable *pts_hash = Z_ARR_P(xyz_arr_p);
    ZEND_HASH_FOREACH_VAL(pts_hash, zv) {
            
        convert_to_string_ex(zv);
        //php_printf("zv: %s \n", Z_STRVAL_P(zv));

        // in x,y,z zerteilen
        array_init(&xyz_arr);
        php_explode(delimiter, Z_STR_P(zv), &xyz_arr, LONG_MAX);

        if (Z_TYPE(xyz_arr) != IS_ARRAY) {
            RETURN_FALSE;
        }

        xyz_hash = Z_ARR_P(&xyz_arr);

        if (NULL != (x = zend_hash_index_find(xyz_hash, 0)) && 
            NULL != (y = zend_hash_index_find(xyz_hash, 1)) ) {

            if( NULL == (t = zend_hash_index_find(xyz_hash, 2)) ) {
                //php_printf("no value for z found \n");
                ZVAL_DOUBLE(&z, 0.0);
                //tellMeWhatYouAre(&z);
            } else {
                //php_printf("value for z found \n");
                ZVAL_COPY_VALUE(&z, t);
                zval_ptr_dtor(t);
                convert_to_double_ex(&z);
                //tellMeWhatYouAre(&z);
            }
            convert_to_double_ex(x);
            convert_to_double_ex(y);

            if (projViaWgs84) {
                //php_printf("transformiere über WGS84 \n");
                coord = projCoordViaWGS84_static(srcProj, tgtProj, wgsProj, Z_DVAL_P(x), Z_DVAL_P(y), Z_DVAL(z) );
            } else {
                //php_printf("transformiere direkt \n");
                coord = projCoord_static(srcProj, tgtProj, Z_DVAL_P(x), Z_DVAL_P(y), Z_DVAL(z));
            }
            add_next_index_zval(return_value, &coord );
        }

    } ZEND_HASH_FOREACH_END();
    
    zval_ptr_dtor(&z);
    zend_string_release(delimiter);
    zval_ptr_dtor(&xyz_arr);
    
    if (projViaWgs84) {
        pj_free(wgsProj);
    }
}


/*ZEND_FUNCTION(pj_test_z) {
    zval z, *t;
    zval *xyz_arr_p;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &xyz_arr_p) == FAILURE) {
        RETURN_FALSE;
    }
    
    ZVAL_DOUBLE(&z, 0.0);
    
    HashTable *xyz_hash = Z_ARR_P(xyz_arr_p);
    t = zend_hash_index_find(xyz_hash, 2);
    if( t == NULL ) {
        php_printf("no value for z found \n");
        tellMeWhatYouAre(&z);
    } else {
        php_printf("value for z found \n");
        tellMeWhatYouAre(t);
        ZVAL_COPY_VALUE(&z, t);
        tellMeWhatYouAre(&z);
        zval_ptr_dtor(t);
    }
    
    //zval_dtor(t);
    zval_ptr_dtor(&z);
    RETURN_ZVAL(&z, 1, 1);
}*/

/*ZEND_FUNCTION(pj_test_z) {
    zval z, t;
    zval *xyz_arr_p;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &xyz_arr_p) == FAILURE) {
        RETURN_FALSE;
    }
    
    ZVAL_DOUBLE(&z, 1.0);
    
    HashTable *xyz_hash = Z_ARR_P(xyz_arr_p);
    t = *(zend_hash_index_find(xyz_hash, 2));
    
    tellMeWhatYouAre(&t);
    tellMeWhatYouAre(&z);

    ZVAL_COPY_VALUE(&z, &t);
    
    zval_ptr_dtor(&t);
    zval_ptr_dtor(&z);
    RETURN_ZVAL(&z, 1, 1);
}*/

/**
 * 
 * @param execute_data
 * @param return_value
 */
ZEND_FUNCTION(pj_transform_string) {
    
    /* user-params */
    zval *srcDefn, *tgtDefn;
    zend_string *str;
    
    /* projection-params */
    projPJ wgsProj, srcProj, tgtProj;
    zend_bool projViaWgs84;
    
    /* coord-params */
    zval pts_arr, xyz_arr;
    zend_string *delimiter, *delimiter2;
    HashTable *pts_hash, *xyz_hash;
    zval coord;
    zval *zv;
    
    zval *x, *y, z, *t;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrS", &srcDefn, &tgtDefn, &str) == FAILURE) {
        RETURN_FALSE;
    }

    srcProj = (projPJ*) zend_fetch_resource_ex(srcDefn, PHP_PROJ4_RES_NAME, le_proj4);
    tgtProj = (projPJ*) zend_fetch_resource_ex(tgtDefn, PHP_PROJ4_RES_NAME, le_proj4);
    
    if( srcProj == NULL || tgtProj == NULL ) {
        RETURN_FALSE;
    }
    projViaWgs84 = !pj_is_latlong(srcProj) && !pj_is_latlong(tgtProj);
    
    /*
     * make sure to go over WGS84 for all transformation between non-geographic coordinate systems
     */
    if (projViaWgs84) {
        wgsProj = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
        if (wgsProj == NULL) {
            RETURN_FALSE;
        }
    }
    
    delimiter = zend_string_init(",", 1, 0);
    delimiter2 = zend_string_init(" ", 1, 0);
    
    array_init(return_value);
    array_init(&pts_arr);
    
    // in einzelne Koordinaten zerteilen
    php_explode(delimiter, str, &pts_arr, LONG_MAX);
    
    if (Z_TYPE(pts_arr) == IS_ARRAY) {
        pts_hash = Z_ARR_P(&pts_arr);
        ZEND_HASH_FOREACH_VAL(pts_hash, zv) {
            
            convert_to_string_ex(zv);
            //php_printf("zv: %s \n", Z_STRVAL_P(zv));
            
            // in x,y,z zerteilen
            array_init(&xyz_arr);
            php_explode(delimiter2, Z_STR_P(zv), &xyz_arr, LONG_MAX);

            if (Z_TYPE(xyz_arr) != IS_ARRAY) {
                RETURN_FALSE;
            }

            xyz_hash = Z_ARR_P(&xyz_arr);

            if (NULL != (x = zend_hash_index_find(xyz_hash, 0)) && 
                NULL != (y = zend_hash_index_find(xyz_hash, 1)) ) {
                
                if( NULL == (t = zend_hash_index_find(xyz_hash, 2)) ) {
                    //php_printf("no value for z found \n");
                    ZVAL_DOUBLE(&z, 0.0);
                    //tellMeWhatYouAre(&z);
                } else {
                    //php_printf("value for z found \n");
                    ZVAL_COPY_VALUE(&z, t);
                    zval_ptr_dtor(t);
                    convert_to_double_ex(&z);
                    //tellMeWhatYouAre(&z);
                }
                convert_to_double_ex(x);
                convert_to_double_ex(y);
                
                if (projViaWgs84) {
                    //php_printf("transformiere über WGS84 \n");
                    coord = projCoordViaWGS84_static(srcProj, tgtProj, wgsProj, Z_DVAL_P(x), Z_DVAL_P(y), Z_DVAL(z) );
                } else {
                    //php_printf("transformiere direkt \n");
                    coord = projCoord_static(srcProj, tgtProj, Z_DVAL_P(x), Z_DVAL_P(y), Z_DVAL(z));
                }
                add_next_index_zval(return_value, &coord );
            }
            
        } ZEND_HASH_FOREACH_END();
    }
    
    // cleanup
    zend_string_release(delimiter);
    zend_string_release(delimiter2);
    zval_ptr_dtor(&pts_arr);
    zval_ptr_dtor(&xyz_arr);
    if (projViaWgs84) {
        pj_free(wgsProj);
    }
}

/**
 * 
 * @param object srcDefn
 * @param object tgtDefn
 * @param float x
 * @param float y
 * @param float z
 * @return array ('x' => x, 'y' => y 'z' => z)
 */
ZEND_FUNCTION(pj_transform_point) {

    double x, y, z = 0;
    zval *srcDefn, *tgtDefn;
    //zend_resource *srcDefn, *tgtDefn;
    projPJ wgsProj, srcProj, tgtProj;
    zend_bool projViaWgs84;
    int p;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrdd|d", &srcDefn, &tgtDefn, &x, &y, &z) == FAILURE) {
        RETURN_FALSE;
    }
    
    srcProj = (projPJ*) zend_fetch_resource_ex(srcDefn, PHP_PROJ4_RES_NAME, le_proj4);
    tgtProj = (projPJ*) zend_fetch_resource_ex(tgtDefn, PHP_PROJ4_RES_NAME, le_proj4);
    
    // if (!srcProj || !tgtProj) {
    if( srcProj == NULL || tgtProj == NULL ) {
        RETURN_FALSE;
    }

    projViaWgs84 = !pj_is_latlong(srcProj) && !pj_is_latlong(tgtProj);

    /*
     * make sure to go over WGS84 for all transformation between non-geographic coordinate systems
     */
    if (projViaWgs84) {

        wgsProj = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
        if (wgsProj == NULL) {
            RETURN_FALSE;
        }

        pj_transform(srcProj, wgsProj, 1, 0, &x, &y, &z);
        x *= RAD_TO_DEG;
        y *= RAD_TO_DEG;
        srcProj = wgsProj;
    }


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

        array_init(return_value);
        add_assoc_double(return_value, "x", x);
        add_assoc_double(return_value, "y", y);
        add_assoc_double(return_value, "z", z);
    }

    if (projViaWgs84) {
        pj_free(wgsProj);
    }
}

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

    //ZEND_FETCH_RESOURCE(pj, projPJ*, &zpj, -1, PHP_PROJ4_RES_NAME, le_proj4);
    pj = (projPJ*) zend_fetch_resource_ex(zpj, PHP_PROJ4_RES_NAME, le_proj4);

    if (pj_is_latlong(pj)) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(pj_is_geocent) {
    zval *zpj;
    projPJ pj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zpj) == FAILURE) {
        RETURN_FALSE;
    }

    //ZEND_FETCH_RESOURCE(pj, projPJ*, &zpj, -1, PHP_PROJ4_RES_NAME, le_proj4);
    pj = (projPJ*) zend_fetch_resource_ex(zpj, PHP_PROJ4_RES_NAME, le_proj4);
    
    if (pj_is_geocent(pj)) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(pj_get_def) {
    zend_long options = 0;
    char *result;
    zval *zpj;
    projPJ pj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &zpj, &options) == FAILURE) {
        RETURN_FALSE;
    }

    //ZEND_FETCH_RESOURCE(pj, projPJ*, &zpj, -1, PHP_PROJ4_RES_NAME, le_proj4);
    pj = (projPJ*) zend_fetch_resource_ex(zpj, PHP_PROJ4_RES_NAME, le_proj4);

    if (pj != NULL && pj) {
        result = pj_get_def(pj, options);

        RETURN_STRING(result);
    }

    RETURN_FALSE;
}

ZEND_FUNCTION(pj_latlong_from_proj) {
    zval *zpj_in;
    projPJ pj_in, pj;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zpj_in) == FAILURE) {
        RETURN_FALSE;
    }

    //ZEND_FETCH_RESOURCE(pj_in, projPJ*, &zpj_in, -1, PHP_PROJ4_RES_NAME, le_proj4);
    pj_in = (projPJ*) zend_fetch_resource_ex(zpj_in, PHP_PROJ4_RES_NAME, le_proj4);
    
    if (pj_in != NULL && pj_in) {
        pj = pj_latlong_from_proj(pj_in);

        if (pj == NULL) {
            RETURN_FALSE;
        }

        //ZEND_REGISTER_RESOURCE(return_value, pj, le_proj4);
        RETURN_RES(zend_register_resource(pj, le_proj4));
    } else {
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(pj_deallocate_grids) {
    pj_deallocate_grids();
}

ZEND_FUNCTION(pj_get_errno_ref) {
    RETURN_LONG(*pj_get_errno_ref());
}

ZEND_FUNCTION(pj_strerrno) {
    zend_long error_code;
    char *result;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &error_code) == FAILURE) {
        RETURN_FALSE;
    }

    result = pj_strerrno(error_code);

    RETURN_STRING(result);
}

ZEND_FUNCTION(pj_get_release) {
    const char *result;

    result = pj_get_release();

    RETURN_STRING(result);
}

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
