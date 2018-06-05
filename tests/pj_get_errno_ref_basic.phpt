--TEST--
pj_get_errno_ref() function - basic test for pj_get_errno_ref()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
#$pj = pj_init_plus('+proj=foo');
$pj = pj_init_plus('+proj=geocent +lat_0=49.5 +lon_0=24.83333333333333 +alpha=0 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel');
var_dump(pj_get_errno($pj));
?>
--EXPECT--
int(0)