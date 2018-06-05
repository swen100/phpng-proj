--TEST--
pj_free() function - basic test for pj_free()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$krovak_input = '+proj=krovak +lat_0=49.5 +lon_0=24.83333333333333 +alpha=0 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel';
$pj = pj_init_plus($krovak_input);
if ($pj === false) {
	die(pj_strerrno(pj_get_errno_ref()));
}
pj_free($pj);
var_dump($pj);
?>
--EXPECT--
resource(4) of type (Unknown)