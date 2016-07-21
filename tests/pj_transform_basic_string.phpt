--TEST--
pj_transform() function - basic test for pj_transform()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$proj_merc = pj_init_plus("+proj=merc +a=6378137 +b=6378137 +units=m +k=1.0 +nadgrids=@null +no_defs");
if ($pj_merc === false) {
	die(pj_strerrno(pj_get_errno_ref()));
}
$proj_wgs84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
if ($proj_wgs84 === false) {
	die(pj_strerrno(pj_get_errno_ref()));
}
$x = "1224514.398726";
$y = "6621293.7227402";
$p = pj_transform_point($proj_merc, $pj_latlong, $x, $y);
var_dump($p);
?>
--EXPECT--
array(3) {
  ["x"]=>
  float(11)
  ["y"]=>
  float(51)
  ["z"]=>
  float(0)
}