--TEST--
pj_get_def() function - basic test for pj_get_def()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$krovak_input = '+proj=krovak +lat_0=49.5 +lon_0=24.83333333333333 +alpha=0 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel';
$pj = pj_init_plus($krovak_input);
if ($pj === false) {
	die(pj_errno_string(pj_get_errno()));
}
var_dump(pj_get_def($pj));
?>
--EXPECT--
string(80) "proj=krovak lat_0=49.5 lon_0=24.83333333333333 k=0.9999 x_0=0 y_0=0 ellps=bessel"