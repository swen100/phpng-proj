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
// with EPSG-code
$pj = proj_create_crs_to_crs('EPSG:3857', 'EPSG:25832');
var_dump($pj);

// with proj-string
$proj_wgs84 = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
$proj_merc = "+proj=merc +a=6378137 +b=6378137 +units=m +k=1.0 +nadgrids=@null +no_defs";
$pj = proj_create_crs_to_crs($proj_wgs84, $proj_merc);
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
resource(%d) of type (Proj)
resource(%d) of type (Proj)
resource(%d) of type (Area)
resource(%d) of type (Proj)

Warning: proj_create_crs_to_crs() expects parameter 3 to be resource, array given in %s
proj_create: Error %i (%s)%S
bool(false)