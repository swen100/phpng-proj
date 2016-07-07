--TEST--
pj_transform() function - array test with count and offset for pj_transform()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$pj_krovak = pj_init_plus('+proj=krovak +lat_0=49.5 +lon_0=24.83333333333333 +alpha=0 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel');
if ($pj_krovak === false) {
	die(pj_strerrno(pj_get_errno_ref()));
}
$pj_latlong = pj_init_plus("+proj=latlong +towgs84=570.8,85.7,462.8,4.998,1.587,5.261,3.56 +units=m");
if ($pj_latlong === false) {
	die(pj_strerrno(pj_get_errno_ref()));
}
$x = array(-739443, -656557, -648029, -669438, -647808);
$y = array(-1045546, -1070176, -1082386, -1077820, -1082527);
$p = pj_transform($pj_krovak, $pj_latlong, 2, 2, $x, $y);
var_dump($p);
?>
--EXPECT--
array(3) {
  ["x"]=>
  array(2) {
    [0]=>
    float(0.27581079889583)
    [1]=>
    float(0.27051821562369)
  }
  ["y"]=>
  array(2) {
    [0]=>
    float(0.86996688420932)
    [1]=>
    float(0.87027496148394)
  }
  ["z"]=>
  array(2) {
    [0]=>
    float(0)
    [1]=>
    float(0)
  }
}