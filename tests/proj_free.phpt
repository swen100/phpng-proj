--TEST--
proj_free() function - basic test for proj_free()
--SKIPIF--
<?php
if (!extension_loaded('proj.4') && !extension_loaded('proj')) { echo 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$krovak_input = '+proj=krovak +lat_0=49.5 +lon_0=24.83333333333333 +alpha=0 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel';
$pj = proj_create($krovak_input);
if ($pj === false) {
	die(proj_get_errno_string(proj_get_errno($pj)));
}
proj_free($pj);
var_dump($pj);
?>
--EXPECT--
resource(4) of type (Unknown)