--TEST--
proj_transform() function - basic array test with strings for proj_transform_array()
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
$coords = ["11 51", "11.5 51.5 20"];
$transformed = proj_transform_array( $proj_wgs84, $proj_merc, $coords );
var_dump($transformed);
?>
--EXPECT--
array(2) {
  [0]=>
  array(3) {
    ["x"]=>
    float(1224514.398726)
    ["y"]=>
    float(6621293.7227402)
    ["z"]=>
    float(0)
  }
  [1]=>
  array(3) {
    ["x"]=>
    float(1280174.1441226)
    ["y"]=>
    float(6710219.0832207)
    ["z"]=>
    float(20)
  }
}