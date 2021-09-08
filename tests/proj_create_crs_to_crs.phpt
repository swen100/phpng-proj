--TEST--
proj_create_crs_to_crs() function - basic test for proj_create_crs_to_crs()
--SKIPIF--
<?php
if (!extension_loaded('proj.4') && !extension_loaded('proj')) { echo 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$srcSRID = 'EPSG:25832';
$tgtSRID = 'EPSG:3857';

$pj = proj_create_crs_to_crs($srcSRID, $tgtSRID);
var_dump($pj);

// with area
$area = proj_area_create();
proj_area_set_bbox($area,10,49,12,51);
var_dump($area);

$pj = proj_create_crs_to_crs($srcSRID, $tgtSRID, $area);
var_dump($pj);

// no area
$pj = proj_create_crs_to_crs($srcSRID, $tgtSRID, []);

try {
    $pj = proj_create('+proj=foo');
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