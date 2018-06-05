--TEST--
proj_create() function - basic test for proj_create()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$pj = proj_create('+proj=krovak +lat_0=49.5 +lon_0=24.83333333333333 +alpha=0 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel');
if ($pj === false) {
	die(proj_get_errno_string(proj_get_errno($pj)));
}
var_dump($pj);
?>
--EXPECT--
resource(4) of type (Proj.4)