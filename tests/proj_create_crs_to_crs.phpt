--TEST--
proj_create_crs_to_crs() function - basic test for proj_create_crs_to_crs()
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
}
?>
--FILE--
<?php
$pj = proj_create_crs_to_crs('EPSG:3857', 'EPSG:25832');
var_dump($pj);

// with area
$area = proj_area_create();
proj_area_set_bbox($area,10,49,12,51);
var_dump($area);

$pj = proj_create_crs_to_crs('EPSG:3857', 'EPSG:25832', $area);
var_dump($pj);

// no area
$pj = proj_create_crs_to_crs('EPSG:3857', 'EPSG:25832', []);

try {
    $pj = proj_create_crs_to_crs('+proj=foo','EPSG:4326');
} catch (Exception $exc) {
    echo $exc->getMessage();
    #proj_get_errno_string(proj_get_errno());
}
var_dump($pj);
?>
--EXPECTF--
resource(4) of type (Proj)
resource(5) of type (Area)
resource(6) of type (Proj)

Warning: proj_create_crs_to_crs() expects parameter 3 to be resource, array given in %s
proj_create: Error -5 (unknown projection id)
bool(false)