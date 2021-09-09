--TEST--
proj_get_def() function - basic test for proj_get_def()
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
}
?>
--FILE--
<?php
$pj = proj_create_crs_to_crs('EPSG:3857', 'EPSG:25832');
if ($pj === false) {
	die(proj_get_errno_string(proj_get_errno($pj)));
}
var_dump(proj_get_def($pj));
?>
--EXPECT--
string(109) "proj=pipeline step inv proj=webmerc lat_0=0 lon_0=0 x_0=0 y_0=0 ellps=WGS84 step proj=utm zone=32 ellps=GRS80"