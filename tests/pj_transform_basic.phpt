--TEST--
proj_transform() function - basic test for proj_transform()
--SKIPIF--
<?php
if (!extension_loaded('proj.4')) { print 'skip proj.4 extension not available'; }
?>
--FILE--
<?php
$proj_merc = proj_create("+proj=merc +a=6378137 +b=6378137 +units=m +k=1.0 +nadgrids=@null +no_defs");
if ($proj_merc === false) {
	die(proj_get_errno_string(proj_get_errno($proj_merc)));
}
$proj_wgs84 = proj_create("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
if ($proj_wgs84 === false) {
	die(proj_get_errno_string(proj_get_errno($proj_wgs84)));
}
$x = 1224514.398726;
$y = 6621293.7227402;
$p = proj_transform_point($proj_merc, $proj_wgs84, $x, $y);
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