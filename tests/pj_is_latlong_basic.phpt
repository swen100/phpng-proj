--TEST--
pj_is_latlong() function - basic test for pj_is_latlong()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$pj = pj_init_plus('+proj=latlong +towgs84=570.8,85.7,462.8,4.998,1.587,5.261,3.56 +units=m');
if ($pj === false) {
	die(pj_strerrno(pj_get_errno_ref()));
}
var_dump(pj_is_latlong($pj));
?>
--EXPECT--
bool(true)