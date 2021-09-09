--TEST--
proj_create_crs_to_crs_from_pj() function - basic test
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
}
?>
--FILE--
<?php
$srcProj = proj_create('EPSG:3857');
$tgtProj = proj_create('EPSG:25832');
$pj = proj_create_crs_to_crs_from_pj($srcProj, $tgtProj);
var_dump($pj);

// with area
$area = proj_area_create();
proj_area_set_bbox($area,10,49,12,51);

$pj = proj_create_crs_to_crs_from_pj($srcProj, $tgtProj, $area);
var_dump($pj);

// invalid parameters
$pj = proj_create_crs_to_crs_from_pj([], $tgtProj);
$pj = proj_create_crs_to_crs_from_pj($srcProj, []);
$pj = proj_create_crs_to_crs_from_pj($srcProj, $tgtProj, []);

?>
--EXPECTF--
resource(%d) of type (Proj)
resource(%d) of type (Proj)

Warning: proj_create_crs_to_crs_from_pj() expects parameter 1 to be resource, array given in %s

Warning: proj_create_crs_to_crs_from_pj() expects parameter 2 to be resource, array given in %s

Warning: proj_create_crs_to_crs_from_pj() expects parameter 3 to be resource, array given in %s