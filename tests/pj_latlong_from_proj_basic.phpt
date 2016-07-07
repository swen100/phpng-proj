--TEST--
pj_latlong_from_proj() function - basic test for pj_latlong_from_proj()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) print 'skip proj.4 extension not available';
?>
--FILE--
<?php
$pj_krovak = pj_init_plus('+proj=krovak +lat_0=49.5 +lon_0=24.83333333333333 +alpha=0 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel');
$pj_latlong = pj_latlong_from_proj($pj_krovak);
var_dump($pj_latlong);
?>
--EXPECT--
resource(5) of type (Proj.4)