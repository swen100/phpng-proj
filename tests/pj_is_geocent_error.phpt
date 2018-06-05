--TEST--
pj_is_geocent() function - error test for pj_is_geocent()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$pj = pj_init_plus('+proj=latlong +towgs84=570.8,85.7,462.8,4.998,1.587,5.261,3.56 +units=m');
if ($pj === false) {
	die(pj_errno_string(pj_get_errno()));
}
var_dump(pj_is_geocent($pj));
?>
--EXPECT--
bool(false)