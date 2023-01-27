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

// with proj-string
$proj_wgs84 = proj_create("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
$proj_merc = proj_create("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +units=m +k=1.0 +nadgrids=@null +no_defs");
$pj = proj_create_crs_to_crs_from_pj($proj_wgs84, $proj_merc);
var_dump($pj);

// invalid parameters
try {
    $pj = proj_create_crs_to_crs_from_pj([], $tgtProj);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    $pj = proj_create_crs_to_crs_from_pj($srcProj, []);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

try {
    $pj = proj_create_crs_to_crs_from_pj($srcProj, $tgtProj, []);
} catch (TypeError $exc) {
    echo $exc->getMessage() . "\n";
}

?>
--EXPECTF--
resource(%d) of type (Proj)
resource(%d) of type (Proj)
proj_create_operations: source_crs is not a CRS
bool(false)
proj_create_crs_to_crs_from_pj(): Argument #1 ($src_crs) must be of type resource, array given
proj_create_crs_to_crs_from_pj(): Argument #2 ($tgtc_crs) must be of type resource, array given
proj_create_crs_to_crs_from_pj(): Argument #3 ($proj_area) must be of type resource, array given