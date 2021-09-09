--TEST--
proj_free() function - basic test for proj_free()
--SKIPIF--
<?php
if (!extension_loaded('proj')) {
    echo 'skip proj extension not available';
}
?>
--FILE--
<?php
$pj = proj_create_crs_to_crs('EPSG:4326', 'EPSG:25832');
if ($pj === false) {
	die(proj_get_errno_string(proj_get_errno($pj)));
}
proj_free($pj);
var_dump($pj);
?>
--EXPECT--
resource(4) of type (Unknown)