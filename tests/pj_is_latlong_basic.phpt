--TEST--
proj_is_latlong() function - basic test for proj_is_latlong()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$pj = proj_create('+proj=latlong +towgs84=570.8,85.7,462.8,4.998,1.587,5.261,3.56 +units=m');
if ($pj === false) {
	die(proj_get_errno_string(proj_get_errno($pj)));
}
var_dump(proj_is_latlong($pj));
?>
--EXPECT--
bool(true)