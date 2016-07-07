--TEST--
pj_transform() function - basic array test with strings for pj_transform()
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
$x = array("-739443", "-656557");
$y = array("-1045546", "-1070176");
$p = pj_transform($pj_krovak, $pj_latlong, count($x), 0, $x, $y);
var_dump($p);
?>
--EXPECT--
array(3) {
  ["x"]=>
  array(2) {
    [0]=>
    float(0.25261488335704)
    [1]=>
    float(0.27340068275302)
  }
  ["y"]=>
  array(2) {
    [0]=>
    float(0.87388279965635)
    [1]=>
    float(0.87171037986086)
  }
  ["z"]=>
  array(2) {
    [0]=>
    float(0)
    [1]=>
    float(0)
  }
}